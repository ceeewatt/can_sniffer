#include <QApplication>
#include <QFileInfo>
#include <QDebug>
#include "main_window.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };
    MainWindow main_window;
    main_window.show();

    return app.exec();
}
