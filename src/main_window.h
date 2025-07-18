#pragma once

#include "can_sniffer.hpp"

#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QDir>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr)
        : QMainWindow{ parent }
    {
        QToolBar* toolbar = addToolBar("Toolbar");
        toolbar->setMovable(false);

        connect_button = new QPushButton("Connect", this);
        select_device_combo = new QComboBox{this};
        choose_dbc_button = new QPushButton("Choose DBC", this);

        QWidgetAction* connect_action = new QWidgetAction(this);
        connect_action->setDefaultWidget(connect_button);
        QWidgetAction* select_device_action = new QWidgetAction(this);
        select_device_action->setDefaultWidget(select_device_combo);
        QWidgetAction* choose_dbc_action = new QWidgetAction(this);
        choose_dbc_action->setDefaultWidget(choose_dbc_button);

        toolbar->addAction(connect_action);
        toolbar->addAction(select_device_action);
        toolbar->addAction(choose_dbc_action);

        dbc_label = new QLabel{this};
        (void)toolbar->addWidget(dbc_label);

        QTableView* table_view = new QTableView{this};
        cf = new CanSniffer{table_view, this};

        connect_button->setCheckable(true);
        connect_button->setEnabled(false);

        // Set default, non-selectable text for the combo box
        select_device_combo->addItem("Select device");
        select_device_combo->addItems(cf->available_devices());
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(select_device_combo->model());
        QModelIndex first_index = model->index(0, select_device_combo->modelColumn(), select_device_combo->rootModelIndex());
        QStandardItem* first_item = model->itemFromIndex(first_index);
        first_item->setSelectable(false);
        first_item->setEnabled(false);

        QObject::connect(
            connect_button, &QPushButton::clicked,
            this, &MainWindow::connect_button_pushed);
        QObject::connect(
            select_device_combo, &QComboBox::activated,
            this, &MainWindow::select_device_combo_clicked);
        QObject::connect(
            choose_dbc_button, &QPushButton::clicked,
            this, &MainWindow::choose_dbc_button_pushed);

        setCentralWidget(table_view);
        setWindowTitle("CAN sniffer");
    }

public slots:
    void connect_button_pushed(bool checked) {
        if (checked)
        {
            cf->connect_device();
            select_device_combo->setEnabled(false);
            choose_dbc_button->setEnabled(false);
        }
        else
        {
            cf->disconnect_device();
            select_device_combo->setEnabled(true);
            choose_dbc_button->setEnabled(true);
        }
    }

    void select_device_combo_clicked(int index) {
        if (index > 0 && cf->select_device(select_device_combo->currentText()))
            connect_button->setEnabled(true);
    }

    void choose_dbc_button_pushed(bool checked) {
        (void)checked;

        QString dbc_file = QFileDialog::getOpenFileName(this, "Open DBC", QDir::homePath(), "DBC Files (*.dbc)");
        if (!dbc_file.isNull())
        {
            cf->parse_dbc(dbc_file);
            dbc_label->setText(dbc_file);
        }
    }

private:
    CanSniffer* cf;
    QPushButton* connect_button;
    QComboBox* select_device_combo;
    QPushButton* choose_dbc_button;
    QLabel* dbc_label;
};
