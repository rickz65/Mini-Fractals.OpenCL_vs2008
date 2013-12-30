CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(control_panel_plugin_mf_test01)
TEMPLATE    = lib

HEADERS     = control_panelplugin.h \
    controlpanel.h
SOURCES     = control_panelplugin.cpp \
    controlpanel.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += designer
} else {
    CONFIG += designer
}

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(control_panel.pri)

FORMS += \
    controlpanel.ui
