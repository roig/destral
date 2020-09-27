#ifndef AP_DEBUG_H
#define AP_DEBUG_H
/*
    Astral Pixel log and debug utilities

    --- Usage ---
        In one of your .c/.cpp files define this to create the implementation functions
        #define AP_DEBUG_IMPL
        #include "ap_debug.h"

        if you want to use c++ streams use: 
        #define AP_DEBUG_STREAMS

        

    --- Assert macros ---
        AP_ASSERT(condition)
        AP_ASSERTM(condition, message)

    --- Logging ---

    Initialize:
        ap_dbg_init(NULL) -> Outputs: Only stdout
        ap_dbg_init("AstralPixel") -> Outputs: stdout and file (like this AstralPixel-2020.09.13-20.40.50.txt)
    
    Logging:

        Generic log macro:
        AP_LOG(ap_dbg_Info, "Hello %s", "World");

        Without sstreams macro usage:
        
        AP_TRACE("Hello %s", "World") -> outputs: 09/13/20 21:44:15 [T] app.cpp:59 Hello World
        AP_INFO("Hello %s", "World")
        AP_WARNING("Hello %s", "World")
        AP_FATAL("Hello %s", "World") -> this macro will abort the full program
        
        With sstreams macro usage:
        AP_TRACE("Hello" << "World") -> outputs: 09/13/20 21:44:15 [T] app.cpp:59 Hello World
        AP_INFO("Hello " << "World")
        AP_WARNING("Hello " << "World")
        AP_FATAL("Hello " << "World") -> this macro will abort the full program

    Cleanup: 
        AP_Cleanup() -> closes the file

*/


#include <assert.h>
#include <stdlib.h>

#ifndef AP_ASSERT
#ifdef NDEBUG
#define AP_ASSERT(condition) do {} while(0)
#else
#define AP_ASSERT(condition) assert(condition)
#endif
#endif

#ifndef AP_ASSERTM
#ifdef NDEBUG
#define AP_ASSERTM(condition, message) do {} while(0)
#else
#define AP_ASSERTM(condition, message) assert(((void)message, condition))
#endif
#endif


#ifdef NDEBUG
    #define AP_LOG(loglvl_, format_, ...) do {} while(0)
    #define AP_TRACE(format_ , ...) do {} while(0)
    #define AP_INFO(format_ , ...) do {} while(0)
    #define AP_WARNING(format_ , ...) do {} while(0)
    #define AP_FATAL(format_, ...) do { AP_LOG(AP_DBG_ERROR, format_, ##__VA_ARGS__); abort(); } while(0);
#else
    #ifdef AP_DEBUG_STREAMS
        #include <sstream>
        #define AP_LOG(loglvl_, sstream)  do {  std::stringstream s_s__; s_s__ << sstream; ap_dbg_Log(loglvl_, __FILE__ , __LINE__, "%s" , s_s__.str().c_str()); } while (0)
        #define AP_TRACE(sstream) AP_LOG(AP_DBG_TRACE, sstream)
        #define AP_INFO(sstream) AP_LOG(AP_DBG_INFO, sstream)
        #define AP_WARNING(sstream) AP_LOG(AP_DBG_WARNING, sstream)
        #define AP_FATAL(sstream) do { AP_LOG(AP_DBG_ERROR, sstream); abort(); } while(0);
    #else
        #define AP_LOG(loglvl_, format_ , ...) ap_dbg_log(loglvl_, __FILE__ , __LINE__, format_ , ##__VA_ARGS__ )
        #define AP_TRACE(format_ , ...) AP_LOG(AP_DBG_TRACE, format_ , ##__VA_ARGS__ )
        #define AP_INFO(format_ , ...) AP_LOG(AP_DBG_INFO, format_ , ##__VA_ARGS__ )
        #define AP_WARNING(format_ , ...) AP_LOG(AP_DBG_WARNING, format_ , ##__VA_ARGS__ )
        #define AP_FATAL(format_, ...) do { AP_LOG(AP_DBG_ERROR, format_, ##__VA_ARGS__); abort(); } while(0);
    #endif AP_DEBUG_STREAMS
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum ap_dbg_loglvl {
    AP_DBG_TRACE = 0,
    AP_DBG_INFO,
    AP_DBG_WARNING,
    AP_DBG_ERROR
};

/**
        Initializes the log system:
        If file_id is NULL it only logs in stdout.
        Else logs in stdout and creates a file like this: (supposing file_id = "AstralPixel")
        AstralPixel-2020.09.13-20.40.50.txt
*/
void ap_dbg_init(const char* file_id);

/**
    Logs the message to the stdout and file if file is opened.
    Use the macro AP_LOG to have all those parameters filled for you.
*/
void ap_dbg_log(enum ap_dbg_loglvl lvl, const char* file, int line, const char* fmt, ...);

/**
    Closes the file if necessary does the cleanup of the log system
*/
void ap_dbg_shutdown();






#ifdef __cplusplus
}
#endif
#ifdef AP_DEBUG_IMPL
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


FILE* g_logfile = 0;

void ap_dbg_init(const char* file_log_name) {
    if (file_log_name) {
        // build the date/hour part of the filename
        time_t timer = time(0);
        struct tm tm_info;
        localtime_s(&tm_info, &timer);
        char time_str[128] = { 0 };
        strftime(time_str, sizeof(time_str), "-%Y.%m.%d-%H.%M.%S.txt", &tm_info);
        

        // build the full name
        size_t full_name_bufsize = strlen(time_str) + strlen(file_log_name) + 1;
        char* full_name = (char* )calloc(full_name_bufsize, sizeof(char));
        strcpy_s(full_name, full_name_bufsize, file_log_name);
        strcat_s(full_name, full_name_bufsize, time_str);
        errno_t err = fopen_s(&g_logfile, full_name, "w+");
        if (err) {
            printf("Can't open log file: %s \n  Logging continues without file log", full_name);
            g_logfile = 0;
        }
        free(full_name);
    }
}

void ap_dbg_shutdown() {
    if (g_logfile) {
        fclose(g_logfile);
    }
}

void ap_dbg_log(enum ap_dbg_loglvl lvl, const char* file, int line, const char* fmt, ...) {
    // select level character
    char lvl_char = 'U';
    switch (lvl) {
    case AP_DBG_TRACE: lvl_char = 'T'; break;
    case AP_DBG_INFO: lvl_char = 'I'; break;
    case AP_DBG_WARNING: lvl_char = 'W'; break;
    case AP_DBG_ERROR: lvl_char = 'E'; break;
    default: lvl_char = 'U'; break;
    }

    // print time first
    time_t timer = time(NULL);
    struct tm tm_info;
    localtime_s(&tm_info, &timer);
    char time_str[48];
    strftime(time_str, sizeof(time_str), "%x %X", &tm_info);

    // print file name only (avoid full path)
    const char* file_name = file;
    const char* found = NULL;
    #ifdef _WIN32 
    found = strrchr(file, '\\');
    #else
    found = strrchr(file, '/');
    #endif
    if (found) {
        found++;
        file_name = found;
    }
    

    // print header
    printf("%s [%c] %s:%d ", time_str, lvl_char, file_name, line);
    if (g_logfile) {
        fprintf(g_logfile, "%s [%c] %s:%d  ", time_str, lvl_char, file_name, line);
    } 

    // print message format to stdout and file if needed
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    putchar('\n');
    if (g_logfile) {
        vfprintf(g_logfile, fmt, args);
        fputc('\n', g_logfile);
    }
    va_end(args);
}


#endif //AP_DEBUG_IMPL
#endif // AP_DEBUG_H




