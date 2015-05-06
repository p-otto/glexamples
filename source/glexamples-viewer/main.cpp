
#if defined(WIN32) && !defined(_DEBUG)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
#endif

#include <gloperate-qtapplication/AbstractApplication.h>

#include <QFileInfo>

#include <memory>

#include <gloperate-qtapplication/Viewer.h>

#include <widgetzeug/dark_fusion_style.hpp>

class Application : public gloperate_qtapplication::AbstractApplication
{
public:
    Application(int & argc, char ** argv)
        : AbstractApplication(argc, argv)
    {
        const QFileInfo fi(QCoreApplication::applicationFilePath());

        QApplication::setApplicationDisplayName(fi.baseName());

        QApplication::setApplicationName("glexamples");
        QApplication::setApplicationVersion("1.0.0");

        QApplication::setOrganizationName("HPI Computer Graphics Systems Group");
        QApplication::setOrganizationDomain("https://github.com/hpicgs");
    }

    virtual ~Application() = default;
};

int main(int argc, char * argv[])
{
    Application app(argc, argv);

    widgetzeug::enableDarkFusionStyle();

    gloperate_qtapplication::Viewer viewer;
    viewer.show();

    return app.exec();
}
