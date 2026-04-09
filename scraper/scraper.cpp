// scraper/scraper.cpp
#include "scraper.h"

void Scraper::fetch(const QUrl& url) {
    auto reply = manager.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        emit dataReady(reply->readAll());
        reply->deleteLater();
    });
}

