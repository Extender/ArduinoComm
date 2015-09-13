#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buffSize=1024;
    buffPos=0;
    cmdStart=0;
    command=0;
    buff=(char*)malloc(buffSize);
    ui->targetBox->setText(/*IP of Arduino Yun*/ "192.168.188.89");
    log("Application started!");
    connect(ui->connectBtn,SIGNAL(clicked()),this,SLOT(connectBtnClicked()));
    connect(ui->toggleGreenLEDBtn,SIGNAL(clicked()),this,SLOT(toggleGreenLEDBtnClicked()));
    connect(ui->toggleRedLEDBtn,SIGNAL(clicked()),this,SLOT(toggleRedLEDBtnClicked()));
    connect(ui->readGreenLEDBtn,SIGNAL(clicked()),this,SLOT(readGreenLEDBtnClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(QString str)
{
    ui->statusBox->append(QString(appended?"\n":"")+str);
    if(!appended)
        appended=true;
}

void MainWindow::connectBtnClicked()
{
    log("Connecting...");
    socket=new QTcpSocket(this);
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(readyRead()),this,SLOT(dataReceived()));
    socket->connectToHost(ui->targetBox->text(),COMM_PORT);
    socket->waitForConnected();
    log("Connected!");
}

void MainWindow::toggleRedLEDBtnClicked()
{
    fs_t pos=0;
    fs_t bufferSize=1024;
    char *buffer=(char*)malloc(bufferSize);
    io::writeUInt16ToBuffer(buffer,CMD_TOGGLE,pos,bufferSize);
    io::writeUInt32ToBuffer(buffer,/*Sensor ID*/ 3,pos,bufferSize);
    int64_t bytesWritten=socket->write(buffer,pos);
    log(text::concat("Wrote ",text::longToString(bytesWritten)," bytes."));
}

void MainWindow::toggleGreenLEDBtnClicked()
{
    fs_t pos=0;
    fs_t bufferSize=1024;
    char *buffer=(char*)malloc(bufferSize);
    io::writeUInt16ToBuffer(buffer,CMD_TOGGLE,pos,bufferSize);
    io::writeUInt32ToBuffer(buffer,/*Sensor ID*/ 2,pos,bufferSize);
    int64_t bytesWritten=socket->write(buffer,pos);
    log(text::concat("Wrote ",text::longToString(bytesWritten)," bytes."));
}

void MainWindow::readGreenLEDBtnClicked()
{
    fs_t pos=0;
    fs_t bufferSize=1024;
    char *buffer=(char*)malloc(bufferSize);
    io::writeUInt16ToBuffer(buffer,CMD_GET_DIGITAL,pos,bufferSize);
    io::writeUInt32ToBuffer(buffer,/*Sensor ID*/ 2,pos,bufferSize);
    int64_t bytesWritten=socket->write(buffer,pos);
    log(text::concat("Wrote ",text::longToString(bytesWritten)," bytes."));
}

void MainWindow::dataReceived()
{
    int64_t bytes=socket->bytesAvailable();
    log(text::concat("Data received (",text::longToString(bytes)," bytes)!"));
    char *append=(char*)malloc(bytes);
    socket->read(append,bytes);
    io::writeRawDataToBuffer(buff,append,bytes,buffPos,buffSize);
    uint32_t diff;
    while((diff=buffPos-cmdStart)>=2)
    {
        if(command==0)
        {
            command=io::peekUInt16(buff,cmdStart);
            if(command==0)
            {
                cmdStart+=2;
                command=0;
            }
        }
        if(diff>=7)
        {
            if(command==CMD_GET_DIGITAL)
            {
                uint32_t sensor=io::peekUInt32(buff,cmdStart+2);
                uint8_t value=io::peekUInt8(buff,cmdStart+6);
                log(text::concat("Digital value of ",text::unsignedIntToString(sensor),": ",text::unsignedIntToString(value)));
                cmdStart+=7; // Not "diff"! There could be more!
                command=0;
            }
        }
        if(diff>=10)
        {
            if(command==CMD_GET_ANALOG)
            {
                uint32_t sensor=io::peekUInt32(buff,cmdStart+2);
                uint32_t value=io::peekUInt32(buff,cmdStart+6);
                log(text::concat("Analog value of ",text::unsignedIntToString(sensor),": ",text::unsignedIntToString(value)));
                cmdStart+=10; // Not "diff"! There could be more!
                command=0;
            }
        }
    }
}

void MainWindow::socketError(QAbstractSocket::SocketError error)
{
    log(text::concat("Error ",text::toString(error),"."));
}
