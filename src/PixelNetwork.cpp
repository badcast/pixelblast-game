#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "PixelNetwork.h"

#ifndef CALLBACK_URL
constexpr char CallbackUrl[] = "https://example.com/callback";
#else
constexpr char CallbackUrl[] = CALLBACK_URL;
#endif

inline std::pair<bool, PixelStats> getPixelStatObject(QJsonObject jsonObject)
{
    PixelStats stat {};
    bool success;
    if((success = (jsonObject["id"].isDouble() && jsonObject["name"].isString() && jsonObject["maxPoints"].isDouble())))
    {
        stat.id = jsonObject["id"].toInt();
        stat.name = jsonObject["name"].toString();
        stat.maxPoints = jsonObject["maxPoints"].toInt();
    }
    return {success, stat};
}

PixelNetwork::PixelNetwork(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    manager->setTransferTimeout(3000);
}

PixelNetwork::~PixelNetwork()
{
}

void PixelNetwork::newClient(QString nickname)
{
    QJsonDocument jdoc;
    QJsonObject json;
    QNetworkReply *reply;

    json["name"] = nickname;
    json["maxPoints"] = 0;
    QString aaa =  QJsonDocument(json).toJson(QJsonDocument::Compact);
    reply = manager->post(QNetworkRequest(QUrl(CallbackUrl)), aaa.toUtf8());
    QObject::connect(reply, &QNetworkReply::finished, this, &PixelNetwork::onReplyCurrent);
}

void PixelNetwork::updateStats(PixelStats stat)
{
    QJsonDocument jdoc;
    QJsonObject json;
    QNetworkReply *reply;

    json["id"] = stat.id;
    json["name"] = stat.name;
    json["maxPoints"] = stat.maxPoints;
    reply = manager->post(QNetworkRequest(QUrl(CallbackUrl)), QJsonDocument(json).toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, &PixelNetwork::onReplyCurrent);
}

void PixelNetwork::readStats()
{
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(CallbackUrl)));
    QObject::connect(reply, &QNetworkReply::finished, this, &PixelNetwork::onReplyStats);
}

// CALLBACK RETURN SLOTS

void PixelNetwork::onReplyCurrent()
{
    PixelStats curStat {};
    bool ok = false;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject data = jdoc["data"]["client"].toObject();
            if((ok = jdoc["ok"].toBool() && !data.isEmpty()))
            {
                const auto &result = getPixelStatObject(data);
                if((ok = result.first))
                    curStat = result.second;
            }
        }
        emit callbackCurrent(curStat, ok);
        reply->deleteLater();
    }
}

void PixelNetwork::onReplyStats()
{
    QList<PixelStats> stats {};
    bool ok = false;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(reply)
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray items = jdoc["data"]["items"].toArray();
            if((ok = jdoc["ok"].toBool()))
            {
                for(int x = 0; x < items.size(); ++x)
                {
                    const auto &result = getPixelStatObject(items[x].toObject());
                    if(!(ok = result.first))
                    {
                        stats.clear();
                        break;
                    }
                    stats.push_back(result.second);
                }
            }
        }
        emit callbackStats(stats, ok);
        reply->deleteLater();
    }
}
