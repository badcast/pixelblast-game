#pragma once

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>

#include "PixelBegin.h"

struct PixelStats
{
    int id;
    QString name;
    int maxPoints;
    int rankPos;
};

class PB_EXPORT PixelNetwork : public QObject
{
    Q_OBJECT

private:
    QNetworkAccessManager *manager;

public:
    PixelNetwork(QObject *parent = nullptr);
    ~PixelNetwork();

    void newClient(QString nickname);
    void updateStats(PixelStats stat);
    void readStats();

signals:
    void callbackCurrent(const PixelStats &stats, bool ok);
    void callbackStats(const QList<PixelStats> &stats, bool ok);

private slots:
    void onReplyCurrent();
    void onReplyStats();
};
