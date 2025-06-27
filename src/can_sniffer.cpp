#include "can_sniffer.hpp"

#include <QDebug>
#include <QDateTime>
#include <QCanUniqueIdDescription>
#include <QCanFrameProcessor>
#include <QCanSignalDescription>

static CanSniffer* g_can_sniffer;

static uint64_t extract_raw_signal(const uint8_t* data, int bit_start, int bit_length, bool little_endian)
{
    // Assumption: maximum SPN bit length is 64
    if (bit_length > 64)
        return 0;

    // Little endian bit order: start bit is LSb
    // Big endian bit order: start bit is MSb
    // See: https://doc.qt.io/qt-6/qcansignaldescription.html#data-endianness-processing
    uint64_t raw = 0;
    if (little_endian)
    {
        for (int i = 0; i < bit_length; ++i)
        {
            int byte_idx = (bit_start + i) / 8;
            int bit_pos = i % 8;

            int this_bit = (data[byte_idx] >> bit_pos) & 1;
            raw = raw | (this_bit << i);
        }
    }
    else
    {
        for (int i = 0; i < bit_length; ++i)
        {
            int byte_idx = (bit_start + i) / 8;
            int bit_pos = i % 8;

            int this_bit = (data[byte_idx] >> bit_pos) & 1;
            raw = raw | (this_bit << (bit_length - i - 1));
        }
    }

    return raw;
}

double CanSniffer::get_timestamp(const QCanBusFrame::TimeStamp& timestamp)
{
    double timestamp_ms;

    switch (plugin)
    {
    case CanSniffer::Plugin::socketcan:
        timestamp_ms =
            static_cast<double>(timestamp.seconds() * 1000) +
            (static_cast<double>(timestamp.microSeconds()) / 1000);
        break;
    case CanSniffer::Plugin::peakcan:
    case CanSniffer::Plugin::vectorcan:
    case CanSniffer::Plugin::none:
        timestamp_ms = 0;
        break;
    }

    return ((timestamp_ms - reference_time_ms) / 1000);
}


/* ============================================================================
 *
 * Section: Member function definitions
 *
 * ============================================================================
 */

/* ============================================================================
 * Subsection: Special member definitions
 * ============================================================================
 */

CanSniffer::CanSniffer(QTableView& tv, QObject* parent)
    : QObject{ parent }
{
    g_can_sniffer = this;

    tv.setModel(&buffer);

    (void)j1939_init(
        &j1939,
        &j1939_name,
        CanSniffer::j1939_address,
        CanSniffer::j1939_tick_rate_ms,
        CanSniffer::can_rx_callback,
        CanSniffer::can_tx_callback,
        CanSniffer::j1939_msg_rx_callback,
        CanSniffer::j1939_startup_delay_callback,
        nullptr);

    QObject::connect(
        &j1939_timer, &QTimer::timeout,
        this, &CanSniffer::j1939_update);
}

/* ============================================================================
 * Subsection: Slot definitions
 * ============================================================================
 */

QList<QString> CanSniffer::available_devices()
{
    QString error_message;
    QList<QCanBusDeviceInfo> dev_list =
        QCanBus::instance()->availableDevices(&error_message);

    if (!error_message.isEmpty())
    {
        qDebug() << error_message;
        return QList<QString>{};
        
    }

    QList<QString> dev_names( dev_list.size() );
    for (const QCanBusDeviceInfo& dev : dev_list)
        dev_names.append(dev.name());

    return dev_names;
}

bool CanSniffer::select_device(const QString& name)
{
    QString error_message;
    QList<QCanBusDeviceInfo> dev_list =
        QCanBus::instance()->availableDevices(&error_message);

    if (!error_message.isEmpty())
    {
        qDebug() << error_message;
        return false;
    }

    for (const QCanBusDeviceInfo& dev : dev_list)
    {
        if (dev.name() == name)
        {
            this->device =
                QCanBus::instance()->createDevice(
                    dev.plugin(),
                    dev.name(),
                    &error_message);

            plugin = get_plugin(dev);
            
            if (!error_message.isEmpty())
            {
                qDebug() << error_message;
                return false;
            }

            return true;
        }
    }

    return false;
}

bool CanSniffer::connect_device()
{
    j1939_timer.start(CanSniffer::j1939_tick_rate_ms);
    buffer.start_timer();
    reference_time_ms = QDateTime::currentMSecsSinceEpoch();
    return device->connectDevice();
}

void CanSniffer::disconnect_device()
{
    j1939_timer.stop();
    buffer.stop_timer();
    device->disconnectDevice();
}

void CanSniffer::parse_dbc(const QString& filename)
{
    if (!dbc_parser.parse(filename))
    {
        qDebug() << dbc_parser.errorString();
        return;
    }

    msg_database = dbc_parser.messageDescriptions();
    pgn_database.clear();

    CanIdConverter converter;
    for (const QCanMessageDescription& msg_desc : msg_database)
    {
        j1939_can_id_converter(
            &converter,
            static_cast<uint32_t>(msg_desc.uniqueId()));

        // Assumption: a PGN value of 0 is an invalid PGN. 
        // This is to ensure only CAN2.0B frames get added.
        if (converter.pgn != 0)
            (void)pgn_database.insertOrAssign(converter.pgn, &msg_desc);
    }

    // Initialize frame processor for decoding DBC frames
    frame_processor.setUniqueIdDescription(QCanDbcFileParser::uniqueIdDescription());
    frame_processor.setMessageDescriptions(dbc_parser.messageDescriptions());
}

/* ============================================================================
 * Subsection: Member function definitions
 * ============================================================================
 */

void CanSniffer::j1939_update()
{
    ::j1939_update(&j1939);
}

