

#include "Aside.hpp"
#include <boost/lexical_cast.hpp>

namespace pecar
{

Aside Aside::_instance;

Authenticater::ResCode Aside::auth(const std::string& user, const std::string& pass, Authority& authority) const
{
//    auther.auth(user, pass);
    static int ifturn = 0;
    authority.ifname = "tun" + boost::lexical_cast<std::string>(ifturn);
    ++ifturn;
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
