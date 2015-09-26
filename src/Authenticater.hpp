
#pragma once

#include "predef.hpp"
#include <ctime>
#include "Authority.hpp"

namespace pecar
{

class Authenticater
{
public:
    enum ResCode {
        CODE_OK,            // 成功
        CODE_EXPIRED,       // 会员过期
        CODE_TRAFFIC_EXHAUST, // 阶段流量耗尽
    };

    template<typename Callback>
    void auth(const std::string& user, const std::string& pass,
        const Callback& callback)
    {
        // send auth request
        Authority *authority = new Authority;
        callback(CODE_OK, authority);
    }
};

}
