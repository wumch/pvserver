
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "Config.hpp"
#include "Channel.hpp"

namespace asio = boost::asio;

namespace pecar
{

class Bus:
    public boost::noncopyable
{
private:
    const Config* const config;
    asio::io_service ioService;
    tcp::acceptor acceptor;

public:
    Bus():
        config(Config::instance()),
        ioService(config->ioServiceNum),
        acceptor(ioService, tcp::endpoint(config->host, config->port))
    {}

    int run()
    {
        startAccept();
        return runForEver();
    }

private:
    void startAccept()
    {
        boost::shared_ptr<Channel> channel(new Channel(ioService));
        acceptor.async_accept(channel->downstream(),
            boost::bind(&Bus::handleAccept, this, asio::placeholders::error, channel));
    }

    void handleAccept(const boost::system::error_code& err, boost::shared_ptr<Channel>& channel)
    {
        CS_SAY("recevied");
        if (CS_BLIKELY(!err))
        {
            channel->start();
        }
        startAccept();
    }

    int runForEver()
    {
        boost::system::error_code err;
        std::size_t rounds = ioService.run(err);
        if (err)
        {
            return err.value();
        }
        else if (rounds == (std::numeric_limits<std::size_t>::max)())
        {
            return runForEver();
        }
        return 0;
    }
};

}
