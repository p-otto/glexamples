
#if defined(WIN32) && !defined(_DEBUG)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
#endif


#include <memory>

#include <QFileInfo>

#include <gloperate-qt/viewer/Application.h>
#include <gloperate-qt/viewer/Viewer.h>

#include <widgetzeug/dark_fusion_style.hpp>

#include "glexamples-version.h"


class Application : public gloperate_qt::Application
{
public:
    Application(int & argc, char ** argv)
    : gloperate_qt::Application(argc, argv)
    {
        const QFileInfo fi(QCoreApplication::applicationFilePath());

        QApplication::setApplicationDisplayName(fi.baseName());

        QApplication::setApplicationName(GLEXAMPLES_PROJECT_NAME);
        QApplication::setApplicationVersion(GLEXAMPLES_VERSION);

        QApplication::setOrganizationName(GLEXAMPLES_AUTHOR_ORGANIZATION);
        QApplication::setOrganizationDomain(GLEXAMPLES_AUTHOR_DOMAIN);
    }

    virtual ~Application() = default;
};


int main(int argc, char * argv[])
{
    Application app(argc, argv);

    widgetzeug::enableDarkFusionStyle();

    gloperate_qt::Viewer viewer;
    viewer.show();

    return app.exec();
}
