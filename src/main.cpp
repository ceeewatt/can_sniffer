#include "can_sniffer.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QDebug>

int main(int argc, char* argv[])
{
    // TODO: temporary debugging
    if (argc < 3)
        return -1;
    QString device_name{ argv[1] };
    QString dbc_file_name{ argv[2] };

    QApplication app{ argc, argv };
    QTableView table_view;
    CanSniffer cf{ table_view };

    QFileInfo dbc_file{ dbc_file_name };
    if (dbc_file.exists())
        cf.parse_dbc(dbc_file.absoluteFilePath());

    if (!cf.select_device(device_name))
        qDebug() << "Error selecting device.";
    
    if (!cf.connect_device())
        qDebug() << "Error connecting device.";

    table_view.show();

    return app.exec();
}
