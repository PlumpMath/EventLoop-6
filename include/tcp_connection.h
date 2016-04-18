#ifndef _TCP_CONNECTION_H
#define _TCP_CONNECTION_H

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include <map>

#include "eventloop.h"
#include "tcp_callbacks.h"
#include "error_code.h"

namespace evt_loop {

struct IPAddress
{
  string ip_;
  uint16_t port_;

  IPAddress(string ip = "", uint16_t port = 0) : ip_(ip), port_(port) {}
  string ToString() const
  {
      char buffer[256] = {0};
      snprintf(buffer, sizeof(buffer), "{ ip: %s, port: %d }", ip_.c_str(), port_);
      return buffer;
  }
};

void SocketAddrToIPAddress(const struct sockaddr_in& sock_addr, IPAddress& ip_addr);

class TcpConnection;

class TcpCreator: public IOEvent
{
  protected:
    TcpCreator() : IOEvent(IOEvent::READ | IOEvent::ERROR) {}
  public:
    virtual void OnConnectionClosed(TcpConnection* conn) = 0;
};

class TcpConnection : public BufferIOEvent
{
  public:
    TcpConnection(int fd, const IPAddress& local_addr, const IPAddress& peer_addr,
            TcpCallbacksPtr tcp_evt_cbs = nullptr, TcpCreator* creator = nullptr);
    ~TcpConnection();

    uint32_t ID() const { return id_; }
    void SetID(uint32_t id) { id_ = id; }

    void Disconnect();

    void SetTcpCallbacks(const TcpCallbacksPtr& tcp_evt_cbs);

    void SetReady(int fd);
    bool IsReady() const;

    const IPAddress& GetLocalAddr() const;
    const IPAddress& GetPeerAddr() const;

  protected:
    void OnReceived(const Message* buffer);
    void OnSent(const Message* buffer);
    void OnClosed();
    void OnError(int errcode, const char* errstr);

  private:
    uint32_t        id_;
    bool            ready_;
    IPAddress       local_addr_;
    IPAddress       peer_addr_;

    TcpCallbacksPtr tcp_evt_cbs_;
    TcpCreator*     creator_;
};

typedef std::shared_ptr<TcpConnection>          TcpConnectionPtr;
typedef std::map<int/*fd*/, TcpConnectionPtr>   FdTcpConnMap;

}  // namespace evt_loop

#endif  // _TCP_CONNECTION_H
