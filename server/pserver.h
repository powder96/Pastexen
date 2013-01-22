#pragma once

#include <QObject>
#include <QHash>
#include <QTcpServer>
#include <QAtomicInt>
#include "psetting.h"

class pThreadPool;

class pServer : public QObject
{
    Q_OBJECT

public:
    static const int LIMIT = 50*1024*1024;

    explicit pServer(QObject *parent = 0);
    
signals:
    
public slots:
    void onConnection();
private:
    void timerEvent(QTimerEvent *);

private:
    QTcpServer  _server;
    QHash<QHostAddress, QAtomicInt> _limits;
    size_t _timeLeft;
};
