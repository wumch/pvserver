
#pragma once

#include "predef.hpp"
#include <boost/noncopyable.hpp>
#include "Outlet.hpp"
#include "Authority.hpp"
#include "Authenticater.hpp"

namespace pecar
{

class Aside
{
private:
    UserOutletMap users;

    Authenticater auther;

    static Aside _instance;

public:
    Aside();

    static Aside* instance()
    {
        return &_instance;
    }

    Authenticater::ResCode auth(const std::string& user,
        const std::string& pass, Authority& authority) const;

private:
    virtual ~Aside();
};

}
