/*
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Chris Adams <chris.adams@jollamobile.com>
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

#include "vkimagecachemodel.h"
#include "abstractsocialcachemodel_p.h"
#include "vkimagesdatabase.h"
#include "vkimagedownloader_p.h"

#include <QtCore/QThread>
#include <QtCore/QStandardPaths>

#include <QtDebug>

static const char *URL_KEY = "url";
static const char *ROW_KEY = "row";
static const char *MODEL_KEY = "model";

static const char *TYPE_KEY = "type";
static const char *PHOTOID_KEY = "photo_id";
static const char *ALBUMID_KEY = "album_id";
static const char *OWNERID_KEY = "owner_id";
static const char *ACCOUNTID_KEY = "account_id";

#define SOCIALCACHE_VK_IMAGE_DIR   PRIVILEGED_DATA_DIR + QLatin1String("/Images/")

class VKImageCacheModelPrivate : public AbstractSocialCacheModelPrivate
{
public:
    VKImageCacheModelPrivate(VKImageCacheModel *q);

    void queue(
            int row,
            VKImageDownloader::ImageType imageType,
            int accountId,
            const QString &user_id,
            const QString &album_id,
            const QString &photo_id,
            const QString &url);

    VKImageDownloader *downloader;
    VKImagesDatabase database;
    VKImageCacheModel::ModelDataType type;
};

VKImageCacheModelPrivate::VKImageCacheModelPrivate(VKImageCacheModel *q)
    : AbstractSocialCacheModelPrivate(q), downloader(0), type(VKImageCacheModel::Images)
{
}

void VKImageCacheModelPrivate::queue(
        int row,
        VKImageDownloader::ImageType imageType,
        int accountId,
        const QString &user_id,
        const QString &album_id,
        const QString &photo_id,
        const QString &url)
{
    VKImageCacheModel *modelPtr = qobject_cast<VKImageCacheModel*>(q_ptr);
    if (downloader) {
        QVariantMap metadata;
        metadata.insert(QLatin1String(ROW_KEY), row);
        metadata.insert(QLatin1String(TYPE_KEY), imageType);
        metadata.insert(QLatin1String(ACCOUNTID_KEY), accountId);
        metadata.insert(QLatin1String(OWNERID_KEY), user_id);
        metadata.insert(QLatin1String(ALBUMID_KEY), album_id);
        metadata.insert(QLatin1String(PHOTOID_KEY), photo_id);
        metadata.insert(QLatin1String(URL_KEY), url);
        metadata.insert(QLatin1String(MODEL_KEY), QVariant::fromValue<void*>((void*)modelPtr));

        downloader->queue(url, metadata);
    }
}

VKImageCacheModel::VKImageCacheModel(QObject *parent)
    : AbstractSocialCacheModel(*(new VKImageCacheModelPrivate(this)), parent)
{
    Q_D(const VKImageCacheModel);
    connect(&d->database, &VKImagesDatabase::queryFinished,
            this, &VKImageCacheModel::queryFinished);
}

VKImageCacheModel::~VKImageCacheModel()
{
    Q_D(VKImageCacheModel);
    if (d->downloader) {
        d->downloader->removeModelFromHash(this);
    }
}

QHash<int, QByteArray> VKImageCacheModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(PhotoId, "photoId");
    roleNames.insert(AlbumId, "albumId");
    roleNames.insert(UserId, "userId");
    roleNames.insert(AccountId, "accountId");
    roleNames.insert(Text, "text");
    roleNames.insert(Date, "date");
    roleNames.insert(Width, "photoWidth");
    roleNames.insert(Height, "photoHeight");
    roleNames.insert(Thumbnail, "thumbnail");
    roleNames.insert(Image, "image");
    roleNames.insert(Count, "dataCount");
    roleNames.insert(MimeType, "mimeType");
    roleNames.insert(ImageSource, "imageSource");
    return roleNames;
}

QString VKImageCacheModel::constructNodeIdentifier(int accountId, const QString &user_id, const QString &album_id, const QString &photo_id)
{
    return QString(QLatin1String("%1:%2:%3:%4")).arg(accountId).arg(user_id).arg(album_id).arg(photo_id);
}

QVariantMap VKImageCacheModel::parseNodeIdentifier(const QString &nid) const
{
    QStringList values = nid.split(':');
    QVariantMap retn;
    retn.insert("accountId", values.value(0));
    retn.insert("user_id", values.value(1));
    retn.insert("album_id", values.value(2));
    retn.insert("photo_id", values.value(3));
    return retn;
}

VKImageCacheModel::ModelDataType VKImageCacheModel::type() const
{
    Q_D(const VKImageCacheModel);
    return d->type;
}

void VKImageCacheModel::setType(VKImageCacheModel::ModelDataType type)
{
    Q_D(VKImageCacheModel);
    if (d->type != type) {
        d->type = type;
        emit typeChanged();
    }
}

VKImageDownloader * VKImageCacheModel::downloader() const
{
    Q_D(const VKImageCacheModel);
    return d->downloader;
}

void VKImageCacheModel::setDownloader(VKImageDownloader *downloader)
{
    Q_D(VKImageCacheModel);
    if (d->downloader != downloader) {
        if (d->downloader) {
            // Disconnect worker object
            disconnect(d->downloader);
            d->downloader->removeModelFromHash(this);
        }

        d->downloader = downloader;
        d->downloader->addModelToHash(this);
        emit downloaderChanged();
    }
}

void VKImageCacheModel::removeImage(const QString &imageId)
{
    Q_D(VKImageCacheModel);

    int row = -1;
    for (int i = 0; i < count(); ++i) {
        QString dbId = data(index(i), VKImageCacheModel::PhotoId).toString();
        if (dbId == imageId) {
            row = i;
            break;
        }
    }

    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        d->m_data.removeAt(row);
        endRemoveRows();

        // Update album image count
        VKImage::ConstPtr image = d->database.image(imageId);
        if (image) {
            VKAlbum::ConstPtr album = d->database.album(image->albumId());
            if (album) {
                d->database.addAlbum(VKAlbum::create(album->id(), album->ownerId(), album->title(),
                                                     album->description(), album->thumbSrc(),
                                                     album->thumbFile(), album->size()-1, album->created(),
                                                     album->updated(), album->accountId()));
            }

            d->database.removeImage(image);
            d->database.commit();
        }
    }
}

QVariant VKImageCacheModel::data(const QModelIndex &index, int role) const
{
    Q_D(const VKImageCacheModel);
    int row = index.row();
    if (row < 0 || row >= d->m_data.count()) {
        return QVariant();
    }

    return d->m_data.at(row).value(role);
}

void VKImageCacheModel::loadImages()
{
    refresh();
}

void VKImageCacheModel::refresh()
{
    Q_D(VKImageCacheModel);

    QVariantMap parsedNodeIdentifier = parseNodeIdentifier(d->nodeIdentifier);

    switch (d->type) {
    case VKImageCacheModel::Users:
        d->database.queryUsers();
        break;
    case VKImageCacheModel::Albums:
        d->database.queryAlbums(parsedNodeIdentifier.value("accountId").toInt(),
                                parsedNodeIdentifier.value("user_id").toString());
        break;
    case VKImageCacheModel::Images:
        if (parsedNodeIdentifier.value("user_id").toString().isEmpty()) {
            d->database.queryUserImages();
        } else if (parsedNodeIdentifier.value("album_id").toString().isEmpty()) {
            d->database.queryUserImages(parsedNodeIdentifier.value("accountId").toInt(),
                                        parsedNodeIdentifier.value("user_id").toString());
        } else {
            d->database.queryAlbumImages(parsedNodeIdentifier.value("accountId").toInt(),
                                         parsedNodeIdentifier.value("user_id").toString(),
                                         parsedNodeIdentifier.value("album_id").toString());
        }
        break;
    default:
        break;
    }
}

// NOTE: this is now called directly by VKImageDownloader
// rather than connected to the imageDownloaded signal, for
// performance reasons.
void VKImageCacheModel::imageDownloaded(const QString &url, const QString &path, const QVariantMap &imageData)
{
    Q_UNUSED(url);
    Q_D(VKImageCacheModel);

    if (path.isEmpty()) {
        // empty path signifies an error, which we don't handle here at the moment.
        // Return, otherwise dataChanged signal would cause UI to read back
        // related value, which, being empty, would trigger another download request,
        // potentially causing never ending loop.
        return;
    }

    int row = imageData.value(ROW_KEY).toInt();
    if (row < 0 || row >= d->m_data.count()) {
        qWarning() << Q_FUNC_INFO
                   << "Invalid row:" << row
                   << "max row:" << d->m_data.count();
        return;
    }

    int type = imageData.value(TYPE_KEY).toInt();
    switch (type) {
    case VKImageDownloader::ThumbnailImage:
        d->m_data[row].insert(VKImageCacheModel::Thumbnail, path);
        break;
    default:
        qWarning() << Q_FUNC_INFO << "invalid downloader type: " << type;
        break;
    }

    emit dataChanged(index(row), index(row));
}

void VKImageCacheModel::queryFinished()
{
    Q_D(VKImageCacheModel);

    QList<QVariantMap> thumbQueue;
    SocialCacheModelData data;
    switch (d->type) {
    case Users: {
        QList<VKUser::ConstPtr> usersData = d->database.users();
        for (int i = 0; i < usersData.count(); i++) {
            const VKUser::ConstPtr &userData(usersData[i]);
            QMap<int, QVariant> userMap;
            userMap.insert(VKImageCacheModel::UserId, userData->id());
            userMap.insert(VKImageCacheModel::AccountId, userData->accountId());
            userMap.insert(VKImageCacheModel::Text, userData->firstName() + ' ' + userData->lastName());
            userMap.insert(VKImageCacheModel::Count, userData->photosCount());
            data.append(userMap);
        }

        if (data.count() > 1) {
            QMap<int, QVariant> userMap;
            int count = 0;
            Q_FOREACH (const VKUser::ConstPtr &userData, usersData) {
                count += userData->photosCount();
            }

            userMap.insert(VKImageCacheModel::UserId, QString());
            userMap.insert(VKImageCacheModel::Thumbnail, QString());
            //: Label for the "show all users from all VK accounts" option
            //% "All"
            userMap.insert(VKImageCacheModel::Text, qtTrId("nemo_socialcache_VK_images_model-all-users"));
            userMap.insert(VKImageCacheModel::Count, count);
            userMap.insert(VKImageCacheModel::AccountId, 0);
            data.prepend(userMap);
        }
        break;
    }
    case Albums: {
        QList<VKAlbum::ConstPtr> albumsData = d->database.albums();
        Q_FOREACH (const VKAlbum::ConstPtr &albumData, albumsData) {
            QMap<int, QVariant> albumMap;
            albumMap.insert(VKImageCacheModel::AlbumId, albumData->id());
            albumMap.insert(VKImageCacheModel::Text, albumData->title());
            albumMap.insert(VKImageCacheModel::Count, albumData->size());
            albumMap.insert(VKImageCacheModel::UserId, albumData->ownerId());
            albumMap.insert(VKImageCacheModel::AccountId, albumData->accountId());
            data.append(albumMap);
        }

        if (data.count() > 1) {
            QVariantMap parsedNodeIdentifier = parseNodeIdentifier(d->nodeIdentifier);

            QMap<int, QVariant> albumMap;
            int count = 0;
            Q_FOREACH (const VKAlbum::ConstPtr &albumData, albumsData) {
                count += albumData->size();
            }

            albumMap.insert(VKImageCacheModel::AlbumId, QString());
            //:  Label for the "show all photos from all albums by this user" option
            //% "All"
            albumMap.insert(VKImageCacheModel::Text, qtTrId("nemo_socialcache_VK_images_model-all-albums"));
            albumMap.insert(VKImageCacheModel::Count, count);
            albumMap.insert(VKImageCacheModel::UserId, parsedNodeIdentifier.value("user_id").toString());
            albumMap.insert(VKImageCacheModel::AccountId, parsedNodeIdentifier.value("accountId").toInt());
            data.prepend(albumMap);
        }
        break;
    }
    case Images: {
        QList<VKImage::ConstPtr> imagesData = d->database.images();

        for (int i = 0; i < imagesData.count(); i ++) {
            const VKImage::ConstPtr &imageData = imagesData.at(i);
            QMap<int, QVariant> imageMap;
            if (imageData->thumbFile().isEmpty()) {
                QVariantMap thumbQueueData;
                thumbQueueData.insert(QLatin1String(ROW_KEY), QVariant::fromValue<int>(i));
                thumbQueueData.insert(QLatin1String(TYPE_KEY), QVariant::fromValue<int>(VKImageDownloader::ThumbnailImage));
                thumbQueueData.insert(QLatin1String(ACCOUNTID_KEY), imageData->accountId());
                thumbQueueData.insert(QLatin1String(OWNERID_KEY), imageData->ownerId());
                thumbQueueData.insert(QLatin1String(ALBUMID_KEY), imageData->albumId());
                thumbQueueData.insert(QLatin1String(PHOTOID_KEY), imageData->id());
                thumbQueueData.insert(QLatin1String(URL_KEY), imageData->thumbSrc());
                thumbQueue.append(thumbQueueData);
            }
            // note: we don't queue the image file until the user explicitly opens that in fullscreen.
            imageMap.insert(VKImageCacheModel::PhotoId, imageData->id());
            imageMap.insert(VKImageCacheModel::AlbumId, imageData->albumId());
            imageMap.insert(VKImageCacheModel::UserId, imageData->ownerId());
            imageMap.insert(VKImageCacheModel::AccountId, imageData->accountId());
            imageMap.insert(VKImageCacheModel::Thumbnail, imageData->thumbFile());
            imageMap.insert(VKImageCacheModel::Image, imageData->photoFile());
            imageMap.insert(VKImageCacheModel::Text, imageData->text());
            imageMap.insert(VKImageCacheModel::Date, imageData->date());
            imageMap.insert(VKImageCacheModel::Width, imageData->width());
            imageMap.insert(VKImageCacheModel::Height, imageData->height());
            imageMap.insert(VKImageCacheModel::MimeType, QLatin1String("image/jpeg"));
            imageMap.insert(VKImageCacheModel::ImageSource, imageData->photoSrc());
            data.append(imageMap);
        }
        break;
    }
    default:
        return;
    }

    updateData(data);

    // now download the queued thumbnails.
    foreach (const QVariantMap &thumbQueueData, thumbQueue) {
        d->queue(thumbQueueData[QLatin1String(ROW_KEY)].toInt(),
                 static_cast<VKImageDownloader::ImageType>(thumbQueueData[QLatin1String("TYPE_KEY")].toInt()),
                 thumbQueueData[QLatin1String(ACCOUNTID_KEY)].toInt(),
                 thumbQueueData[QLatin1String(OWNERID_KEY)].toString(),
                 thumbQueueData[QLatin1String(ALBUMID_KEY)].toString(),
                 thumbQueueData[QLatin1String(PHOTOID_KEY)].toString(),
                 thumbQueueData[QLatin1String(URL_KEY)].toString());
    }
}
