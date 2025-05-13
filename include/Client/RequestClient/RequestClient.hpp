#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

class RequestClient {
public:
  RequestClient();

  int setSocket(std::string const &);
  int getrcvFd() const;

private:
  int _sockFd;
  int _rcvFd;

  static constexpr int _timeout = 500; // 500ms timeout
};

#endif
