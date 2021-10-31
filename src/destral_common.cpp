#include <destral/destral_common.h>
#include <fstream>
#include <chrono>


namespace ds::log {

    static std::ofstream g_logfile;

    void msg(level log_level, const char* file, int line, const std::string_view& msg) {

        // get a precise timestamp as a string
        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);
        const auto now_milis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        struct tm tm_t;  localtime_s(&tm_t, &in_time_t);



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


        // select level character
        const char* lvl_char = "[U]";
        switch (log_level) {
        case level::DS_LOG_TRACE: { lvl_char = "[T]";  } break;
        case level::DS_LOG_INFO: { lvl_char = "[I]"; } break;
        case level::DS_LOG_WARNING: { lvl_char = "[W]"; }break;
        case level::DS_LOG_ERROR: { lvl_char = "[E]";  }break;
        default:
            break;
        }
        
        
        // build message log
        char time_str[256];
        std::strftime(time_str, 256, "%Y-%m-%d %H:%M:%S", &tm_t);
        std::string full_msg = std::format("{}.{:0>3} {} {}:{} {}\n", time_str, now_milis.count(), lvl_char, file_name, line, msg);
        std::printf("%s", full_msg.c_str());
        std::fflush(stdout);  // We can setvbuf(stdout, NULL, _IONBF, 0);  to disable buffering entirely
        if (g_logfile) {
            g_logfile << full_msg;
        }
        

    }

    void init(const char* log_file) {
        if (!log_file) {
            return;
        }

        if (!g_logfile.is_open()) {
            time_t timer = time(NULL);
            struct tm tm_info;
            localtime_s(&tm_info, &timer);

            std::ostringstream oss;
            oss << std::put_time(&tm_info, "%d-%m-%Y %H-%M-%S");

            std::string filename = log_file;
            filename += "-";
            filename += oss.str();
            filename += ".txt";
            g_logfile.open(filename, std::ios_base::trunc | std::ios_base::out | std::ios_base::in);
            if (!g_logfile.is_open()) {
                std::printf("%s",std::format("Can't open log file {} for writting!\n Logging continues without writing to log file!", filename).c_str());
            }
        }
    }

    void shutdown() {
        if (g_logfile.is_open()) {
            g_logfile << "File logging Terminated";
            g_logfile.close();
        }
    }
}