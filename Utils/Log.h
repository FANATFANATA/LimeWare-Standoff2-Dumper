#ifndef LIMEWARE_STANDOFF2_DUMPER_LOG_H
#define LIMEWARE_STANDOFF2_DUMPER_LOG_H


class Log {
public:
    void info(const char *fmt, ...);
    void debug(const char *fmt, ...);
    void error(const char *fmt, ...);
    void warning(const char *fmt, ...);
    void fatal(const char *fmt, ...);
};


#endif