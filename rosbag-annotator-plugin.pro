TEMPLATE = lib
TARGET = rosbagannotatorplugin
QT += qml quick multimedia
CONFIG += plugin c++11

TARGET = $$qtLibraryTarget($$TARGET)
uri = ch.epfl.chili.ros.annotation

# Input
SOURCES += \
        rosbagannotatorplugin.cpp \
        rosbagannotator.cpp \
        imageitem.cpp

HEADERS += \
        rosbagannotatorplugin.h \
        rosbagannotator.h \
        imageitem.h

INCLUDEPATH += /opt/ros/kinetic/include
LIBS += -L"/opt/ros/kinetic/lib" -lrosbag_storage -lroscpp_serialization

DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}