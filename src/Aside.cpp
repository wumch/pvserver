

#include "Aside.hpp"

namespace pecar
{

Aside Aside::_instance;

Authenticater::ResCode Aside::auth(const std::string& user, const std::string& pass, Authority& authority) const
{
//    auther.auth(user, pass);
    authority.ifname = "tun0";
    authority.drBufSize = 64 << 10;
    authority.dwBufSize = 64 << 10;
    authority.urBufSize = 64 << 10;
    authority.uwBufSize = 64 << 10;
    return Authenticater::CODE_OK;
}

Aside::Aside()
{}

Aside::~Aside()
{}

}
