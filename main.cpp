#include <QFile>
#include <QTextStream>
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file(":/RES/style");
    if(file.open(QFile::ReadOnly))
    {
        QTextStream qts(&file);
        QString styles = qts.readAll();
        a.setStyleSheet(styles);
    }

    MainWindow mainWindow;
    mainWindow.show();
    return a.exec();
}
