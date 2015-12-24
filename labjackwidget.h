#ifndef LABJACKWIDGET_H
#define LABJACKWIDGET_H

#include <QWidget>
#include <QThread>

#include "labjackthread.h"

namespace Ui {
class LabJackWidget;
}

class LabJackWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LabJackWidget(QWidget *parent = 0);
    ~LabJackWidget();

signals:
    void connectLabJack(); // open connection
    void initializeLabJack(const unsigned int samplesPerSec); // initialize settings
    void startAcquisition(); // start timer
    void stopAcquisition(); // stop timer
    void disconnectLabJack(); // disconnect

private slots:
    void workerStatusChanged(int status);

    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_initializeLJbutton_clicked();

    void on_startRecordButton_clicked();

    void on_stopRecordButton_clicked();

private:
    Ui::LabJackWidget *ui;

    QThread m_thread; // LabJack Thread will live in here
    LabJackThread *m_worker;
};

#endif // LABJACKWIDGET_H