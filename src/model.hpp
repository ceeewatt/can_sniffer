#pragma once

#include <QVariant>
#include <QAbstractTableModel>

struct VisualFrame
{
    double timestamp;
    quint32 id;
    quint8 src;
    quint8 dst;
    quint8 pri;
    quint16 len;
    QVariantMap signal_values;
    QByteArray payload;
    QVariantMap physical_units;
};

class Model : public QAbstractTableModel
{
    Q_OBJECT

public:
    Model(QObject* parent = nullptr)
        : QAbstractTableModel{ parent }
    {}

    // Virtual functions provided by base class, to be used by connected view
    //  for fetching/displaying data.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public:
    bool insertFrames(const QList<VisualFrame>& vf, const QModelIndex &parent = QModelIndex());

private:
    QList<VisualFrame> datastore;
};
