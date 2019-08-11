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
    typedef std::chrono::system_clock::time_point TimePoint;
public:
    explicit TimerManager(size_t thread_count) : soonest_changed_(false),  stopped_(false) {
      for(size_t i = 0; i < thread_count; ++i) {
        std::thread th([this]{ this->TimerWorker();});
        th.detach();
        timer_threads_.emplace_back(std::move(th));
      }
    }
    TimerManager(TimerManager&) = delete;
    TimerManager& operator=(TimerManager&) = delete;

    ~TimerManager() {
      stopped_ = true;
      jobs_cv_.notify_all();
    }

    void ScheduleAtFixedRate(const std::chrono::system_clock::time_point& first_run_point,
                             int64_t interval,
                             const std::function<void(void)>& job) {
      auto next_run_point = first_run_point + std::chrono::milliseconds(interval);
      auto job_with_rate = [this, next_run_point, interval, job]() {
              ScheduleAtFixedRate(next_run_point, interval, job); 
              job();
          };
      ScheduleAbsolute(first_run_point, job_with_rate);
    }

    void ScheduleAtFixedDelay(int64_t inital_delay,
                              int64_t interval,
                              const std::function<void(void)>& job) {
      auto job_with_dealy = [this, interval, job]() {
              job();
              ScheduleAtFixedDelay(interval, interval, job); 
          };
      ScheduleRelative(inital_delay, job_with_dealy);
    }

    template<class R, class P>
    void ScheduleRelative(const std::chrono::duration<R, P>& timeout,
                          const std::function<void(void)>& job) {
      auto tp = std::chrono::system_clock::now() + timeout;
      ScheduleAbsolute(tp, job);
    }

    void ScheduleRelative(int64_t timeout_ms,
                          const std::function<void(void)>& job) {
      auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
      ScheduleAbsolute(tp, job);
    }

    void ScheduleAbsolute(int64_t unix_time_ms, const std::function<void(void)>& job) {
      auto duration = std::chrono::milliseconds(unix_time_ms);
      std::chrono::system_clock::time_point tp(duration);
      ScheduleAbsolute(tp, job);
    }

    void ScheduleAbsolute(const std::chrono::system_clock::time_point& point,
                          const std::function<void(void)>& job) {
      {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        if (job_queue_.empty() || point <= job_queue_.begin()->first) {
          soonest_changed_ = true;
        }
        job_queue_.insert(std::make_pair(point, job));
      }
      if (soonest_changed_) {
        jobs_cv_.notify_one();
      }
    }

private:
    std::vector<std::thread> timer_threads_;
    std::multimap<TimePoint, std::function<void(void)>> job_queue_;
    std::mutex              jobs_mutex_;
    std::condition_variable jobs_cv_;
    std::atomic_bool soonest_changed_;

    std::atomic_bool stopped_;

    bool NeedWakeUp() const {
      return soonest_changed_ || stopped_
        || (job_queue_.size() > 0 && job_queue_.begin()->first <= std::chrono::system_clock::now());
    }

    void TimerWorker() {
      while(!stopped_) {
        std::function<void(void)> job;
        std::chrono::milliseconds to_wait;

        {
          std::lock_guard<std::mutex> lock(jobs_mutex_);
          if (job_queue_.empty()) {
             to_wait = std::chrono::milliseconds(3600 * 1000);
          } else {
            TimePoint now = std::chrono::system_clock::now();
            auto soonest = job_queue_.begin();
            if (soonest->first <= now) {
              job = soonest->second;
              job_queue_.erase(soonest);
              soonest_changed_ = true;
            } else {
              to_wait = std::chrono::duration_cast<std::chrono::milliseconds>(soonest->first - now);
            }
          }
        }

        if (job) {
          jobs_cv_.notify_one();
          job();
          job = std::function<void(void)>();
        } else {
          std::unique_lock<std::mutex> lock(jobs_mutex_);
          soonest_changed_ = false;
          jobs_cv_.wait_for(lock, to_wait, [this]{return this->NeedWakeUp(); });
        }
      }
    }
};

#endif // _TIMER_MANAGER_H_

