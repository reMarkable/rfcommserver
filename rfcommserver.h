#ifndef RFCOMMSERVER_H
#define RFCOMMSERVER_H

#include <QThread>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <QTcpSocket>



class RFCommServer : public QThread
{
    Q_OBJECT
public:
    explicit RFCommServer(QObject *parent = 0);

protected:
    void run() override;

signals:
    void socketReady(int fd);
    void tcpReceived(QByteArray data);

public slots:
private slots:
    void connectTcp();
    void onSocketReady(int fd);
    void onRfcReceived(QByteArray data);
    void onTcpReceived();

private:
    QTcpSocket m_tcpSocket;
};

#endif // RFCOMMSERVER_H
