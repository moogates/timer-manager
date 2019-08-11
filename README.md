# timer-manager
A timer manager driven by a thread pool, written in c++11, by Yuwei Mu.  If you find any
problems using this timer manager, please send me a mail(moogates@163.com)!
# Example
## ScheduleAtFixedRate
```
  TimerManager tm(8);
  // auto timer_point = std::chrono::system_clock::from_time_t(time(NULL) / 3600 * 3600 + 3600); // start since next clock hour
  auto timer_point = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
  tm.ScheduleAtFixedRate(timer_point, 2000, []() {
      std::cout << Uptime() << "do some work with ScheduleAtFixedRate." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  });
```
Output:
```
[2019/08/11 14:18:09.541, 00:00:01.004] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:11.542, 00:00:03.004] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:13.538, 00:00:05.000] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:15.541, 00:00:07.004] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:17.540, 00:00:09.003] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:19.542, 00:00:11.004] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:21.538, 00:00:13.000] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:23.539, 00:00:15.001] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:25.539, 00:00:17.001] do some work with ScheduleAtFixedRate.
[2019/08/11 14:18:27.538, 00:00:19.000] do some work with ScheduleAtFixedRate.
```

## ScheduleAtFixedDelay
```
  TimerManager tm(4);
  tm.ScheduleAtFixedDelay(1000, 2000, []() { 
      std::cout << Uptime() << "do some work with ScheduleAtFixedDelay." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
  });
```
Output:
```
[2019/08/11 14:19:52.725, 00:00:01.000] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:19:55.232, 00:00:03.506] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:19:57.734, 00:00:06.009] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:00.238, 00:00:08.512] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:02.740, 00:00:11.014] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:05.244, 00:00:13.519] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:07.749, 00:00:16.023] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:10.255, 00:00:18.530] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:12.758, 00:00:21.033] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:15.267, 00:00:23.541] do some work with ScheduleAtFixedDelay.
[2019/08/11 14:20:17.769, 00:00:26.044] do some work with ScheduleAtFixedDelay.
```
## ScheduleRelative
  TimerManager tm(4);

  {
    int64_t timeout_ms = 197;
    tm.ScheduleRelative(timeout_ms, [timeout_ms]() { 
        std::cout << Uptime() << "do some work with ScheduleRelative after " << timeout_ms << "ms" << std::endl;
    });
  }

  {
    std::chrono::milliseconds timeout(1200);
    tm.ScheduleRelative(timeout, []() { 
        std::cout << Uptime() << "do some work with ScheduleRelative after some duration." << std::endl;
    });
  }
```
## ScheduleAbsolute
```
  TimerManager tm(4);
  { 
    // schedules at a std::chrono time_point (800ms later)
    auto timer_point = std::chrono::system_clock::now() + std::chrono::milliseconds(800);
    tm.ScheduleAbsolute(timer_point, []() {
        std::cout << Uptime() << "do some work with ScheduleAbsolute at some time_point." << std::endl;
    });
  }

  {
    auto now = std::chrono::system_clock::now();
    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // schedules at a time point in ms count (2000ms later)
    int64_t unix_time_ms = now_ms + 2000;
    tm.ScheduleAbsolute(unix_time_ms, [unix_time_ms]() { 
        std::cout << Uptime() << "do some work with ScheduleAbsolute at " << unix_time_ms << "(ms since 1970)" << std::endl;
    });
  }
```

