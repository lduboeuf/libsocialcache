/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Lucien Xu <lucien.xu@jollamobile.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef SYNCHELPER_H
#define SYNCHELPER_H

#include <QtCore/QObject>
#include <QtQml/QQmlParserStatus>
#include "socialsyncinterface.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <buteosyncfw6/SyncClientInterface.h>
#else
#include <buteosyncfw5/SyncClientInterface.h>
#endif

class SyncHelper : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(SocialSyncInterface::SocialNetwork socialNetwork READ socialNetwork
               WRITE setSocialNetwork NOTIFY socialNetworkChanged)
    Q_PROPERTY(SocialSyncInterface::DataType dataType READ dataType
               WRITE setDataType NOTIFY dataTypeChanged)
    Q_PROPERTY(QStringList syncProfiles READ syncProfiles NOTIFY syncProfilesChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
public:
    explicit SyncHelper(QObject *parent = 0);

    void classBegin();
    void componentComplete();

    SocialSyncInterface::SocialNetwork socialNetwork() const;
    void setSocialNetwork(SocialSyncInterface::SocialNetwork socialNetwork);

    SocialSyncInterface::DataType dataType() const;
    void setDataType(SocialSyncInterface::DataType dataType);

    QStringList syncProfiles() const;

    bool loading() const;

public Q_SLOTS:
    void sync();
Q_SIGNALS:
    void socialNetworkChanged();
    void dataTypeChanged();
    void syncProfilesChanged();
    void loadingChanged();
    void profileDeleted();

private slots:
    void slotSyncStatus(const QString &aProfileId, int aStatus, const QString &aMessage,
                        int aStatusDetails);
    void slotProfileChanged(QString aProfileId,int aChangeType, QString aChangedProfile);

private:
    void refresh();
    void refreshSyncProfiles();
    void checkCurrentRun();
    void setLoading(bool loading);
    bool profileIdMatches(const QString &profileId) const;

    Buteo::SyncClientInterface *m_interface;
    SocialSyncInterface::SocialNetwork m_socialNetwork;
    SocialSyncInterface::DataType m_dataType;
    QStringList m_activeSyncs;
    QStringList m_syncProfiles;
    bool m_complete;
    bool m_loading;
};

#endif // SYNCHELPER_H
