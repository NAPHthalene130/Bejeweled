#pragma once

#include <asio.hpp>
#include <system_error>

namespace boost {
    namespace asio = ::asio;
    namespace system {
        using error_code = std::error_code;
    }
}
