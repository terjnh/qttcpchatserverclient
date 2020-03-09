#include "server.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{

}

// Number of connected clients
int Server::count()
{
    return m_list.count();
}

void Server::close()
{
    // Close all connected sockets
    foreach(QTcpSocket *socket, m_list)
    {
        socket->close();    // Production-level app: should check if there is a pointer to each list item
    }
    qDeleteAll(m_list);     // Delete all pointers in m_list
    m_list.clear();

    emit changed();

    QTcpServer::close();    // Close down base class : QTcpServer
}

void Server::disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());  // Get the sender of that signal
    if(!socket) return;     // If not a TCP socket, get out

    m_list.removeAll(socket);       // use removeAll instead of removeOne, in case of duplicates

    // Disconnect the signal-slot
    disconnect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);
    disconnect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
    socket->deleteLater();  // Let Qt do the memory management, and have it delete the socket (later) when it's not longer used
    // alternatively, we can just 'delete socket'

    emit changed();
}

// Used when the individual socket has data
void Server::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if(!socket) return;

    QByteArray data = socket->readAll();

    // As this is a chat server, when you send something, we want everybody to get a copy of it...
    foreach(QTcpSocket *socket, m_list)
    {
        socket->write(data);        // This may cause issues in production level apps. Might need more checks before writing
    }
}

void Server::incomingConnection(qintptr handle)
{
    QTcpSocket *socket = new QTcpSocket();
    //qintptr handle: getting that from the underlying OS
    //   Basically our network card lets the OS know smth's happening, OS, lets us know smth's happening, then it gives us a number
    //   we take that 'socket' and set it to that number (handle) as shown below.
    socket->setSocketDescriptor(handle);

    if(!socket->waitForConnected(3000))
    {
        delete socket;
        return;
    }

    m_list.append(socket);  //append to our list of connected sockets
    connect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);

    emit changed();
    socket->write(m_message.toLatin1());
}


// Getter & Setter [m_message]
QString Server::message() const
{
    return m_message;
}

void Server::setMessage(const QString &message)
{
    m_message = message;
}
