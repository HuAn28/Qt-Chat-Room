#include "clientdialog.h"
#include "ui_clientdialog.h"

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
{
    ui->setupUi(this);
    status = false;  // 离线
    ui->sendButton->setEnabled(false);
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError()));
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

// 发送按钮对应的槽函数
void ClientDialog::on_sendButton_clicked()
{
    // 获取用户输入聊天消息
    QString msg = ui->messageEdit->text();
    if(msg == "") {
        return;
    }
    msg = username + ":" + msg;
    // 发送聊天消息
    tcpSocket.write(msg.toUtf8());
    // 清空消息输入框
    ui->messageEdit->clear();
}

// 连接服务器按钮对应的槽函数
void ClientDialog::on_connectButton_clicked()
{
    // 如果是离线状态，则建立和服务器连接
    if(status == false) {
        // 获取服务器 IP
        serverIP.setAddress(ui->serverIpEdit->text());
        // 获取服务器端口
        serverPort = ui->serverPortEdit->text().toShort();
        // 获取聊天室昵称
        username = ui->usernameEdit->text();
        // 向服务器发送连接请求
        // 成功发送信号:connected
        // 失败发送信号:error
        tcpSocket.connectToHost(serverIP, serverPort);
    }
    // 如果是在线状态，则断开和服务器连接
    else {
        // 向服务器发送离开聊天室的提示信息
        QString msg = username + ":离开了聊天室！";
        tcpSocket.write(msg.toUtf8());
        // 关闭和服务器的连接，同时发送信号:disconnected
        tcpSocket.disconnectFromHost();
    }
}

// 和服务器连接成功时执行的槽函数
void ClientDialog::onConnected()
{
    ui->connectButton->setText("离开聊天室");
    status = true;  // 在线
    ui->sendButton->setEnabled(true);      // 恢复按钮状态
    ui->serverIpEdit->setEnabled(false);   // 禁用IP
    ui->serverPortEdit->setEnabled(false); // 禁用port
    ui->usernameEdit->setEnabled(false);   // 禁用昵称

    // 向服务器发送进入聊天室提示消息
    QString msg = username + ":进入聊天室";
    // toUtf8:QString转换QByteArray
    tcpSocket.write(msg.toUtf8());
}

// 和服务器断开连接时执行的槽函数
void ClientDialog::onDisconnected()
{
    ui->connectButton->setText("连接服务器");
    status = false;  // 离线
    ui->sendButton->setEnabled(false);    // 禁用按钮状态
    ui->serverIpEdit->setEnabled(true);   // 恢复IP
    ui->serverPortEdit->setEnabled(true); // 恢复port
    ui->usernameEdit->setEnabled(true);   // 恢复昵称
}

// 接受聊天信息的槽函数
void ClientDialog::onReadyRead()
{
    if(tcpSocket.bytesAvailable()) {
        // 接受消息
        QByteArray buf = tcpSocket.readAll();
        // 显示消息
        ui->listWidget->addItem(buf);
        ui->listWidget->scrollToBottom();
    }
}

// 网络异常执行的槽函数
void ClientDialog::onError()
{
    // errorString():用户获取网络异常的原因
    QMessageBox::critical(this, "ERROR", tcpSocket.errorString());
}







/*
        ui->connectButton->setText("连接服务器");
        status = false;*/

