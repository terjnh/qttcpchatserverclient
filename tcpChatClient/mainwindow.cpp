#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect signals and slots
    connect(&m_socket, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);

    // QAbstractSocket is an abstract class, and is an inherited part from QSocket
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &MainWindow::readyRead);

    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->btnSend->setEnabled(false);

    m_model.setStringList(m_list);
    ui->listView->setModel(&m_model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// User clicks 'connect' button
void MainWindow::on_btnConnect_clicked()
{
    if(m_socket.isOpen()) m_socket.close();

    m_name = QInputDialog::getText(this, "Name", "What is your name?",
                                   QLineEdit::EchoMode::Normal, m_name);

    bool ok;
    quint16 port = static_cast<quint16>(ui->txtPort->text().toInt(&ok));
    if(!ok)
    {
        QMessageBox::critical(this, "Error", "Please enter a valid port number!");
        return;
    }

    // Valid port number... continue...
    m_socket.connectToHost(ui->txtServer->text(), port);
    ui->btnConnect->setEnabled(false);
    ui->btnDisconnect->setEnabled(false);
    ui->btnSend->setEnabled(false);

    if(!m_socket.waitForConnected(3000))
    {
        on_btnDisconnect_clicked();
        QMessageBox::critical(this, "Error", "Could not connect to server!");
        return;
    }
}

void MainWindow::on_btnDisconnect_clicked()
{
    m_socket.close();
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->btnSend->setEnabled(false);
}

// Can be called when we are connected, use this to Send Something
void MainWindow::on_btnSend_clicked()
{
    if(!m_socket.isOpen()) return;

    // Send information to the server, who will in turn send that info to all the clients connected
    QByteArray data;
    data.append(m_name);
    data.append(QString(" "));
    data.append(ui->txtMessage->text());  // append the text of whatever the user just typed in

    m_socket.write(data);       // write that information out to the socket
    ui->txtMessage->setText(QString());   // Empty QString

}

// When we are actually connected
void MainWindow::connected()
{
    ui->btnConnect->setEnabled(false);
    ui->btnDisconnect->setEnabled(true);
    ui->btnSend->setEnabled(true);
}

void MainWindow::disconnected()
{
    ui->btnConnect->setEnabled(true);
    ui->btnDisconnect->setEnabled(false);
    ui->btnSend->setEnabled(false);
}

// When we have data in from the socket. Server sends some data, client receives the data
void MainWindow::readyRead()
{
    QByteArray data = m_socket.readAll();
    QString message(data);
    m_list.append(message);
    // Update our model
    m_model.setStringList(m_list);
    ui->listView->scrollToBottom();

}

// Something bad happened
void MainWindow::error(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    QMessageBox::critical(this, "Error", m_socket.errorString());
}
