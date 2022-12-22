#include "mainwindow.h"
#include "ui_material_batt.h"

//#include "ui_mainwindow.h"

#define _WIN32_WINNT 0x0A00
#define version 1.8


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile styleF(":css/css/dark.css");

    if(!styleF.open(QFile::ReadOnly)){
        qDebug()<<"can't open style";
    }else{
        QString qssStr = styleF.readAll();
        qApp->setStyleSheet(qssStr);
    }
    ui->dateEdit->setDate(QDate::currentDate());
    serialFind();

    //qDebug()<<_WIN32_WINNT;
    ui->ver->setText("Ver: "+QString::number(version));

    serialFind();
    QThread *thread_New = new QThread;
   // thread_New = new QThread; //изменения
    Port *PortNew = new Port();
    PortNew->moveToThread(thread_New);
    PortNew->thisPort.moveToThread(thread_New);
    connect(PortNew, SIGNAL(error_(QString)), this, SLOT(Print(QString)));
    connect(thread_New, SIGNAL(started()), PortNew, SLOT(process_Port()));

    connect(PortNew, SIGNAL(finished_Port()), thread_New, SLOT(quit()));
    connect(thread_New, SIGNAL(finished()), PortNew, SLOT(deleteLater()));
    connect(PortNew, SIGNAL(finished_Port()), thread_New, SLOT(deleteLater()));

    connect(this,SIGNAL(savesettings(QString)),PortNew,SLOT(Write_Settings_Port(QString)));
    connect(ui->OK, SIGNAL(clicked()),PortNew,SLOT(ConnectPort()));
    //connect(ui->pushButton_2, SIGNAL(clicked()),PortNew,SLOT(DisconnectPort()));
    connect(PortNew, SIGNAL(outPort(QString)), this, SLOT(read(QString)));
    connect(this,SIGNAL(writeData(QByteArray)),PortNew,SLOT(WriteToPort(QByteArray)));
    connect(this,SIGNAL(cData()), PortNew, SLOT(ClearData()));
    thread_New->start();
    qDebug()<<_WIN32_WINNT;
    ui->ver->setText("Ver: "+QString::number(version));

}

MainWindow::~MainWindow(){
    thread_New->quit();
    thread_New->wait(1000);
    delete ui;
}

void MainWindow::serialFind(){
    QSerialPortInfo info;
    ui->chooseSerial->clear();
    foreach (info ,QSerialPortInfo::availablePorts()) {
        ui->chooseSerial->addItem(info.portName());
        //qDebug() << info.portName();
    }
}

void MainWindow::on_OK_clicked()
{
    if(serial==true){
         savesettings(ui->chooseSerial->currentText());
         ui->Discharge->setEnabled(true);
         ui->Charge->setEnabled(true);
         ui->cycling->setEnabled(true);
         ui->graph->setEnabled(true);
         ui->seal->setEnabled(true);
         ui->unseal->setEnabled(true);
         ui->fullAccess->setEnabled(true);
         ui->setSerial->setEnabled(true);
         ui->setDate->setEnabled(true);
         ui->graph->setEnabled(true);
         connect(this, &MainWindow::hasData,this, &MainWindow::toFile);
     }
        qDebug()<<"Chosen port: "<<ui->chooseSerial->currentText();
        serial=false;
}

void MainWindow::on_Start_clicked()
{
    ui->Discharge->setEnabled(true);
    ui->Charge->setEnabled(true);
    ui->cycling->setEnabled(true);
    //ui->graph->setEnabled(true);
    ui->seal->setEnabled(true);
    ui->unseal->setEnabled(true);
    ui->fullAccess->setEnabled(true);
    ui->setSerial->setEnabled(true);
    ui->setDate->setEnabled(true);
    ui->graph->setEnabled(true);
}

