// scraper/scraper_plugin.cpp
#include "scraper_plugin.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ScraperPlugin::ScraperPlugin() 
    : m_status(PluginStatus::Unloaded)
    , m_network(nullptr)
    , m_scrapeTimer(nullptr)
    , m_pagesScraped(0)
    , m_dataExtracted(0)
    , m_startTime(0)
{
    m_htmlTagRegex = QRegularExpression("<[^>]*>");
}

ScraperPlugin::~ScraperPlugin() {
    cleanup();
}

QString ScraperPlugin::name() const {
    return "ScraperPlugin";
}

QString ScraperPlugin::version() const {
    return "1.0.0";
}

QString ScraperPlugin::description() const {
    return "Web scraper plugin for data extraction and monitoring";
}

bool ScraperPlugin::initialize(const QVariantMap& config) {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Unloaded) {
        m_lastError = "Plugin already initialized";
        return false;
    }
    
    m_config = config;
    m_config["scrape_interval"] = m_config.value("scrape_interval", 300); // 5 minutes
    m_config["user_agent"] = m_config.value("user_agent", "SecureAgent-Scraper/1.0");
    m_config["max_pages"] = m_config.value("max_pages", 100);
    m_config["enable_js"] = m_config.value("enable_js", false);
    
    m_network = new QNetworkAccessManager(this);
    
    m_status = PluginStatus::Loaded;
    m_lastError.clear();
    
    if (m_callback) {
        m_callback(name(), {{"event", "initialized"}, {"config", m_config}});
    }
    
    return true;
}

void ScraperPlugin::start() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Loaded) {
        m_lastError = "Plugin not initialized";
        return;
    }
    
    setupScrapingSchedule();
    
    m_startTime = QDateTime::currentSecsSinceEpoch();
    m_status = PluginStatus::Running;
    m_lastError.clear();
    
    if (m_callback) {
        m_callback(name(), {{"event", "started"}, {"interval", m_config["scrape_interval"]}});
    }
}

void ScraperPlugin::stop() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Running) {
        return;
    }
    
    if (m_scrapeTimer) {
        m_scrapeTimer->stop();
        delete m_scrapeTimer;
        m_scrapeTimer = nullptr;
    }
    
    m_status = PluginStatus::Loaded;
    
    if (m_callback) {
        m_callback(name(), {{"event", "stopped"}});
    }
}

void ScraperPlugin::cleanup() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status == PluginStatus::Running) {
        stop();
    }
    
    if (m_network) {
        delete m_network;
        m_network = nullptr;
    }
    
    m_status = PluginStatus::Unloaded;
    m_pagesScraped = 0;
    m_dataExtracted = 0;
    m_scrapedUrls.clear();
}

void ScraperPlugin::execute() {
    QVariantMap params;
    params["action"] = "scrape_sample";
    executeWithParams(params);
}

