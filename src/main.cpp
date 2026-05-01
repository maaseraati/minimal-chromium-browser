#include "browser_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(morphine::AppTitle);
    morphine::BrowserWindow window;
    window.show();
    return app.exec();
}
