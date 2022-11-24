#include "ModelViewer.h"
#include "ViewWidget3D.h"

#include <QWindow>

#include <QtWidgets/QApplication>

#include <iostream>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ModelViewer w;
    w.show();

    return a.exec();
}