QVariantMap ScraperPlugin::executeWithParams(const QVariantMap& params) {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap result;
    result["plugin"] = name();
    result["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    QString action = params.value("action", "status").toString();
    
    if (action == "scrape_url") {
        QString url = params.value("url").toString();
        QStringList selectors = params.value("selectors", QStringList()).toStringList();
        
        if (!url.isEmpty()) {
            scrapeUrl(url, selectors);
            result["status"] = "scraping_started";
            result["url"] = url;
        } else {
            result["error"] = "Missing URL parameter";
        }
    } else if (action == "get_stats") {
        result["pages_scraped"] = m_pagesScraped;
        result["data_extracted"] = m_dataExtracted;
        result["scraped_urls"] = m_scrapedUrls;
        result["uptime_seconds"] = m_startTime > 0 ? QDateTime::currentSecsSinceEpoch() - m_startTime : 0;
    } else if (action == "scrape_sample") {
        // Scrape a sample website for demonstration
        QStringList selectors;
        selectors << "title" << "h1" << "p";
        scrapeUrl("https://httpbin.org/html", selectors);
        result["status"] = "sample_scraping_started";
    } else {
        result["status"] = "unknown_action";
        result["action"] = action;
    }
    
    return result;
}

PluginStatus ScraperPlugin::status() const {
    QMutexLocker locker(&m_mutex);
    return m_status;
}

QVariantMap ScraperPlugin::getMetrics() const {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap metrics;
    metrics["pages_scraped"] = m_pagesScraped;
    metrics["data_extracted"] = m_dataExtracted;
    metrics["uptime_seconds"] = m_startTime > 0 ? QDateTime::currentSecsSinceEpoch() - m_startTime : 0;
    metrics["scraped_urls_count"] = m_scrapedUrls.size();
    metrics["status"] = static_cast<int>(m_status);
    
    return metrics;
}

QString ScraperPlugin::getLastError() const {
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool ScraperPlugin::setConfig(const QVariantMap& config) {
    QMutexLocker locker(&m_mutex);
    m_config = config;
    return true;
}

QVariantMap ScraperPlugin::getConfig() const {
    QMutexLocker locker(&m_mutex);
    return m_config;
}

void ScraperPlugin::setCallback(std::function<void(const QString&, const QVariantMap&)> callback) {
    m_callback = callback;
}

void ScraperPlugin::setupScrapingSchedule() {
    m_scrapeTimer = new QTimer(this);
    connect(m_scrapeTimer, &QTimer::timeout, this, &ScraperPlugin::onScrapeTimer);
    
    int interval = m_config["scrape_interval"].toInt() * 1000; // Convert to milliseconds
    m_scrapeTimer->start(interval);
}

void ScraperPlugin::scrapeUrl(const QString& url, const QStringList& selectors) {
    if (!m_network) return;
    
    QUrl targetUrl(url);
    QNetworkRequest request(targetUrl);
    request.setRawHeader("User-Agent", m_config["user_agent"].toString().toUtf8());
    
    QNetworkReply* reply = m_network->get(request);
    
    // Store selectors for use when request finishes
    reply->setProperty("selectors", selectors);
    reply->setProperty("url", url);
    
    connect(reply, &QNetworkReply::finished, this, &ScraperPlugin::onScrapeFinished);
}

void ScraperPlugin::parseHtml(const QString& html, const QStringList& selectors) {
    QJsonArray extractedData;
    
    if (selectors.isEmpty()) {
        // Extract all text content if no selectors specified
        QString cleanText = html;
        cleanText.remove(m_htmlTagRegex);
        cleanText = cleanText.simplified();
        
        QJsonObject textData;
        textData["selector"] = "text_content";
        textData["content"] = cleanText.left(500); // Limit to first 500 chars
        extractedData.append(textData);
    } else {
        // Extract based on selectors (simplified implementation)
        for (const QString& selector : selectors) {
            QJsonObject elementData;
            elementData["selector"] = selector;
            
            // In a real implementation, this would use proper HTML parsing
            // For now, we simulate extraction
            if (selector == "title") {
                QRegularExpression titleRegex("<title>([^<]+)</title>");
                QRegularExpressionMatch match = titleRegex.match(html);
                if (match.hasMatch()) {
                    elementData["content"] = match.captured(1);
                }
            } else if (selector == "h1") {
                QRegularExpression h1Regex("<h1[^>]*>([^<]+)</h1>");
                QRegularExpressionMatch match = h1Regex.match(html);
                if (match.hasMatch()) {
                    elementData["content"] = match.captured(1);
                }
            } else {
                elementData["content"] = "Selector not implemented: " + selector;
            }
            
            extractedData.append(elementData);
        }
    }
    
    m_dataExtracted += extractedData.size();
    
    if (m_callback) {
        m_callback(name(), {
            {"event", "data_extracted"},
            {"data", extractedData},
            {"selectors", selectors}
        });
    }
}

void ScraperPlugin::onScrapeFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString url = reply->property("url").toString();
    QStringList selectors = reply->property("selectors").toStringList();
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray htmlData = reply->readAll();
        QString html = QString::fromUtf8(htmlData);
        
        parseHtml(html, selectors);
        
        m_pagesScraped++;
        if (!m_scrapedUrls.contains(url)) {
            m_scrapedUrls.append(url);
        }
        
        if (m_callback) {
            m_callback(name(), {
                {"event", "page_scraped"},
                {"url", url},
                {"content_size", htmlData.size()},
                {"selectors", selectors}
            });
        }
    } else {
        m_lastError = reply->errorString();
        
        if (m_callback) {
            m_callback(name(), {
                {"event", "scrape_error"},
                {"url", url},
                {"error", reply->errorString()}
            });
        }
    }
    
    reply->deleteLater();
}

void ScraperPlugin::onScrapeTimer() {
    // Perform scheduled scraping
    QVariantMap params;
    params["action"] = "scrape_sample";
    executeWithParams(params);
    
    updateMetrics();
}

void ScraperPlugin::updateMetrics() {
    if (m_callback) {
        m_callback(name(), {
            {"event", "metrics_update"},
            {"metrics", getMetrics()}
        });
    }
}

#include "scraper_plugin.moc"