bool CanSniffer::can_rx(struct J1939CanFrame* jframe)
{
    static CanIdConverter can_id_converter;

    if (!device->framesAvailable())
        return false;

    QCanBusFrame qframe = device->readFrame();

    bool ret = false;
    if (qframe.hasExtendedFrameFormat())
    {
        (void)j1939_can_id_converter(&can_id_converter, qframe.frameId());
        quint32 pgn = can_id_converter.pgn;

        if (pgn_whitelist.contains(pgn))
        {
            jframe->id = qframe.frameId() | (1 << 31);
            jframe->len = qframe.payload().size();
            std::memcpy(jframe->data, qframe.payload().data(), jframe->len);
            ret = true;
        }
    }

    add_qframe_to_buffer(qframe);
    return false;
}

bool CanSniffer::j1939_msg_rx(struct J1939Msg* msg)
{
    (void)msg;
    return false;
    return ret;
}

void CanSniffer::add_qframe_to_buffer(const QCanBusFrame& qframe)
{
    CanIdConverter converter;
    quint8 dst{};
    QVariantMap signal_values;
    QVariantMap physical_units;
    bool extended;
    if (qframe.hasExtendedFrameFormat())
    {
        extended = true;
        if (j1939_can_id_converter(&converter, qframe.frameId()))
            dst = J1939_ADDR_GLOBAL;
        else
            dst = converter.ps;

        if (pgn_database.contains(converter.pgn))
        {
            // TODO: could cause issues if this lookup relies on matching against
            //  CAN ID. The CAN ID in the DBC won't necessarily reflect the CAN ID
            //  on the bus if the node addresses aren't static.
            signal_values = frame_processor.parseFrame(qframe).signalValues;
            const QCanMessageDescription* msg_desc = pgn_database.value(converter.pgn);

            // Create a mapping of signal name <-> physical unit, with the same key order as
            //  signal_values.
            for (auto it = signal_values.cbegin(), end = signal_values.cend(); it != end; ++it)
            {
                QString name = it.key();

                physical_units[name] = msg_desc->signalDescriptionForName(name).physicalUnit();
            }
        }
    }
    else
    {
        // Note: Purposefully not parsing CAN2.0A frames.
        extended = false;
    }

    VisualFrame vf {
        .timestamp = get_timestamp(qframe.timeStamp()),
        .id = static_cast<quint32>(qframe.frameId()),
        .src = converter.sa,
        .dst = dst,
        .pri = converter.pri,
        .len = static_cast<quint16>(qframe.payload().size()),
        .signal_values = signal_values,
        .payload = qframe.payload(),
        .physical_units = physical_units,
        .extended = extended
    };

    buffer.enqueue(vf);
}

void CanSniffer::add_j1939_msg_to_buffer(const J1939Msg* msg)
{
    // Manual decoding of J1939 Transport Protocol messages b/c Qt CAN Bus API
    //  doesn't support multi-packet (> 8 byte payload) message parsing.

    QVariantMap signal_values;
    QVariantMap physical_units;
    QList<QString> physical_units_old;
    if (pgn_database.contains(msg->pgn))
    {
        QList<QCanSignalDescription> signal_desc = pgn_database.value(msg->pgn)->signalDescriptions();

        uint64_t raw{};
        for (const QCanSignalDescription& sig : signal_desc)
        {
            raw = extract_raw_signal(msg->data, sig.startBit(), sig.bitLength(), sig.dataEndian() == QSysInfo::LittleEndian);

            double factor = (sig.factor() != 0 && sig.factor() != qQNaN()) ? sig.factor() : 1;
            double scaling = (sig.scaling() != 0 && sig.scaling() != qQNaN()) ? sig.scaling() : 1;
            double offset = (sig.offset() != qQNaN()) ? sig.offset() : 0;

            double physical_value = scaling * (raw * factor + offset);
            if (sig.maximum() != qQNaN() && physical_value > sig.maximum())
                physical_value = sig.maximum();
            else if (sig.minimum() != qQNaN() && physical_value < sig.minimum())
                physical_value = sig.minimum();

            // TODO: not handling multiplexed signals... no Qt documentation yet?

            physical_units[sig.name()] = sig.physicalUnit();
        }
    }
    
    VisualFrame vf {
        .timestamp = (QDateTime::currentMSecsSinceEpoch() - reference_time_ms) / 1000.0,
        .id = static_cast<quint32>(j1939_msg_to_can_id(msg)),
        .src = msg->src,
        .dst = msg->dst,
        .pri = msg->pri,
        .len = msg->len,
        .signal_values = signal_values,
        .payload = QByteArray{ reinterpret_cast<char*>(msg->data), msg->len },
        .physical_units = physical_units,
        .extended = true
    };

    buffer.enqueue(vf);
}


CanSniffer::Plugin CanSniffer::get_plugin(const QCanBusDeviceInfo& dev)
{
    QString plugin_name = dev.plugin();

    if (plugin_name == "socketcan")
    {
        return Plugin::socketcan;
    }
    else
    {
        // TODO: other plugins
        return Plugin::none;
    }
}

/* ============================================================================
 *
 * Section: Static function definitions
 *
 * ============================================================================
 */

bool CanSniffer::can_rx_callback(struct J1939CanFrame *jframe)
{
    return g_can_sniffer->can_rx(jframe);
}

bool CanSniffer::can_tx_callback(struct J1939Msg* msg)
{
    (void)msg;
    return false;
}

void CanSniffer::j1939_msg_rx_callback(struct J1939Msg* msg)
{
    g_can_sniffer->j1939_msg_rx(msg);
}

void CanSniffer::j1939_startup_delay_callback(void* param)
{
    (void)param;
}
