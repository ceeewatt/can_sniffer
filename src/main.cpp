#include "can_sniffer.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };

    QTableView table_view;
    CanSniffer cf{ table_view };

    QFileInfo dbc{ "sample.dbc" };

    if (dbc.exists())
        cf.parse_dbc(dbc.absoluteFilePath());

    if (!cf.select_device(QStringLiteral("vcan0")))
        qDebug() << "Error selecting device.";
    
    if (!cf.connect_device())
        qDebug() << "Error connecting device.";

    table_view.show();

    return app.exec();
}
