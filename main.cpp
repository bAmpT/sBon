#include "mainwindow.h"
#include "productmodel.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("TamTam-Soft");
    QCoreApplication::setApplicationName("sBon");

    qRegisterMetaType<Product>("Product");
    qRegisterMetaType<QList<Product>>("Products");

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "sBon_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.setWindowTitle("sBon");
    w.show();
    return a.exec();
}
