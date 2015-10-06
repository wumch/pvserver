
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>

namespace pecar
{

class Portal;

class Config
{
    friend class Portal;
private:
    static Config _instance;

    Config():
        desc("allowed config options")
    {}

    void load(boost::filesystem::path file);

    static Config* mutableInstance()
    {
        return &_instance;
    }

    void initDesc();

public:
    static const Config* instance()
    {
        return &_instance;
    }

    void init(int argc, char* argv[]);

public:
    std::string programName;
    std::size_t workerCount;
    std::size_t ioThreads;
    boost::filesystem::path pidFile;
    std::size_t stackSize;
    bool memlock;

    bool reuseAddress;
    std::size_t maxConnections;
    std::size_t backlog;
    bool dsTcpNodelay;

    std::size_t ioServiceNum;
    boost::asio::ip::address host;
    uint16_t port;

    std::string interfaceNameTpl;

    uint8_t passwordLen;

    typedef struct timeval Timeval;
    Timeval dsRecvTimeout, dsSendTimeout,
        usRecvTimeout, usSendTimeout;

    int uwPendingInterval, dwPendingInterval;

    std::size_t initBufferSize;
    std::size_t userPassTotalLen;

    bool dsLinger, usLinger;
    int dsLingerTimeout, usLingerTimeout;

    bool multiThreads, multiIoThreads;
    mutable boost::mutex workMutex, ioMutex;


    boost::program_options::variables_map options;
    boost::program_options::options_description desc;

private:    // handy utility
    Timeval toTimeval(std::time_t val) const
    {
        Timeval tval = {val, 0};
        return tval;
    }

};

}
