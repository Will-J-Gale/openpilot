#include <iostream>

#define BLACK "\e[1;90m";
#define RED "\e[1;91m";
#define GREEN "\e[1;92m";
#define YELLOW "\e[1;93m";
#define BLUE "\e[1;94m";
#define PURPLE "\e[1;95m";
#define CYAN "\e[1;96m";
#define WHITE "\e[1;97m";
#define BLACK_BOLD "\e[1;30m";
#define RED_BOLD "\e[1;31m";
#define GREEN_BOLD "\e[1;32m";
#define YELLOW_BOLD "\e[1;33m";
#define BLUE_BOLD "\e[1;34m";
#define PURPLE_BOLD "\e[1;35m";
#define CYAN_BOLD "\e[1;36m";
#define WHITE_BOLD "\e[1;37m";
#define RESET "\e[0m";

#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) printf(YELLOW fmt RESET, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf(RED fmt RESET, ##__VA_ARGS__)