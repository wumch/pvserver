
#pragma once

#include "predef.hpp"
#include <unistd.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include "Config.hpp"
#include "Bus.hpp"

using boost::asio::ip::tcp;

namespace pecar
{

class Portal
{
private:
    Bus bus;
    const Config* const config;
    bool pidFileSelfCreated;

public:
    Portal():
        config(Config::instance()), pidFileSelfCreated(false)
    {}

    static void init(int argc, char* argv[])
    {
        Config::mutableInstance()->init(argc, argv);
    }

    int run()
    {
        savePid();
        return bus.run();
    }

    ~Portal()
    {
        clearPid();
    }

private:
    void savePid()
    {
        if (boost::filesystem::exists(config->pidFile))
        {
            if (boost::filesystem::file_size(config->pidFile) > 0)
            {
                CS_DIE("pid-file [" << config->pidFile << "] already exists and is not empty");
            }
        }

        boost::filesystem::path dir(config->pidFile.parent_path());
        if (!boost::filesystem::is_directory(dir))
        {
            try
            {
                boost::filesystem::create_directories(dir);
            }
            catch (const std::exception& e)
            {
                CS_DIE(e.what());
            }
        }

        try
        {
            bool pidFileAlreadyExists = boost::filesystem::exists(config->pidFile);
            std::ofstream of(config->pidFile.c_str());
            if (!of.is_open())
            {
                CS_DIE("failed on opening pid-file [" << config->pidFile << "]");
            }
            of << getpid() << std::endl;
            of.close();
            if (!of.good())
            {
                CS_DIE("failed on writing pid to [" << config->pidFile << "]");
            }
            CS_SAY("pid written to [" << config->pidFile << "]");
            pidFileSelfCreated = pidFileAlreadyExists;
        }
        catch (const std::exception& e)
        {
            CS_DIE(e.what());
        }
    }

    void clearPid()
    {
        if (boost::filesystem::exists(config->pidFile))
        {
            if (pidFileSelfCreated)
            {
                boost::filesystem::remove(config->pidFile);
            }
            else
            {
                boost::system::error_code err;
                boost::filesystem::resize_file(config->pidFile, 0, err);
                if (err)
                {
                    CS_ERR("failed on truncate pid-file: [" << config->pidFile << "], error: " << err.message());
                }
            }
        }
    }
};

}
