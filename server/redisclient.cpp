#include "redisclient.h"

TRedisClient::TRedisClient() {
}

void TRedisClient::SetTimeout(int timeout) {
    this->Timeout = timeout;
}

int TRedisClient::GetTimeout() {
    return Timeout;
}

bool TRedisClient::Connect(QString host, int port) {
    Socket->connectToHost(host, port);
    if(!Socket->waitForConnected(Timeout)) {
        return false;
    }
    return true;
}

bool TRedisClient::Disconnect() {
    Socket->disconnectFromHost();
    return true;
}

int TRedisClient::GetErrorCode() {
    return Socket->error();
}

QString TRedisClient::GetErrorString() {
    return Socket->errorString();
}

QStringList TRedisClient::Query(QString cmd)
{
    QStringList response;
    if(SendRequest(cmd)) {
        response = ReadResponse();
    }
    return response;
}

bool TRedisClient::SendRequest(QString cmd)
{
    QTextStream stream(Socket);

    QString test=cmd;
    int k;

    QChar c, next;
    QStringList parts;
    QString buffer = "";
    bool open = false;
    for(k = 0; k < test.length(); k++) {
        c = test.at(k);
        if(open) {
            next = k < test.length() - 1 ? test.at(k+1) : ' ';
            if(c == '\\' && next == '"'){
                buffer += '"';
                k++;
            }
            else if(c == '"') {
                open = false;
            }
            else {
                buffer += c;
            }
        } else {
            if(!c.isSpace()) {
                if(c=='"') {
                    open=true;
                } else {
                    buffer+=c;
                }
            } else if(!buffer.isEmpty()){
                parts << buffer;
                buffer = "";
            }
        }
    }

    if(!buffer.isEmpty()) {
        parts << buffer;
    }

    QString bin = "";
    bin.append(QString("*%1\r\n").arg(parts.length()));
    int i;
    for(i = 0; i < parts.length(); i++) {
        bin.append(QString("$%1\r\n").arg(parts.at(i).length()));
        bin.append(QString("%1\r\n").arg(parts.at(i)));
    }

    stream << bin;
    stream.flush();

    if (!Socket->waitForReadyRead(Timeout)) {
        return false;
    }
    return true;
}

QStringList TRedisClient::ReadResponse()
{
    QString reply = "";
    while (Socket->canReadLine()) {
        reply.append(Socket->readLine());
    }

    QChar first=reply.at(0);
    QString type,value;
    QStringList result;
    if (first == '+') {
        type = "string";
        value = reply.mid(1);
        value.chop(2);
        result << type << value;
    } else if(first == '-') {
        type = "error";
        value = reply.mid(1);
        value.chop(2);
        result << type << value;
    } else if(first == ':') {
        type = "integer";
        value = reply.mid(1);
        value.chop(2);
        result << type << value;
    } else if(first == '$') {
        type = "bulk";
        int index = reply.indexOf("\r\n");
        int len = reply.mid(1, index - 1).toInt();
        if(len == -1) {
            value = "NULL";
        } else {
            value = reply.mid(index+2, len);
        }
        result << type << value;
    }else if(first == '*') {
        type = "list";
        int index = reply.indexOf("\r\n");
        int count = reply.mid(1, index - 1).toInt();
        int i;
        int len;
        int pos = index + 2;
        result << type;
        for(i=0; i< count; i++) {
            index = reply.indexOf("\r\n",pos);
            len = reply.mid(pos + 1, index - pos - 1).toInt();
            if(len == -1) {
                result << "NULL";
            } else {
                result << reply.mid(index + 2, len);
            }
            pos = index + 2 + len + 2;
        }
    }
    return result;
}

bool TRedisClient::IsConnected()
{
    return Socket->state() == QTcpSocket::ConnectedState;
}
