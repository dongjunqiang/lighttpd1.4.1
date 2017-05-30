#include "debug.h"

std::string now(const std::string &format) {
    char buf[20];
    time_t ts = std::time(NULL);
    std::strftime(buf, 20, format.c_str(), std::localtime(&ts));
    return buf;
}

/* 供CPP文件使用, 打印调试日志 */
void debug_log(std::string msg) {
    std::ofstream out;
    out.open("/tmp/luna.debug.log", std::ios::out|std::ios::app);
    out << now("%Y-%m-%d %H:%M:%S") << " " << msg << std::endl;;
    out.close();
}

