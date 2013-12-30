#include "control_panel.h"
#include "control_panelplugin.h"

#include <QtPlugin>

Control_PanelPlugin::Control_PanelPlugin(QObject *parent)
    : QObject(parent)
{
    m_initialized = false;
}

void Control_PanelPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool Control_PanelPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *Control_PanelPlugin::createWidget(QWidget *parent)
{
    return new Control_Panel(parent);
}

QString Control_PanelPlugin::name() const
{
    return QLatin1String("Control_Panel");
}

QString Control_PanelPlugin::group() const
{
    return QLatin1String("MFC-QT-Interconnect-Class");
}

QIcon Control_PanelPlugin::icon() const
{
    return QIcon(QLatin1String(":/Mini-Fractals-logo-128x128.ico"));
}

QString Control_PanelPlugin::toolTip() const
{
    return QLatin1String("QT Control Panel");
}

QString Control_PanelPlugin::whatsThis() const
{
    return QLatin1String("This is a Control Panel of various controls.\nNote the Large Tab Control that unlike MFC designer in VS 2012, has a indepenent design-time work surface for each TAB in the TAB control ... WYSIWYG designer of GUI for TAB controls ... like in .net ... but not ... because if it has to be something else, let's make it cross-platform as much as possible, a hence the use of QT ... iPhone, Android, Windows Phone, Windows Desktop, OSX, and X-Windows+Linux+UNIX(linux is a penguin)\n");
}

bool Control_PanelPlugin::isContainer() const
{
    return true;
}

QString Control_PanelPlugin::domXml() const
{
    return QLatin1String("<widget class=\"Control_Panel\" name=\"control_Panel\">\n</widget>\n");
}

QString Control_PanelPlugin::includeFile() const
{
    return QLatin1String("control_panel.h");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(control_panel_plugin_mf_test01, Control_PanelPlugin)
#endif // QT_VERSION < 0x050000
