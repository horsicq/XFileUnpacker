#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

// Application metadata
#define APP_NAME "XFileUnpacker"
#define APP_VERSION "0.1.0"
#define APP_ORGANIZATION "XFileUnpacker"
#define APP_DESCRIPTION "File unpacking utility"

namespace Global {
    constexpr const char* VERSION = APP_VERSION;
    constexpr const char* NAME = APP_NAME;
    constexpr const char* DESCRIPTION = APP_DESCRIPTION;
}

#endif // GLOBAL_H
