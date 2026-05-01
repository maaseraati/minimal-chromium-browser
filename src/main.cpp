#include "browserwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("Morphine"));
    QCoreApplication::setOrganizationName(QStringLiteral("Morphine"));

    QApplication app(argc, argv);

    BrowserWindow window;
    window.show();

    return QApplication::exec();
}
