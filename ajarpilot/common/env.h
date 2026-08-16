#include <cstdlib>

static inline const char* get_env(const char* key, const char* default_value = nullptr)
{
    const char* value = std::getenv(key);

    if(value == nullptr and default_value != nullptr)
    {
        return default_value;
    }

    return value;
}

static inline bool env_exits(const char* key)
{
    return std::getenv(key) != nullptr;
}