void MainWindow::read(QString data)
{
    QList<QString> dates;
    char c = ',';
    byte num = 0;
    for(int i=0;i<data.length();i++){
        if(data[i] == c){
            num ++;
        }
    }
    for(int i=0;i<num+1;i++){
        QString vl = data.split(",")[i];
        dates.append(vl);
    }
    if(!dates.isEmpty()){
        emit hasData(dates);
        dates[15].truncate(1);
        qDebug()<<dates;
        if((dates[0]!=dates[1])&&(dates[0]!=dates[3])&&(dates[0]!=dates[4])&&(dates[0]!=dates[5])){
            ui->Vol->display(dates[0]);
        }else{
            qDebug() << "Voltage: " << dates[0] <<endl;
        }
        if((dates[1]!=dates[0])&&(dates[1]!=dates[3])&&(dates[1]!=dates[4])&&(dates[1]!=dates[5])){
            ui->Cur->display(dates[1]);
        }else{
            qDebug() << "Current: " << dates[1] <<endl;
        }
        bool ok = true;
        ui->Temp->display(dates[2]);

        if(ui->Capacity->isChecked()){
            ui->Cap->display(dates[11]);
        }else{
            ui->Cap->display(dates[3]);
        }

        ui->Cell1_V->display(dates[4]);
        ui->Cell2_V->display(dates[5]);
        ui->Cell3_V->display(dates[6]);
        ui->Cell4_V->display(dates[7]);
        if(sn == true){
            ui->serial->setText(dates[8]);
            sn = false;
        }
        if((dates[8]==rest)&&(sn==false)){
            ui->serial->setText(dates[8]);
            rest = "";
            sn = false;
        }

        if(dat == true){
            intToYear(dates[10]);
            ui->dateEdit->setDate(aDate);
            dat = false;
        }
        if((dates[10].toInt(&ok)==cDate)&&(dat==false)){
            intToYear(dates[10]);
            ui->dateEdit->setDate(aDate);
            cDate = 0;
            dat = false;
        }

        if((dates[9]=="1")||(dates[9]=="2")){
            ui->FA->setStyleSheet("QLabel {background: #FF5722}");
            ui->SS->setStyleSheet("QLabel {background: #FF5722}");
        }else{
            status(decToBin(wordToByte(dates[9]))[1]);
        }

        //qDebug()<<"discharge "<<ui->Discharge->isChecked();
        //qDebug()<<"charge "<<ui->Charge->isChecked();

        /*if((ui->Discharge->isChecked()) && (dates[1].toInt() < -5) && (dates[15] == "0")){
            ui->Discharge->setText("Разряд");
            ui->Charge->setEnabled(true);
            ui->cycling->setEnabled(true);
        }else if((ui->Discharge->isChecked()) && (dates[15] == "1")){
            ui->Discharge->setText("Остановить разряд");
            ui->Charge->setEnabled(false);
            ui->cycling->setEnabled(false);
        }else if(!(ui->Discharge->isChecked()) && (dates[1].toInt() < -5) && (dates[15] == "0")){
            ui->Discharge->setText("Разряд");
            ui->Charge->setEnabled(true);
            ui->cycling->setEnabled(true);
        }

        if((ui->Charge->isChecked()) && (dates[1].toInt() <= 0) && (dates[14] == "0")){
            ui->Charge->setText("Заряд");            
            ui->Discharge->setEnabled(true);
            ui->cycling->setEnabled(true);
        }else if((ui->Charge->isChecked() && (dates[14]) == "1")){
            ui->Charge->setText("Остановить заряд");
            ui->Discharge->setEnabled(false);
            ui->cycling->setEnabled(false);
        }else if(!(ui->Charge->isChecked()) && (dates[1]).toInt() <= 0 && (dates[14] == "0")){
            ui->Charge->setText("Заряд");
            ui->Discharge->setEnabled(true);
            ui->cycling->setEnabled(true);
        }*/

        if((dates[12]=="1")&&(dates[13]=="0")){
            ui->Status->setStyleSheet("QLabel {background-color : #00E676;}");
            ui->Status->setText("Идет заряд");
        }else if((dates[12]=="0")&&(dates[13]=="1")){
            ui->Status->setStyleSheet("QLabel {background-color : #FF5722;}");
            ui->Status->setText("Идет разряд");
        }else if ((dates[12]=="0")&&(dates[13]=="0")&&(dates[14]=="0")&&(dates[15]=="0")){
            ui->Status->setStyleSheet("QLabel {background-color : #FF9100; color: #212121}");
            ui->Status->setText("Состояние");
        }

        if((dates[14]=="1")&&(dates[15]=="0")){
            ui->Status->setText("Идет заряд");
            ui->Status->setStyleSheet("QLabel {background-color : #00E676;}");
        }else if((dates[14]=="0")&&(dates[15]=="1")){
            ui->Status->setText("Идет разряд");
            ui->Status->setStyleSheet("QLabel {background-color : #FF5722;}");
        }
    }
   dates.clear();
}


