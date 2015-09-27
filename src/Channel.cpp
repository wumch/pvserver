
#include "Channel.hpp"
extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
}
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <crypto++/aes.h>
#include "Aside.hpp"

#define __PECAR_KICK_IF(cond) if (CS_UNLIKELY(cond)) { CS_SAY("[" << (uint64_t)this << "] will return"); shutdown(); return; }

#define __PECAR_KICK_IF_ERR(err) if (CS_UNLIKELY(err)) { CS_SAY("[" << (uint64_t)this << "]: [" << CS_OC_RED(err.message()) << "]"); shutdown(); return; }

#define __PECAR_FALSE_IF(err) if (CS_UNLIKELY(err)) { CS_SAY("[" << (uint64_t)this << "] will return false."); shutdown(); return false; }

#define __PECAR_BUFFER(buf)     asio::buffer(buf.data, buf.capacity)

namespace pecar
{

int Channel::getIf(const std::string& name) const
{
    CS_SAY("will open tun device");
    int interface = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
    CS_DUMP(interface);

    if (interface >= 0)
    {
        ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = IFF_TUN;
        std::memcpy(ifr.ifr_name, name.data(), std::min(name.size(), sizeof(ifr.ifr_name)));
        CS_DUMP(std::string(name.data(), 0, std::min(name.size(), sizeof(ifr.ifr_name))));

        if (ioctl(interface, TUNSETIFF, &ifr)) {
            CS_DIE("Cannot get TUN interface");
        }
    }
    CS_SAY("done");

    return interface;
}

void Channel::handshake()
{
    asio::async_read(ds, __PECAR_BUFFER(dr),
        asio::transfer_exactly((CryptoPP::AES::DEFAULT_KEYLENGTH + CryptoPP::AES::BLOCKSIZE) * 2 + 2),
        boost::bind(&Channel::handleUserPassLen, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Channel::handleUserPassLen(const boost::system::error_code& err, int bytesRead)
{
    CS_DUMP(bytesRead);
    __PECAR_KICK_IF_ERR(err);

    const char* ptr = dr.data;
    crypto.setEncKeyWithIv(ptr, CryptoPP::AES::DEFAULT_KEYLENGTH,
        ptr + CryptoPP::AES::DEFAULT_KEYLENGTH, CryptoPP::AES::BLOCKSIZE);
    ptr += CryptoPP::AES::DEFAULT_KEYLENGTH + CryptoPP::AES::BLOCKSIZE;
    crypto.setDecKeyWithIv(ptr, CryptoPP::AES::DEFAULT_KEYLENGTH,
        ptr + CryptoPP::AES::DEFAULT_KEYLENGTH, CryptoPP::AES::BLOCKSIZE);
    uint8_t userPassLen[2];
    crypto.decrypt(ptr + CryptoPP::AES::DEFAULT_KEYLENGTH + CryptoPP::AES::BLOCKSIZE, 2, userPassLen);

    const int totalLen = userPassLen[0] + userPassLen[1];
    if (CS_BLIKELY(0 < userPassLen[0] && 0 < userPassLen[1] && totalLen <= config->userPassTotalLen))
    {
        asio::async_read(ds, __PECAR_BUFFER(dr), asio::transfer_exactly(totalLen),
            boost::bind(&Channel::handleUserPass, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred, userPassLen[0], userPassLen[1]));
    }
}

void Channel::handleUserPass(const boost::system::error_code& err, int bytesRead,
    uint8_t userLen, uint8_t passLen)
{
    __PECAR_KICK_IF_ERR(err);

    uint8_t res = Aside::instance()->auth(std::string(dr.data, userLen), std::string(dr.data + userLen, passLen), authority);
    if (res == Authenticater::CODE_OK)
    {
        CS_SAY("authed");
        prepareBuffers();
        crypto.encrypt(&res, sizeof(res), dw.data);
        asio::async_write(ds, __PECAR_BUFFER(dw), asio::transfer_exactly(sizeof(res)),
            boost::bind(&Channel::handleAuthResSent, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    else
    {
        CS_SAY("auth denied");
        crypto.encrypt(&res, sizeof(res), dr.data);     // use dr last time.
        asio::async_write(ds, __PECAR_BUFFER(dr), asio::transfer_exactly(sizeof(res)),
            boost::bind(&Channel::handleFatalError, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
}

void Channel::handleAuthResSent(const boost::system::error_code& err, int bytesWritten)
{
    CS_DUMP(bytesWritten);
    __PECAR_KICK_IF_ERR(err);

    if (prepareInterface())
    {
        CS_SAY("prepared");
        continueRead();
    }
}

void Channel::handleDsRead(const boost::system::error_code& err, int bytesRead)
{
    CS_DUMP(bytesRead);
    __PECAR_KICK_IF_ERR(err);

    CS_DUMP(int(dr.data[0]));
    CS_DUMP(int(dr.data[1]));
    CS_DUMP(int(dr.data[2]));
    CS_DUMP(int(dr.data[3]));
    if (dr.data[0] != 0)
    {
        crypto.decrypt(dr.data, bytesRead, uw.data);
        CS_DUMP(int(uw.data[0]));
        CS_DUMP(int(uw.data[1]));
        CS_DUMP(int(uw.data[2]));
        CS_DUMP(int(uw.data[3]));
        asio::async_write(us, __PECAR_BUFFER(uw), asio::transfer_exactly(bytesRead),
            boost::bind(&Channel::handleUsWritten, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    continueRead();
}

void Channel::handleUsRead(const boost::system::error_code& err, int bytesRead)
{
    CS_DUMP(bytesRead);
    __PECAR_KICK_IF_ERR(err);

    crypto.encrypt(ur.data, bytesRead, dw.data);
    asio::async_write(ds, __PECAR_BUFFER(dw), asio::transfer_exactly(bytesRead),
        boost::bind(&Channel::handleDsWritten, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
    continueRead();
}

void Channel::handleDsWritten(const boost::system::error_code& err, int bytesWritten)
{
    CS_DUMP(bytesWritten);
    __PECAR_KICK_IF_ERR(err);

    continueRead();
}

void Channel::handleUsWritten(const boost::system::error_code& err, int bytesWritten)
{
    CS_DUMP(bytesWritten);
    __PECAR_KICK_IF_ERR(err);

    continueRead();
    CS_SAY("waiting for upstream readable");
}

void Channel::continueRead()
{
    ds.async_read_some(__PECAR_BUFFER(dr), boost::bind(&Channel::handleDsRead, shared_from_this(),
        asio::placeholders::error, asio::placeholders::bytes_transferred));
    us.async_read_some(__PECAR_BUFFER(ur), boost::bind(&Channel::handleUsRead, shared_from_this(),
        asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Channel::prepareBuffers()
{
    dr.setCapacity(authority.drBufSize);
    dw.setCapacity(authority.dwBufSize);
    ur.setCapacity(authority.urBufSize);
    uw.setCapacity(authority.uwBufSize);
}

bool Channel::prepareInterface()
{
    CS_SAY("will get interface");
    ifd = getIf(authority.ifname);
    CS_DUMP(ifd);
    if (!(ifd < 0))
    {
        us.assign(ifd);
        return true;
    }
    return false;
}

void Channel::handleFatalError(const boost::system::error_code& err, int bytesWritten)
{
    CS_SAY("fatal error, shutdown");
    shutdown();
}

void Channel::shutdown()
{
    CS_SAY("shutdown");
    boost::system::error_code err;
    if (us.is_open())
    {
        us.close(err);  // TODO: ensure close(ifd) not called here.
    }
    if (ifd != invalid_fd)
    {
        close(ifd);
        ifd = invalid_fd;
    }
    if (ds.is_open())
    {
        ds.shutdown(tcp::socket::shutdown_both, err);
        ds.close(err);
    }
}

}

#undef __PECAR_KICK_IF
#undef __PECAR_KICK_IF_ERR
#undef __PECAR_FALSE_IF
#undef __PECAR_BUFFER
