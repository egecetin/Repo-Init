#pragma once

#include <array>
#include <atomic>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

/// Global variable to check if the servers are running
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::vector<std::pair<std::string, std::shared_ptr<std::atomic_flag>>> vCheckFlag;

/**
 * Converts errno to a readable string
 * @param[in] errVal errno value
 * @return std::string Error message
 */
inline std::string getErrnoString(int errVal)
{
    std::array<char, BUFSIZ> buffer{};
    return strerror_r(errVal, buffer.data(), BUFSIZ);
}
