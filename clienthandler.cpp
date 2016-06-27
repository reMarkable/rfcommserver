#include "clienthandler.h"

#include <unistd.h>


ClientHandler::ClientHandler(int socketFd, QObject *parent) : QObject(parent),
    m_socket(QAbstractSocket::UnknownSocketType, parent),
    m_notifier(socketFd, QSocketNotifier::Read)
{

    m_socket.setSocketDescriptor(socketFd);
    connect(&m_notifier, &QSocketNotifier::activated, this, &ClientHandler::onReadyRead);
    qDebug() << "New client handler, socket state:" << m_socket.state();
    onReadyRead();
}

void ClientHandler::sendData(QByteArray data)
{
    qDebug() << "SENDING RFC";
    m_socket.write(data);
}

void ClientHandler::onReadyRead()
{
    emit dataReceived(m_socket.readAll());
}