void MainWindow::on_graph_clicked()
{
    QJsonDocument jDoc;
    QJsonObject jOb;
    QVector<double> vol,time,current,temperature;
    QString in;
    QString str = QFileDialog::getOpenFileName(0, "Open Dialog", QApplication::applicationDirPath()+"/log/", "*.json");
    if(!str.isNull()){
        QFile file(str);
        if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
            while(!file.atEnd()){
                in = file.readLine();
                jDoc = QJsonDocument::fromJson(in.toUtf8());
                jOb = jDoc.object();
                vol.append(jOb.value(QString("Voltage")).toString().toDouble());
                time.append(jOb.value(QString("Time")).toDouble());
                current.append(jOb.value(QString("Current")).toString().toDouble());
                temperature.append(jOb.value(QString("Temperature")).toString().toDouble());
            }
        }
        file.close();
    }else{return;}

    QRect wRect(QPoint(150,150), QSize(1000,750));
    widget = new QWidget();
    widget->setGeometry(wRect);

    QPushButton *pButton = new QPushButton(widget);
    connect(pButton, SIGNAL(clicked()), this, SLOT(print()));
    QPushButton *sButton = new QPushButton(widget);
    connect(sButton, SIGNAL(clicked()), this, SLOT(snapShot()));
    QFont font = widget->font();
    font.setPointSize(12);
    pButton->setGeometry(10,700,100,50);
    pButton->setText("Печать");
    pButton->setFont(font);
    sButton->setGeometry(110,700,100,50);
    sButton->setText("Сохранить");
    sButton->setFont(font);

    customPlot = new QCustomPlot(widget);
    customPlot->resize(1000,350);
    customPlot->move(0,0);

    customPlot->addGraph(customPlot->xAxis, customPlot->yAxis);
    customPlot->graph(0)->setPen((QColor(Qt::red)));
    customPlot->graph(0)->setData(time,vol);
    customPlot->graph(0)->setAntialiasedFill(false);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->setInteraction(QCP::iRangeDrag, true);
    customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    customPlot->axisRect()->setupFullAxesBox();

    customPlot->xAxis->setLabel("Время");
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss.zzz");
    customPlot->xAxis->setRange(minElem(time),maxElem(time));

    customPlot->yAxis->setAutoTickStep(true);
    customPlot->yAxis->setAutoTicks(true);
    customPlot->yAxis->setLabel("Напряжение");
    customPlot->yAxis->setRange(minElem(vol),maxElem(vol)+200);

    customPlot->rescaleAxes();
    customPlot->replot();


    customPlot2 = new QCustomPlot(widget);
    customPlot2->resize(1000,350);
    customPlot2->move(0,350);
    customPlot2->addGraph(customPlot2->xAxis, customPlot2->yAxis);

    customPlot2->graph(0)->setPen((QColor(Qt::blue)));
    customPlot2->graph(0)->setData(time,current);
    customPlot2->graph(0)->setAntialiasedFill(false);

    customPlot2->setInteraction(QCP::iRangeZoom,true);
    customPlot2->setInteraction(QCP::iRangeDrag, true);
    customPlot2->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    customPlot2->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    customPlot2->axisRect()->setupFullAxesBox();

    customPlot2->xAxis->setLabel("Время");
    customPlot2->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot2->xAxis->setDateTimeFormat("hh:mm:ss.zzz");
    customPlot2->xAxis->setRange(minElem(time),maxElem(time));

    customPlot2->yAxis->setAutoTickStep(true);
    customPlot2->yAxis->setAutoTicks(true);
    customPlot2->yAxis->setLabel("Ток");
    customPlot2->yAxis->setRange(minElem(current),maxElem(current)+200);

    customPlot2->rescaleAxes();
    customPlot2->replot();
    widget->show();
}

void MainWindow::on_setSerial_clicked()
{
    rest = ui->serial->text();
    QString res = "s"+rest;
    writeData(res.toLatin1());
    QFile fileName(QApplication::applicationDirPath()+"/log/"+ui->serial->text()+".json");
    fileName.open(QIODevice::WriteOnly);
    fileName.close();
    //qDebug()<<res;;
    sn=true;
}

