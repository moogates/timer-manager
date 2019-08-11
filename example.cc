#include "timer_manager.h"

#include <iostream>
// #include <cstdlib>

static auto begin_time = std::chrono::system_clock::now();

std::string FormatUnixTime(time_t rawtime) {
  struct tm * timeinfo;
  char buffer[32];
  timeinfo = localtime (&rawtime);
  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", timeinfo);
  return std::string(buffer);
}

std::string Uptime() {
  auto current_time = std::chrono::system_clock::now();
  int64_t unix_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count();
  int64_t uptime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - begin_time).count();
  int64_t uptime_sec = uptime_ms / 1000;

  char buf[64] = {0};
  snprintf(buf, 63, "[%s.%03d, %02d:%02d:%02d.%03d] ",
                    FormatUnixTime(unix_ms/1000).c_str(),
                    int(unix_ms % 1000),
                    int(uptime_sec / 3600),
                    int(uptime_sec % 3600 / 60),
                    int(uptime_sec % 60),
                    int(uptime_ms % 1000));
  return std::string(buf);
}

int main() {
  TimerManager tm(10);

  {
    // auto timer_point = std::chrono::system_clock::from_time_t(time(NULL) / 3600 * 3600 + 3600); // start since next clock hour
    auto timer_point = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
    tm.ScheduleAtFixedRate(timer_point, 2000, []() { 
        std::cout << Uptime() << "do some work with ScheduleAtFixedRate." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    });
  }

  {
    tm.ScheduleAtFixedDelay(1000, 2000, []() { 
        std::cout << Uptime() << "do some work with ScheduleAtFixedDelay." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
  }

  {
    int64_t timeout_ms = 197;
    tm.ScheduleRelative(timeout_ms, [timeout_ms]() { 
        std::cout << Uptime() << "do some work with ScheduleRelative after " << timeout_ms << "ms" << std::endl;
    });
  }

  {
    auto now = std::chrono::system_clock::now();
    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    int64_t unix_time_ms = now_ms + 2000;
    tm.ScheduleAbsolute(unix_time_ms, [unix_time_ms]() { 
        std::cout << Uptime() << "do some work with ScheduleAbsolute at " << unix_time_ms << "(ms since 1970)" << std::endl;
    });
  }

  {
    std::chrono::milliseconds timeout(1200);
    tm.ScheduleRelative(timeout, []() { 
        std::cout << Uptime() << "do some work with ScheduleRelative after some duration." << std::endl;
    });
  }

  { 
    auto timer_point = std::chrono::system_clock::now() + std::chrono::milliseconds(800);
    tm.ScheduleAbsolute(timer_point, []() {
        std::cout << Uptime() << "do some work with ScheduleAbsolute at some time_point." << std::endl;
    });
  }
  std::this_thread::sleep_for(std::chrono::seconds(86400));
  return 0;
}

