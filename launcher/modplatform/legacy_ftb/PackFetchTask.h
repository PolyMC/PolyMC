#pragma once

#include "PackHelpers.h"
#include "net/NetJob.h"
#include <QByteArray>
#include <QObject>
#include <QTemporaryDir>

namespace LegacyFTB {

class PackFetchTask : public QObject {

    Q_OBJECT

public:
    PackFetchTask(shared_qobject_ptr<QNetworkAccessManager> network) : QObject(nullptr), m_network(network) {};
    ~PackFetchTask() override = default;

    void fetch();
    void fetchPrivate(const QStringList &toFetch);

private:
    shared_qobject_ptr<QNetworkAccessManager> m_network;
    NetJob::Ptr jobPtr;

    QByteArray publicModpacksXmlFileData;
    QByteArray thirdPartyModpacksXmlFileData;

    bool parseAndAddPacks(QByteArray &data, PackType packType, ModpackList &list);
    ModpackList publicPacks;
    ModpackList thirdPartyPacks;

protected slots:
    void fileDownloadFinished();
    void fileDownloadFailed(QString reason);

signals:
    void finished(ModpackList publicPacks, ModpackList thirdPartyPacks);
    void failed(QString reason);

    void privateFileDownloadFinished(Modpack modpack);
    void privateFileDownloadFailed(QString reason, QString packCode);
};

}
