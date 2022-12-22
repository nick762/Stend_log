#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QSerialPortInfo>
#include <QtCore/QTime>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include "qcustomplot.h"
#include "json.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <port.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Charge_clicked();
    void on_Discharge_clicked();
    void serialFind();
    void on_OK_clicked();
    void read(QString data);
    void intToYear(QString date);
    void on_cycling_clicked(bool checked);
    void on_graph_clicked();
    void toFile(QList<QString> value);
    void Print(QString);
    void on_setSerial_clicked();
    QList<QString> wordToByte(QString dat);
    QList<QString> decToBin(QList<QString> dat);
    void status(QString stat);
    void on_setDate_clicked();
    void on_seal_clicked();
    void on_unseal_clicked();
    void on_fullAccess_clicked();
    void on_SerialFind_clicked();
    void on_Capacity_clicked(bool checked);
    void on_Stop_clicked();

    void on_Start_clicked();

private:
    Ui::MainWindow *ui;
    //QSerialPort *port;
    QWidget *widget;
    QCustomPlot *customPlot, *customPlot2;
    QCPGraph *graphic, *graphic2;
    QFileDialog *fDialog;
    Port *PortNew;
    QThread *thread_New;
    bool cycled = false;
    bool sn = true;
    bool dat = true;
    bool serial = true;
    bool hasFile = false;
    QString rest = "";
    double minElem(QVector<double> input);
    double maxElem(QVector<double> input);
    uint16_t cDate;
    QDate aDate;
    //QSqlDatabase myDb;
    /*enum class Command quint8;
    enum class Command : quint8
    {
        c = 0x63,
        d = 0x64,
        y = 0x79,
        l = 0x6c,
        u = 0x75,
        f = 0x66,
        e = 0x65,
        s = 0x73,
        k = 0x6b
    };*/

signals:
    void hasData(QList<QString> val);
    void savesettings(QString name);
    void writeData(QByteArray data);
    void start();
    void error_(QString err);
};

#endif // MAINWINDOW_H
