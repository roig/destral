#include <destral/core/destral_common.h>
//#include <iostream>
//#include <string>
//#include <sstream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/format.h>

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
        fmt::text_style style = fg(fmt::color::white);
        switch (log_level) {
        case level::DS_LOG_TRACE: { lvl_char = "[T]"; style = fg(fmt::color::forest_green); } break;
        case level::DS_LOG_INFO: { lvl_char = "[I]"; } break;
        case level::DS_LOG_WARNING: { lvl_char = "[W]"; style = fg(fmt::color::yellow); }break;
        case level::DS_LOG_ERROR: { lvl_char = "[E]";  style = fg(fmt::color::red); }break;
        default:
            break;
        }
        // build message log
       
        const std::string full_msg = fmt::format(style, "{:%x %X}.{:0>3} {} {}:{} {}\n", tm_t, now_milis.count(), lvl_char, file_name, line, msg);

  /*    std::stringstream full_msg;
        full_msg << std::put_time(&tm_t, "%x %X") << "." << std::setfill('0') << std::setw(3) << now_milis.count()
            << " " << lvl_char << " " << file_name << ":" << line << " " << msg << '\n';*/
        // write the message to log
        //std::cout << full_msg.rdbuf();


        fmt::print(full_msg);
        if (g_logfile) {
            /*full_msg.seekg(0, std::ios::beg);
            g_logfile << full_msg.rdbuf();*/
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
                fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "{} {} {}", "Can't open log file ", filename, "for writting!\n Logging continues without writing to log file!");
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