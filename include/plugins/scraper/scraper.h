/**
 * @file scraper.h
 * @brief Web scraper for HTML content extraction
 */

#pragma once
#include <QtNetwork>

/**
 * @brief Web scraper for HTML content extraction
 *
 * Fetches HTML content from URLs and emits the data when ready.
 * Uses QNetworkAccessManager for HTTP/HTTPS requests.
 */
class Scraper : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Fetches HTML content from the specified URL
     * @param url Target URL to scrape
     */
    void fetch(const QUrl& url);

signals:
    /**
     * @brief Emitted when HTML data is ready
     * @param html HTML content
     */
    void dataReady(QString html);

private:
    QNetworkAccessManager manager;
};

