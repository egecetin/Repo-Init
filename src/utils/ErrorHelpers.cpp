#include "utils/ErrorHelpers.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::vector<std::pair<std::string, std::shared_ptr<std::atomic_flag>>> vCheckFlag;

char *checkError(int, char *buffer, int) { return buffer; }

char *checkError(char *result, char *, int) { return result; }