void MainWindow::on_setDate_clicked()
{
    uint16_t year,month,day;
    //uint16_t cDate;
    year = ui->dateEdit->date().year();
    month = ui->dateEdit->date().month();
    day = ui->dateEdit->date().day();
    year = (year-1980)*512;
    month = month*32;
    cDate = year+month+day;
    QString res = "e"+QString::number(cDate);
    writeData(res.toLatin1());
    qDebug()<<res.toLatin1();
    dat = true;
    //connect(this, &MainWindow::hasData,this, &MainWindow::toFile);
}

void MainWindow::intToYear(QString date){
    uint16_t year,month,day,fdat;
    //QDate aDate;
    fdat = date.toInt();
    year = fdat >> 9;
    year = ((year << 9)/512)+1980;
    month = fdat << 7;
    month = month >> 12;
    month =  (month << 5)/32;
    day = fdat << 11;
    day=day >> 11;
    aDate.setDate(year,month,day);
    //qDebug() << year << "/" << month << "/" << day << endl << aDate;
    //ui->dateEdit->setDate(aDate);
    dat = true;
}

/*void MainWindow::createFile(QList<QString> value){
    if(hasFile==true){
        QFile file(QApplication::applicationDirPath()+"/"+value[8]+".json");
        file.open(QIODevice::WriteOnly);
        file.close;
        hasFile==false;
    }
}*/


void MainWindow::toFile(QList<QString> value)
{

    // CSV
    //uint16_t i = 0;
    float tmp = value[2].toFloat();
    int tp = 0;
    if(tmp>0){
        tp = (int)(tmp + 0.5);
    }else{
        tp = (int)(tmp - 0.5);
    }
    //bool state = false;

    if((value[0]!=value[1])&&(value[0]!=value[3])&&(value[0]!=value[4])&&(value[0]!=value[5])&&(value[0]!="65535")&&(value[0]!="0")){
        value[0] = value[0].remove(0,1);
    }
    QFile file2(QApplication::applicationDirPath()+"/log/"+value[8]+".csv");
    if(file2.exists()){
        if(file2.open(QIODevice::Append|QIODevice::Text)){
            QTextStream out(&file2);
            out<<value[0]<<";"<<value[1]<<";"<<tp<<";"<<value[3]<<";"<<value[4]<<";"<<value[5]<<";"<<value[6]<<";"<<value[7]<<";"<<QTime::currentTime().toString()<<"\n";
            file2.close();
        }
    }else{
        if(file2.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream out(&file2);
            out<<"Voltage"<<";"<<"Current"<<";"<<"Temperature"<<";"<<"Capacity"<<";"<<"C1V"<<";"<<"C2V"<<";"<<"C3V"<<";"<<"C4V"<<";"<<"Time"<<"\n";
            out<<value[0]<<";"<<value[1]<<";"<<tp<<";"<<value[3]<<";"<<value[4]<<";"<<value[5]<<";"<<value[6]<<";"<<value[7]<<";"<<QTime::currentTime().toString()<<"\n";
            file2.close();
        }
     }

    QtJson::JsonObject setData;
    QByteArray status; 
    //if((value[0]!=value[1])&&(value[0]!=value[3])&&(value[0]!=value[4])&&(value[0]!=value[5])&&(value[0]!="65535")&&(value[0]!="0")){
    //    setData["Voltage"] = value[0].remove(0,1);
    //}
    setData["Voltage"] = value[0];
    setData["Current"] = value[1];
    setData["Temperature"] = value[2];
    setData["Capacity"] = value[3];
    //setData["Full charge capacity"] = value[11];
    setData["Time"] = QDateTime::currentDateTime().toTime_t();
    setData["RealTime"] = QTime::currentTime().toString(Qt::TextDate);
    //qDebug()<<QTime::currentTime().toString(Qt::TextDate);
    setData["S/N"] = value[8];
    status = QtJson::serialize(setData);
    QFile file(QApplication::applicationDirPath()+"/log/"+value[8]+".json");
    if(file.exists()){
        if(file.open(QIODevice::Append|QIODevice::Text)){
            QTextStream out(&file);
            out<<status<<"\n";
            file.close();
        }
    }else{
        if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream out(&file);
            out<<status<<"\n";
            file.close();
        }
    }
}

double MainWindow::minElem(QVector<double> input)
{
    int m=0;
    int i=1;
    while(i<input.length()){
        if(input[i]<input[m]){
            m=i;
        }
        i+=1;
    }
    return input[m];
}

