#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QAbstractSocket>
#include <QSocketNotifier>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(int socketFd, QObject *parent = 0);

signals:
    void dataReceived(QByteArray data);
    void disconnected();

public slots:
    void sendData(QByteArray data);


private slots:
    void onReadyRead();

private:
    QAbstractSocket m_socket;
    QSocketNotifier m_notifier;
};

#endif // CLIENTHANDLER_H
