#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include "io.h"
#include "text.h"
#include "commands.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QTcpSocket *socket;
    bool appended;
    char *buff;
    fs_t buffSize;
    fs_t buffPos;
    fs_t cmdStart;
    uint32_t command;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void log(QString str);

public slots:
    void connectBtnClicked();
    void toggleRedLEDBtnClicked();
    void toggleGreenLEDBtnClicked();
    void readGreenLEDBtnClicked();
    void dataReceived();
    void socketError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
