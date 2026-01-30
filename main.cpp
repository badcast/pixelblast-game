#include <QApplication>

#include "PixelBlastGame.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PixelBlast b;
    b.resize(1024,768);
    b.startGame();
    b.show();
    return a.exec();
}
