// scraper/scraper.h
#pragma once
#include <QtNetwork>

class Scraper : public QObject {
    Q_OBJECT
public:
    void fetch(const QUrl& url);

signals:
    void dataReady(QString html);

private:
    QNetworkAccessManager manager;
};

