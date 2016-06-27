#include "rfcommserver.h"
#include "clienthandler.h"
#include <errno.h>
#include <unistd.h>
#include <QTimer>

RFCommServer::RFCommServer(QObject *parent) : QThread(parent)
{
    connect(this, &RFCommServer::socketReady, this, &RFCommServer::onSocketReady);
    connect(&m_tcpSocket, &QTcpSocket::readyRead, this, &RFCommServer::onTcpReceived);
    connect(&m_tcpSocket, &QTcpSocket::connected, []() {
        qDebug() << "Connected to localhost";
    });
    connect(&m_tcpSocket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
        [&](QAbstractSocket::SocketError socketError){
        qWarning() << "SOCKET ERROR" << m_tcpSocket.errorString();
    });
    connect(&m_tcpSocket, &QTcpSocket::disconnected, [&]() {
        qWarning() << "SOCKET DISCONNECTED";
        QTimer::singleShot(100, this, SLOT(connectTcp()));
    });
    QTimer::singleShot(100, this, SLOT(connectTcp()));
}

void RFCommServer::run()
{
    struct sockaddr_rc addr;
    struct rfcomm_conninfo conn;
    socklen_t optlen;
    int sk, nsk, opt = 0;
    char ba[18];
    int channel = 10;

    bdaddr_t bdaddr;
    memset(bdaddr.b, 0, sizeof(bdaddr.b));
    int defer_setup = 0;

    /* Create socket */
    sk = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sk < 0) {
        qWarning() << "Can't create socket" << strerror(errno) << errno;
        return;
    }

    /* Bind to local address */
    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    bacpy(&addr.rc_bdaddr, &bdaddr);
    addr.rc_channel = channel;

    if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        qWarning() << "Can't bind socket" << strerror(errno) << errno;
        goto error;
    }

    if (opt && setsockopt(sk, SOL_RFCOMM, RFCOMM_LM, &opt, sizeof(opt)) < 0) {
        qWarning() << "Can't set rfcom link mode" << strerror(errno) << errno;
        ::close(sk);
        return;
    }

    /* Enable deferred setup */
    opt = defer_setup;

    if (opt && setsockopt(sk, SOL_BLUETOOTH, BT_DEFER_SETUP,
                        &opt, sizeof(opt)) < 0) {
        qWarning() << "Can't enable deferred setup" << strerror(errno) << errno;
        ::close(sk);
        return;
    }

    /* Listen for connections */
    if (::listen(sk, 10)) {
        qWarning() << "Can't listen on socket" << strerror(errno) << errno;
        ::close(sk);
        return;
    }

    /* Check for socket address */
    memset(&addr, 0, sizeof(addr));
    optlen = sizeof(addr);

    if (getsockname(sk, (struct sockaddr *) &addr, &optlen) < 0) {
        qWarning() << "Can't get socket name" << strerror(errno) << errno;
        ::close(sk);
        return;
    }

    channel = addr.rc_channel;

    qDebug() << "Waiting for connection on channel" << channel;

    while (!isInterruptionRequested()) {
        memset(&addr, 0, sizeof(addr));
        optlen = sizeof(addr);

        nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
        if (nsk < 0) {
            qWarning() << "Accept failed" << strerror(errno) << errno;
            break;
        }
        qDebug() << "New connection!" << nsk;
        struct rfcomm_conninfo conn;
        socklen_t optlen;
        int priority;

        memset(&conn, 0, sizeof(conn));
        optlen = sizeof(conn);

        if (getsockopt(nsk, SOL_RFCOMM, RFCOMM_CONNINFO, &conn, &optlen) < 0) {
            qWarning() << "Can't get RFCOMM connection info" << strerror(errno) << errno;
            ::close(nsk);
            continue;
        }

        int opt = 0;
        optlen = sizeof(priority);
        if (getsockopt(nsk, SOL_SOCKET, SO_PRIORITY, &opt, &optlen) < 0) {
            qWarning() << "Can't get socket priority" << strerror(errno) << errno;
            ::close(nsk);
            continue;
        }

        char ba[18];
        ba2str(&addr.rc_bdaddr, ba);
        qDebug() << "Connect from %s [handle %d, "
                 << "class 0x%02x%02x%02x, priority %d]" <<
                    ba << conn.hci_handle << conn.dev_class[2] <<
                    conn.dev_class[1] << conn.dev_class[0] << opt;

        /* Enable SO_LINGER */
        struct linger l;
        l.l_onoff = 1;
        l.l_linger = 30; // wait for 30 seconds for all messages to be sent before returning //linger;

        if (setsockopt(nsk, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
            qWarning() << "Can't enable linger" << strerror(errno) << errno;
            ::close(nsk);
            continue;
        }
        emit socketReady(nsk);
    }

error:
    ::close(sk);

}

void RFCommServer::connectTcp()
{
    qDebug() << "Connecting to tcp";
    m_tcpSocket.connectToHost("localhost", 8008);
}

void RFCommServer::onSocketReady(int fd)
{
    ClientHandler *handler = new ClientHandler(fd, this);
    connect(handler, &ClientHandler::dataReceived, this, &RFCommServer::onRfcReceived);
    connect(this, &RFCommServer::tcpReceived, handler, &ClientHandler::sendData);
//    connect(this, SIGNAL(tcpReceived), handler, SLOT(sendData));
}

void RFCommServer::onRfcReceived(QByteArray data)
{
    qDebug() << "RFC RECEVIED, forwarding";
    m_tcpSocket.write(data);
}

void RFCommServer::onTcpReceived()
{
    qDebug() << "TCP RECEVIED, forwarding";
    emit tcpReceived(m_tcpSocket.readAll());
}
