

#include "Aside.hpp"

namespace pecar
{

Aside Aside::_instance;

Authenticater::ResCode Aside::auth(const std::string& user, const std::string& pass, Authority& authority) const
{
//    auther.auth(user, pass);
    authority.ifname = "tun0";
    authority.drBufSize = 32 << 10;
    authority.dwBufSize = 32 << 10;
    authority.urBufSize = 32 << 10;
    authority.uwBufSize = 32 << 10;
    return Authenticater::CODE_OK;
}

Aside::Aside()
{}

Aside::~Aside()
{}

}
