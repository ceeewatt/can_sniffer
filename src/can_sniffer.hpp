#pragma once

extern "C" {
    #include "j1939.h"

    struct CanIdConverter {
        uint8_t pri;
        uint8_t dp;
        uint8_t pf;
        uint8_t ps;
        uint8_t sa;
        uint32_t pgn;
    };
}

#include "model_buffer.hpp"

#include <QTableView>
#include <QCanBus>
#include <QTimer>
#include <QCanDbcFileParser>
#include <QCanFrameProcessor>
#include <QHash>
#include <QCanMessageDescription>

class CanSniffer : public QObject
{
    Q_OBJECT

public:
    enum Plugin {
        socketcan,
        peakcan,
        vectorcan,
        none
    };

public:
    CanSniffer(QTableView* tv, QObject* parent = nullptr);

public slots:
    // TODO: connect to appropriate widget signals
    QList<QString> available_devices();
    bool select_device(const QString& name);
    bool connect_device();
    void disconnect_device();
    void parse_dbc(const QString& filename);

public:
    static bool can_rx_callback(struct J1939CanFrame* jframe);
    static bool can_tx_callback(struct J1939Msg* msg);
    static void j1939_msg_rx_callback(struct J1939Msg* msg);
    static void j1939_startup_delay_callback(void* param);

private:
    QCanBusDevice* device;
    ModelBuffer buffer;
    Plugin plugin;
    qint64 reference_time_ms;
    QCanDbcFileParser dbc_parser;
    QCanFrameProcessor frame_processor;
    QList<QCanMessageDescription> msg_database;
    QHash<quint32, const QCanMessageDescription*> pgn_database;
    void add_qframe_to_buffer(const QCanBusFrame& qframe);
    void add_j1939_msg_to_buffer(const J1939Msg* msg);
    Plugin get_plugin(const QCanBusDeviceInfo& dev);
    double get_timestamp(const QCanBusFrame::TimeStamp& timestamp);

private:
    static constexpr uint8_t j1939_address{ 0x00 };
    static constexpr int j1939_tick_rate_ms{ 10 };

private:
    J1939 j1939;
    J1939Name j1939_name;
    QTimer j1939_timer;
    const QList<quint32> pgn_whitelist{ J1939_TP_CM_PGN, J1939_TP_DT_PGN };
    void j1939_update();
    bool can_rx(struct J1939CanFrame* jframe);

};
