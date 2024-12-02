TEMPLATE = subdirs
SUBDIRS = lib

CONFIG(qml): {
    SUBDIRS += qml
    qml.depends = lib
}