double MainWindow::maxElem(QVector<double> input)
{
    int m=0;
    int i=1;
    while(i<input.length()){
        if(input[i]>input[m]){
            m=i;
        }
        i+=1;
    }
    return input[m];
}

void MainWindow::Print(QString  str)
{
    qDebug()<<str<<endl;
}

QList<QString> MainWindow::wordToByte(QString dat){
    bool ok;
    QList<QString> mlsb;
    uint16_t iVal = dat.toInt(&ok,10);
    mlsb.append(QString::number(iVal&255)); //младший байт
    mlsb.append(QString::number(iVal>>8)); // старший байт
//    qDebug()<<mlsb;
    return mlsb;
}

QList<QString> MainWindow::decToBin(QList<QString> dat){
    QList<QString> binDat;
    bool ok;
    uint8_t msbBin,lsbBin;
    lsbBin = dat[0].toInt(&ok,10);
    msbBin = dat[1].toInt(&ok,10);
    binDat.append(QString::number(lsbBin,2));
    binDat.append(QString::number(msbBin,2));
//    qDebug()<<binDat;
    return binDat;
}

void MainWindow::status(QString stat){
    if(stat[1]=='0'){
        ui->FA->setStyleSheet("QLabel {background: #00E676}");
    }else{
        ui->FA->setStyleSheet("QLabel {background: #FF5722}");
    }
    if(stat[2]=='0'){
        ui->SS->setStyleSheet("QLabel {background: #00E676}");
    }else{
        ui->SS->setStyleSheet("QLabel {background: #FF5722}");
    }
}

void MainWindow::on_Charge_clicked()
{
   /* qDebug("cpress");
    if(!checked){
        ui->Charge->setText("Заряд");
        ui->Discharge->setEnabled(true);
        ui->cycling->setEnabled(true);
    }else{
        ui->Charge->setText("Остановить заряд");
        ui->Discharge->setEnabled(false);
        ui->cycling->setEnabled(false);;
    }*/
  writeData("c");
}

void MainWindow::on_Discharge_clicked()
{
    qDebug("dpress");
    /*if(!checked){
        ui->Discharge->setText("Разряд");
        ui->Charge->setEnabled(true);
        ui->cycling->setEnabled(true);
    }else{
        ui->Discharge->setText("Остановить разряд");
        ui->Charge->setEnabled(false);
        ui->cycling->setEnabled(false);
    }*/
        writeData("d");
}

void MainWindow::on_cycling_clicked(bool checked)
{
    if(checked){
        //writeData("y");
        ui->cycling->setText("Остановить цикл");
        ui->Charge->setEnabled(false);
        ui->Discharge->setEnabled(false);
    }else{
        //writeData("y");
        ui->cycling->setText("Начать цикл");
        ui->Charge->setEnabled(true);
        ui->Discharge->setEnabled(true);
    }
    writeData("y");
}

void MainWindow::on_seal_clicked()
{
    writeData("l");
}

void MainWindow::on_unseal_clicked()
{
    writeData("u");
}

void MainWindow::on_fullAccess_clicked()
{
    writeData("f");
}

void MainWindow::on_SerialFind_clicked()
{
    serialFind();
}

void MainWindow::on_Capacity_clicked(bool checked)
{
    if(checked){
        //qDebug("checked");
        ui->Capacity->setText("Eмкость (mAh)");
    }else{
        //qDebug("not checeked");
        ui->Capacity->setText("Емкость (%)");
    }

}


void MainWindow::on_Stop_clicked()
{
    if(ui->Charge->isChecked()){
        writeData("c");
    }
    if(ui->Discharge->isChecked()){
        writeData("d");
    }
    if(ui->cycling->isChecked()){
        writeData("y");
    }
    ui->Cur->display(0);
    ui->Vol->display(0);
    ui->Temp->display(0);
    ui->Cap->display(0);
    ui->serial->setText("0");
    ui->dateEdit->setDate((QDate::currentDate()));
    ui->Cell1_V->display(0);
    ui->Cell2_V->display(0);
    ui->Cell3_V->display(0);
    ui->Cell4_V->display(0);
    ui->Status->setStyleSheet("QLabel {background-color : #FF9100; color: #212121}");
    ui->Status->setText("Состояние");
    ui->FA->setStyleSheet("QLabel {background: #FF5722}");
    ui->SS->setStyleSheet("QLabel {background: #FF5722}");
    serial = true;    
}

