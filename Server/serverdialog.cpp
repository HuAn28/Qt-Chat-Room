#include "serverdialog.h"
#include "ui_serverdialog.h"

ServerDialog::ServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDialog)
{
    ui->setupUi(this);
    // 当有客户端向服务器发送连接请求，发送信号: newConnection
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));  // 不用阻塞等待客户端连接
    // 定时器到时发送信号:timeout
    connect(&timer, SIGNAL(timeout()), SLOT(onTimeout()));
}

ServerDialog::~ServerDialog()
{
    delete ui;
}

// 创建服务器按钮对应的槽函数
void ServerDialog::on_createButton_clicked()
{
    // 获取服务器端口
    port = ui->portEdit->text().toShort();
    // 设置服务器IP和端口
    if(tcpServer.listen(QHostAddress::Any, port) == true) {
        qDebug() << "创建服务器成功！";
        // 禁用"创建服务器按钮"和"端口输入"
        ui->createButton->setEnabled(false);
        ui->portEdit->setEnabled(false);
        // 开启定时器
        timer.start(3000);
    }
    else {
        qDebug() << "创建服务器失败！";
    }
}

// 相应客户端连接请求的槽函数
void ServerDialog::onNewConnection()
{
    // 获取和客户端通信的套接字
    QTcpSocket* tcpClient = tcpServer.nextPendingConnection();
    // 保存套接字到容器
    tcpClientList.append(tcpClient);
    // 当客户端向服务器发送消息时，通信套接字发送信号:readyRead
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

// 接收客户端消息的槽函数
void ServerDialog::onReadyRead()
{
    // 遍历容器哪个客户端给服务器发送了信息
    for(int i = 0; i < tcpClientList.size(); i++) {
        // bytesAvilable: 获取当前套接字等待读取消息字节数
        // 返回0表示没有消息
        // 返回>0，说明当前套接子有消息到来
        if(tcpClientList.at(i)->bytesAvailable()) {
            // 读取消息并保存
            QByteArray buf = tcpClientList.at(i)->readAll();
            // 显示聊天消息
            ui->listWidget->addItem(buf);
            ui->listWidget->scrollToBottom();
            // 转发消息给所有在线客户端
            sendMessage(buf);
        }
    }
}

// 转发聊天消息给其它客户端
void ServerDialog::sendMessage(const QByteArray& buf)
{
    for(int i = 0; i < tcpClientList.size(); i++) {
        tcpClientList.at(i)->write(buf);
    }
}

// 定时器到时后将执行的槽函数
void ServerDialog::onTimeout(void) {
    // 遍历检查容器中保存的客户端通信套接字是否已经断开链接，如果是则删除
    for(int i = 0; i < tcpClientList.size(); i++) {
        if(tcpClientList.at(i)->state() == QAbstractSocket::UnconnectedState) {
            tcpClientList.removeAt(i);
            i--;
        }
    }
}
