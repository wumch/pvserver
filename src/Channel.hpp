
#pragma once

#include "predef.hpp"
#include <string>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/posix/stream_descriptor_service.hpp>
#include <boost/static_assert.hpp>
#include "Config.hpp"
#include "Crypto.hpp"
#include "Buffer.hpp"
#include "Authority.hpp"


namespace asio = boost::asio;
using asio::ip::tcp;

namespace pecar
{

class Channel:
    public boost::noncopyable,
    public boost::enable_shared_from_this<Channel>
{
private:
    enum { invalid_fd = -1 };
    enum {
        ip_pack_len_end = 4,
        ip_pack_min_len = 20,
        ip_pack_max_len = 65535
    };
    BOOST_STATIC_ASSERT(ip_pack_min_len >= 4);

    const Config* const config;
    asio::io_service& ioService;
    int ifd;        // interface file descriptor. (works as upstream).
    tcp::socket ds;
    asio::posix::stream_descriptor us;

    Buffer dr, dw, ur, uw;

    Crypto crypto;
    Authority authority;

    int dwPending, uwPending;
    asio::deadline_timer dwTimer, uwTimer;

public:
    explicit Channel(asio::io_service& _ioService):
        config(Config::instance()),
        ioService(_ioService),
        ifd(invalid_fd),
        ds(ioService),
        us(ioService),
        dr(config->initBufferSize),
        dwPending(0), uwPending(0),
        dwTimer(ioService, boost::posix_time::milliseconds(config->dwPendingInterval)),
        uwTimer(ioService, boost::posix_time::milliseconds(config->uwPendingInterval))
    {}

    void start()
    {
        handshake();
    }

    tcp::socket& downstream()
    {
        return ds;
    }

    ~Channel()
    {
        shutdown();
    }

private:
    int getIf(const std::string& name) const;

    void shutdown();

    void handshake();

    void handleUserPassLen(const boost::system::error_code& err, int bytesRead);

    void handleUserPass(const boost::system::error_code& err,
        int bytesRead, uint8_t userLen, uint8_t passLen);

    void handleAuthResSent(const boost::system::error_code& err, int bytesRead);

    void prepareBuffers();

    bool prepareInterface();

    void handleFatalError(const boost::system::error_code& err, int bytesWritten);

    void handleDsRead(const boost::system::error_code& err, int bytesRead, int bytesRemain);
    void handleDsRead(int bytesRead, int bytesRemain);

    void handleDsWritten(const boost::system::error_code& err, int bytesWritten);

    void handleUsRead(const boost::system::error_code& err, int bytesRead);
    void handleUsRead(int bytesRead);

    void handleUsWritten(const boost::system::error_code& err, int bytesWritten);

    int usWritePack(const char* begin, int bytesRemain);

    void continueReadDs(const char* offset, int bytesRemain);

    uint16_t readNetUint16(const char* data) const;
};

}
