include(../../common.pri)

TEMPLATE = lib
TARGET = socialcacheqml
TARGET = $$qtLibraryTarget($$TARGET)

MODULENAME = org/nemomobile/socialcache
TARGETPATH = $$[QT_INSTALL_QML]/$$MODULENAME

LIBS += -L../lib -lsocialcache
INCLUDEPATH += ../lib/


QT += gui qml sql network dbus
CONFIG += plugin

CONFIG(nodeps):{
DEFINES += NO_DEPS
} else {
CONFIG += link_pkgconfig
PKGCONFIG += buteosyncfw$${QT_MAJOR_VERSION} libsailfishkeyprovider
HEADERS += synchelper.h \
    keyproviderhelper.h
SOURCES += synchelper.cpp \
    keyproviderhelper.cpp
}


HEADERS += \
    abstractsocialcachemodel.h \
    abstractsocialcachemodel_p.h \
    postimagehelper_p.h \
    synchronizelists_p.h \
    facebook/facebookimagecachemodel.h \
    facebook/facebookimagedownloader.h \
    facebook/facebookimagedownloader_p.h \
    facebook/facebookimagedownloaderconstants_p.h \
    facebook/facebookpostsmodel.h \
    facebook/facebooknotificationsmodel.h \
    twitter/twitterpostsmodel.h \
    generic/socialimagedownloader.h \
    generic/socialimagedownloader_p.h \
    onedrive/onedriveimagedownloader_p.h \
    onedrive/onedriveimagedownloaderconstants_p.h \
    onedrive/onedriveimagedownloader.h \
    onedrive/onedriveimagecachemodel.h \
    dropbox/dropboximagecachemodel.h \
    dropbox/dropboximagedownloader.h \
    dropbox/dropboximagedownloader_p.h \
    dropbox/dropboximagedownloaderconstants_p.h \
    vk/vkimagecachemodel.h \
    vk/vkimagedownloader.h \
    vk/vkimagedownloader_p.h \
    vk/vkpostsmodel.h

SOURCES += plugin.cpp \
    abstractsocialcachemodel.cpp \
    facebook/facebookimagecachemodel.cpp \
    facebook/facebookimagedownloader.cpp \
    facebook/facebookpostsmodel.cpp \
    facebook/facebooknotificationsmodel.cpp \
    twitter/twitterpostsmodel.cpp \
    generic/socialimagedownloader.cpp \
    onedrive/onedriveimagedownloader.cpp \
    onedrive/onedriveimagecachemodel.cpp \
    dropbox/dropboximagecachemodel.cpp \
    dropbox/dropboximagedownloader.cpp \
    vk/vkimagecachemodel.cpp \
    vk/vkimagedownloader.cpp \
    vk/vkpostsmodel.cpp

OTHER_FILES += qmldir plugins.qmltypes
import.files = qmldir plugins.qmltypes

import.path = $$TARGETPATH
target.path = $$TARGETPATH

INSTALLS += target import

qmltypes.commands = qmlplugindump -nonrelocatable org.nemomobile.socialcache 1.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

# translations
TS_FILE = $$OUT_PWD/socialcache.ts
EE_QM = $$OUT_PWD/socialcache_eng_en.qm

qtPrepareTool(LUPDATE, lupdate)

ts.commands += $$LUPDATE $$PWD/.. -ts $$TS_FILE
ts.CONFIG += no_check_exist no_link
ts.output = $$TS_FILE
ts.input = ..

ts_install.files = $$TS_FILE
ts_install.path = /usr/share/translations/source
ts_install.CONFIG += no_check_exist

qtPrepareTool(LRELEASE, lrelease)

# should add -markuntranslated "-" when proper translations are in place (or for testing)
engineering_english.commands += $${LRELEASE} -idbased $$TS_FILE -qm $$EE_QM
engineering_english.CONFIG += no_check_exist no_link
engineering_english.depends = ts
engineering_english.input = $$TS_FILE
engineering_english.output = $$EE_QM

engineering_english_install.path = /usr/share/translations
engineering_english_install.files = $$EE_QM
engineering_english_install.CONFIG += no_check_exist

QMAKE_EXTRA_TARGETS += ts engineering_english
PRE_TARGETDEPS += ts engineering_english
INSTALLS += ts_install engineering_english_install
