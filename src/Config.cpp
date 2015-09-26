
#include "Config.hpp"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#define _PECAR_OUT_CONFIG_PROPERTY(property)     << CS_OC_GREEN(#property) << ":\t\t" << CS_OC_RED(property) << std::endl

namespace pecar
{

void Config::init(int argc, char* argv[])
{
    boost::filesystem::path programPath = argv[0];
#if BOOST_VERSION > 104200
    programName = programPath.filename().string();
#else
    programName = programPath.filename();
#endif

    boost::filesystem::path defaultConfig("etc/" + programName + ".conf");
    desc.add_options()
        ("help,h", "show this help and exit.")
        ("config,c", boost::program_options::value<boost::filesystem::path>()->default_value(defaultConfig),
            ("config file, default " + defaultConfig.string() + ".").c_str())
        ("no-config-file", boost::program_options::bool_switch()->default_value(false),
            "force do not load options from config file, default false.");
    initDesc();

    try
    {
        boost::program_options::command_line_parser parser(argc, argv);
        parser.options(desc).allow_unregistered().style(boost::program_options::command_line_style::unix_style);
        boost::program_options::store(parser.run(), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE(e.what() << CS_LINESEP << desc);
    }
    boost::program_options::notify(options);

    if (options.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }
    else
    {
        bool noConfigFile = options["no-config-file"].as<bool>();
        if (!noConfigFile)
        {
            load(options["config"].as<boost::filesystem::path>());
        }
    }
}

void Config::initDesc()
{
    boost::filesystem::path defaultPidFile("/var/run/" + programName + ".pid");

    desc.add_options()
        ("host", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "listen addr, default 127.0.0.1.")

        ("port", boost::program_options::value<uint16_t>()->default_value(1080), "listen port, default 1080.")

        ("reuse-address", boost::program_options::bool_switch()->default_value(true),
            "whether reuse-address on startup or not, default on.")

        ("tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature for downstream socket or not, default on.")

        ("pid-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("pid file, default " + defaultPidFile.string() + ".").c_str())

        ("stack-size", boost::program_options::value<std::size_t>()->default_value(0),
            "stack size limit (KB), 0 means not set, default 0.")

        ("memlock", boost::program_options::bool_switch()->default_value(false),
            "enable memlock or not, default off.")

        ("worker-count", boost::program_options::value<std::size_t>()->default_value(1),
            "num of worker threads, default 1.")

        ("io-threads", boost::program_options::value<std::size_t>()->default_value(1),
            "num of io threads, default 1.")

        ("io-services", boost::program_options::value<std::size_t>()->default_value(1),
            "num of io services, default 1.")

        ("max-connections", boost::program_options::value<std::size_t>()->default_value(100000),
            "max in and out coming connections, default 100000.")

        ("listen-backlog", boost::program_options::value<std::size_t>()->default_value(1024),
            "listen backlog, default 1024.")

        ("downstream-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for receive from downstream (second), 0 means never timeout, default 30.")

        ("downstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to downstream (second), 0 means never timeout, default 30.")

        ("upstream-receive-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for receive from uptream (second), 0 means never timeout, default 30.")

        ("upstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to uptream (second), 0 means never timeout, default 30.")

        ("initial-buffer-size", boost::program_options::value<std::size_t>()->default_value(32),
            "initial buffer size (KB), at least 1, default 1.")

        ("downstream-linger", boost::program_options::bool_switch()->default_value(true),
            "socket linger on/off for downstream, default off.")

        ("downstream-linger-timeout", boost::program_options::value<int>()->default_value(3),
            "socket linger timeout for downstream (second), default 3.")

        ("upstream-linger", boost::program_options::bool_switch()->default_value(false),
            "socket linger on/off for upstream, default off.")

        ("upstream-linger-timeout", boost::program_options::value<int>()->default_value(0),
            "socket linger timeout for upstream (second), default 3.")

        ("username-password-total-max-len", boost::program_options::value<std::size_t>()->default_value(254),
            "max length of username and password, default 254.")
    ;
}

void Config::load(boost::filesystem::path file)
{
    try
    {
        boost::program_options::store(boost::program_options::parse_config_file<char>(file.c_str(), desc), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE("faild on read/parse config-file: " << file << "\n" << e.what());
    }
    boost::program_options::notify(options);

    host = boost::asio::ip::address_v4::from_string(options["host"].as<std::string>());
    port = options["port"].as<uint16_t>();

    reuseAddress = options["reuse-address"].as<bool>();

    pidFile = options["pid-file"].as<boost::filesystem::path>();
    stackSize = options["stack-size"].as<std::size_t>() << 10;
    memlock = options["memlock"].as<bool>();
    workerCount = options["worker-count"].as<std::size_t>();
    ioThreads = options["io-threads"].as<std::size_t>();
    ioServiceNum = options["io-services"].as<std::size_t>();
    maxConnections = options["max-connections"].as<std::size_t>();
    backlog = options["listen-backlog"].as<std::size_t>();

    dsTcpNodelay = options["tcp-nodelay"].as<bool>();

    dsRecvTimeout = toTimeval(options["downstream-receive-timeout"].as<std::time_t>());
    dsSendTimeout = toTimeval(options["downstream-send-timeout"].as<std::time_t>());
    usRecvTimeout = toTimeval(options["upstream-receive-timeout"].as<std::time_t>());
    usSendTimeout = toTimeval(options["upstream-send-timeout"].as<std::time_t>());

    initBufferSize = options["initial-buffer-size"].as<std::size_t>() << 10;

    dsLinger = options["downstream-linger"].as<bool>();
    dsLingerTimeout = options["downstream-linger-timeout"].as<int>();
    usLinger = options["upstream-linger"].as<bool>();
    usLingerTimeout = options["upstream-linger-timeout"].as<int>();

    userPassTotalLen = options["username-password-total-max-len"].as<std::size_t>();

    multiThreads = workerCount > 1;
    multiIoThreads = ioThreads > 1;

    CS_SAY(
        "loaded configs in [" << file.string() << "]:" << std::endl
        _PECAR_OUT_CONFIG_PROPERTY(programName)
        _PECAR_OUT_CONFIG_PROPERTY(host)
        _PECAR_OUT_CONFIG_PROPERTY(port)
        _PECAR_OUT_CONFIG_PROPERTY(workerCount)
        _PECAR_OUT_CONFIG_PROPERTY(ioThreads)
        _PECAR_OUT_CONFIG_PROPERTY(stackSize)
        _PECAR_OUT_CONFIG_PROPERTY(memlock)
        _PECAR_OUT_CONFIG_PROPERTY(reuseAddress)
        _PECAR_OUT_CONFIG_PROPERTY(maxConnections)
        _PECAR_OUT_CONFIG_PROPERTY(backlog)
        _PECAR_OUT_CONFIG_PROPERTY(dsTcpNodelay)
        _PECAR_OUT_CONFIG_PROPERTY(ioServiceNum)
        _PECAR_OUT_CONFIG_PROPERTY(dsRecvTimeout.tv_sec)
        _PECAR_OUT_CONFIG_PROPERTY(dsSendTimeout.tv_sec)
        _PECAR_OUT_CONFIG_PROPERTY(usRecvTimeout.tv_sec)
        _PECAR_OUT_CONFIG_PROPERTY(usSendTimeout.tv_sec)
        _PECAR_OUT_CONFIG_PROPERTY(initBufferSize)
        _PECAR_OUT_CONFIG_PROPERTY(dsLinger)
        _PECAR_OUT_CONFIG_PROPERTY(dsLingerTimeout)
        _PECAR_OUT_CONFIG_PROPERTY(usLinger)
        _PECAR_OUT_CONFIG_PROPERTY(usLingerTimeout)
        _PECAR_OUT_CONFIG_PROPERTY(userPassTotalLen)
        _PECAR_OUT_CONFIG_PROPERTY(multiThreads)
        _PECAR_OUT_CONFIG_PROPERTY(multiIoThreads)
    );
}

Config Config::_instance;

}

#undef _PECAR_OUT_CONFIG_PROPERTY
