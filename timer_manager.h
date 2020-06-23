/*
 * A timer manager driven by a thread pool, written in c++11, by Yuwei Mu.
 * If you find any problem using this timer manager, please send me a mail!
 * Github : https://github.com/moogates/timer-manager
 * Mail   : moogates@163.com
*/
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_

#include <cstdint>
#include <atomic>
#include <map>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

class TimerManager {
public:
    explicit TimerManager(size_t worker_threads);
    ~TimerManager();

    void Start();

    void ScheduleRelative(int64_t timeout_ms, const std::function<void(void)>& job);
    template<class R, class P>
    void ScheduleRelative(const std::chrono::duration<R, P>& timeout,
                          const std::function<void(void)>& job) {
      auto tp = std::chrono::system_clock::now() + timeout;
      ScheduleAbsolute(tp, job);
    }


    void ScheduleAbsolute(int64_t unix_time_ms, const std::function<void(void)>& job);
    void ScheduleAbsolute(const std::chrono::system_clock::time_point& point,
                          const std::function<void(void)>& job);

    void ScheduleAtFixedRate(const std::chrono::system_clock::time_point& first_run_point,
                             int64_t interval,
                             const std::function<void(void)>& job);

    void ScheduleAtFixedDelay(int64_t inital_delay,
                              int64_t interval,
                              const std::function<void(void)>& job);

private:
    using TimePoint = std::chrono::system_clock::time_point;
    // typedef std::chrono::system_clock::time_point TimePoint;

    std::vector<std::thread> timer_threads_;
    std::multimap<TimePoint, std::function<void(void)>> job_queue_;
    std::mutex              jobs_mutex_;
    std::condition_variable jobs_cv_;
    std::atomic_bool soonest_changed_;

    std::atomic_bool stopped_;

private:
    TimerManager(TimerManager&) = delete;
    TimerManager& operator=(TimerManager&) = delete;
    // TimerManager(TimerManager&&) = delete;
    // TimerManager& operator=(TimerManager&&) = delete;

    void TimerWorker();
};

#endif // _TIMER_MANAGER_H_

