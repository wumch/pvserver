
#pragma once

#include "predef.hpp"
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>
#include "Authority.hpp"
#include "Channel.hpp"

namespace pecar
{

class Outlet
{
public:
    typedef boost::ptr_vector<Channel> ChannelList;

    Authority* authority;
    ChannelList channels;

    explicit Outlet(Authority* _authority):
        authority(_authority)
    {}

    Outlet(const Outlet& other):
        authority(other.authority)
    {
//        channels = o.channels;
    }
};

typedef boost::unordered_map<std::string, Outlet> UserOutletMap;

}
