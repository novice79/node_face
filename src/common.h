#pragma once
// #define _HAS_EXCEPTIONS 1


template<typename T>
class TSQueue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    TSQueue()
    {}
    TSQueue(TSQueue const& other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue=other.data_queue;
    }
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        value=data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};

class FreeGo {
    static FreeGo* instance_;

public:
    static FreeGo* instance();
    void init();
    boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
private:

}; // class end

#define FREEGO_TRACE BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::trace)
#define FREEGO_DEBUG BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::debug)
#define FREEGO_INFO  BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::info)
#define FREEGO_WARN  BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::warning)
#define FREEGO_ERROR BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::error)
#define FREEGO_FATAL BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::fatal)
