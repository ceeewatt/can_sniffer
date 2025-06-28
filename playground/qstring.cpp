#include <QString>
#include <QDebug>

int main(void)
{
    quint32 id = 0x11223344;

    QString temp = QString{ "%1" }.arg(id, 0, 16);

    qDebug() << temp;

    return 0;
}
