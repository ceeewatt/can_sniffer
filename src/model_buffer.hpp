#pragma once

#include "model.hpp"

#include <QTimer>

class ModelBuffer : public Model
{
    Q_OBJECT

public:
    ModelBuffer(QObject* parent = nullptr)
        : Model{ parent }
    {
        QObject::connect(
            &flush_timer, &QTimer::timeout,
            this, &ModelBuffer::flush);
    }

    void enqueue(const VisualFrame& vf) {
        buffer.append(vf);
    }

    void start_timer() {
        flush_timer.start(timer_interval_ms);
    }

    void stop_timer() {
        flush_timer.stop();
    }

private:
    static constexpr int timer_interval_ms{ 100 };

private slots:
    void flush() {
        if (insertFrames(buffer))
            buffer.clear();
    }

private:
    QList<VisualFrame> buffer;
    QTimer flush_timer;
};
