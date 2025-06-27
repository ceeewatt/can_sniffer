#include "model.hpp"

static QString get_signal_values_string(const VisualFrame& vf)
{
    QString ret;

    if (vf.signal_values.isEmpty())
        return ret;

    for (auto it = vf.signal_values.cbegin(), end = vf.signal_values.cend(); it != end; ++it)
    {
        QString sig_name = it.key();
        QString sig_value = (*it).toString();
        QString unit = vf.physical_units.value(sig_name).toString();
        ret.append(QString{ "%1 = %2 %3, " }.arg(sig_name).arg(sig_value).arg(unit));
    }

    if (!ret.isEmpty())
        ret.chop(2);

    return ret;
}

static QString get_payload_string(const VisualFrame& vf)
{
    QStringList hexList;

    for (uint8_t byte : vf.payload) {
        hexList << QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
    }

    return hexList.join(' ');
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return datastore.size();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 8;
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant{};

    const VisualFrame& this_frame = datastore.at(index.row());

    switch (index.column())
    {
    case 0:
        return QString{ "%1" }.arg(this_frame.timestamp);
    case 1:
        return this_frame.extended ?
               QString{ "0x" } + QString{ "%1" }.arg(this_frame.id, 0, 16).toUpper() :
               QString{};
    case 2:
        return this_frame.extended ?
               QString{ "0x" } + QString{ "%1" }.arg(this_frame.src, 0, 16).toUpper() :
               QString{};
    case 3:
        return this_frame.extended ?
               QString{ "0x" } + QString{ "%1" }.arg(this_frame.dst, 0, 16).toUpper() :
               QString{};
    case 4:
        return QString{ "%1" }.arg(this_frame.pri);
    case 5:
        return QString{ "%1" }.arg(this_frame.len);
    case 6:
        return get_signal_values_string(this_frame);
    case 7:
        return get_payload_string(this_frame);
    default:
        return QString{};
    }
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant{};

    switch (section)
    {
    case 0:
        return QString{ "Timestamp" };
    case 1:
        return QString{ "ID" };
    case 2:
        return QString{ "src" };
    case 3:
        return QString{ "dst" };
    case 4:
        return QString{ "pri" };
    case 5:
        return QString{ "len" };
    case 6:
        return QString{ "signals" };
    case 7:
        return QString{ "data" };
    default:
        return QString{};
    }
}

bool Model::insertFrames(const QList<VisualFrame>& vf, const QModelIndex &parent)
{
    if (vf.isEmpty())
        return false;

    int first = rowCount();
    int last = first + (vf.size() - 1);

    beginInsertRows(parent, first, last);
    datastore.append(vf);
    endInsertRows();

    return true;
}
