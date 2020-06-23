#include "timer_manager.h"

TimerManager::TimerManager(size_t worker_threads) : soonest_changed_(false),  stopped_(false) {
  for(size_t i = 0; i < worker_threads; ++i) {
    std::thread th([this]{ TimerWorker();}); // FIXME: 不要在ctor中发布this指针到其他thread
    th.detach();
    timer_threads_.push_back(std::move(th));
  }
}

void TimerManager::Start() {
}

TimerManager::~TimerManager() {
  {
    std::unique_lock<std::mutex> lock(jobs_mutex_); // 对condvar所依赖条件的更新必需要加锁, 以确保能通知通知的顺序
    stopped_ = true;
  }
  jobs_cv_.notify_all();
}

void TimerManager::ScheduleAtFixedRate(const std::chrono::system_clock::time_point& first_run_point,
                         int64_t interval,
                         const std::function<void(void)>& job) {
  auto next_run_point = first_run_point + std::chrono::milliseconds(interval);
  auto job_with_rate = [this, next_run_point, interval, job]() {
          ScheduleAtFixedRate(next_run_point, interval, job); 
          job();
      };
  ScheduleAbsolute(first_run_point, job_with_rate);
}

void TimerManager::ScheduleAtFixedDelay(int64_t inital_delay,
                          int64_t interval,
                          const std::function<void(void)>& job) { // TODO : std::function 的move ctor 性能是否更好?
  auto job_with_dealy = [this, interval, job]() {
          job();
          ScheduleAtFixedDelay(interval, interval, job); 
      };
  ScheduleRelative(inital_delay, job_with_dealy);
}

void TimerManager::ScheduleRelative(int64_t timeout_ms,
                      const std::function<void(void)>& job) {
  auto tp = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
  ScheduleAbsolute(tp, job);
}

void TimerManager::ScheduleAbsolute(int64_t unix_time_ms, const std::function<void(void)>& job) {
  auto duration = std::chrono::milliseconds(unix_time_ms);
  TimePoint tp(duration);
  ScheduleAbsolute(tp, job);
}

void TimerManager::ScheduleAbsolute(const std::chrono::system_clock::time_point& point,
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

void TimerManager::TimerWorker() {
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
          job = soonest->second; // TODO : 1. use move? 2. sizeof(std::function)?
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
      job = nullptr; // TODO : std::function 可以是nullptr
    } else {
      std::unique_lock<std::mutex> lock(jobs_mutex_);
      soonest_changed_ = false;
      jobs_cv_.wait_for(lock, to_wait, [this] {
        return soonest_changed_ || stopped_
            || (job_queue_.size() > 0 && job_queue_.begin()->first <= std::chrono::system_clock::now());
      });
    }
  }
}

