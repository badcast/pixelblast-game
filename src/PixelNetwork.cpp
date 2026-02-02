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

inline std::pair<bool, PixelStats> getPixelStatObjec(QJsonObject jobj)
{
    PixelStats stat {};
    bool success;
    if((success = (jobj["id"].isDouble() && jobj["name"].isString() && jobj["maxPoints"].isDouble())))
    {
        stat.id = jobj["id"].toInt();
        stat.name = jobj["name"].toString();
        stat.maxPoints = jobj["maxPoints"].toInt();
    }
    return {success, stat};
}

PixelNetwork::PixelNetwork(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    manager->setTransferTimeout(5000);
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
    reply = manager->post(QNetworkRequest(QUrl(CallbackUrl)), jdoc.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, &PixelNetwork::onReplyCurrent);
}

void PixelNetwork::changeData(PixelStats stat)
{
    QJsonDocument jdoc;
    QJsonObject json;
    QNetworkReply *reply;

    json["id"] = stat.id;
    json["name"] = stat.name;
    json["maxPoints"] = stat.maxPoints;
    reply = manager->post(QNetworkRequest(QUrl(CallbackUrl)), jdoc.toJson(QJsonDocument::Compact));
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
                const auto &result = getPixelStatObjec(data);
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
                    const auto &result = getPixelStatObjec(items[x].toObject());
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
