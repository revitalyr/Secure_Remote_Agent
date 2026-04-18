/**
 * @file scraper_plugin.h
 * @brief Scraper plugin for web scraping operations
 */

#pragma once
#include "../loader/plugin_interface.h"
#include <QNetworkAccessManager>
#include <QTimer>
#include <QMutex>
#include <QRegularExpression>

/**
 * @brief Scraper plugin for web scraping operations
 *
 * Implements scheduled web scraping with configurable selectors.
 * Extracts data from HTML content and provides metrics on pages
 * scraped and data extracted.
 */
class ScraperPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.secureagent.IPlugin" FILE "scraper_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    /**
     * @brief Constructs a scraper plugin
     */
    ScraperPlugin();

    /**
     * @brief Destroys the scraper plugin
     */
    ~ScraperPlugin() override;

    // IPlugin interface
    /**
     * @brief Gets the plugin name
     * @return Plugin name
     */
    QString name() const override;

    /**
     * @brief Gets the plugin version
     * @return Plugin version string
     */
    QString version() const override;

    /**
     * @brief Gets the plugin description
     * @return Plugin description
     */
    QString description() const override;

    /**
     * @brief Initializes the plugin with configuration
     * @param config Configuration map (default: empty)
     * @return true if initialization succeeded, false otherwise
     */
    bool initialize(const QVariantMap& config = QVariantMap()) override;

    /**
     * @brief Starts the scraping schedule
     */
    void start() override;

    /**
     * @brief Stops the scraping schedule
     */
    void stop() override;

    /**
     * @brief Cleans up plugin resources
     */
    void cleanup() override;

    /**
     * @brief Executes the plugin with default parameters
     */
    void execute() override;

    /**
     * @brief Executes the plugin with custom parameters
     * @param params Execution parameters (url, selectors)
     * @return Result map
     */
    QVariantMap executeWithParams(const QVariantMap& params) override;

    /**
     * @brief Gets the current plugin status
     * @return Plugin status
     */
    PluginStatus status() const override;

    /**
     * @brief Gets plugin metrics
     * @return Metrics map (pages scraped, data extracted)
     */
    QVariantMap getMetrics() const override;

    /**
     * @brief Gets the last error message
     * @return Error message string
     */
    QString getLastError() const override;

    /**
     * @brief Sets plugin configuration
     * @param config Configuration map
     * @return true if configuration was set successfully, false otherwise
     */
    bool setConfig(const QVariantMap& config) override;

    /**
     * @brief Gets plugin configuration
     * @return Configuration map
     */
    QVariantMap getConfig() const override;

    /**
     * @brief Sets a callback for async operations
     * @param callback Callback function
     */
    void setCallback(std::function<void(const QString&, const QVariantMap&)> callback) override;

private slots:
    /**
     * @brief Handles scrape completion
     */
    void onScrapeFinished();

    /**
     * @brief Handles scrape timer timeout
     */
    void onScrapeTimer();

private:
    /**
     * @brief Sets up the scraping schedule
     */
    void setupScrapingSchedule();

    /**
     * @brief Scrapes the specified URL with selectors
     * @param url Target URL
     * @param selectors CSS selectors for data extraction
     */
    void scrapeUrl(const QString& url, const QStringList& selectors);

    /**
     * @brief Parses HTML content with selectors
     * @param html HTML content
     * @param selectors CSS selectors
     */
    void parseHtml(const QString& html, const QStringList& selectors);

    /**
     * @brief Updates plugin metrics
     */
    void updateMetrics();

    PluginStatus m_status;
    QVariantMap m_config;
    QString m_lastError;
    std::function<void(const QString&, const QVariantMap&)> m_callback;

    QNetworkAccessManager* m_network;
    QTimer* m_scrapeTimer;
    QRegularExpression m_htmlTagRegex;

    // Metrics
    qint64 m_pagesScraped;
    qint64 m_dataExtracted;
    qint64 m_startTime;
    QStringList m_scrapedUrls;

    mutable QMutex m_mutex;
};
