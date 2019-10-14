#pragma once
#include <cstdio>
#include <ctime>
#include <cstdlib>

#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#include <future>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp> 
#include <boost/property_tree/json_parser.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
namespace pt = boost::property_tree;
// OpenCV includes
#include <opencv2/opencv.hpp>

#include <napi.h>


class FreeGo {
    static FreeGo* instance_;

public:
    static FreeGo* instance();
    void init();
    boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;
private:

}; // class end
std::string hexStr(const std::string& buff);
#define FREEGO_TRACE BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::trace)
#define FREEGO_DEBUG BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::debug)
#define FREEGO_INFO  BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::info)
#define FREEGO_WARN  BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::warning)
#define FREEGO_ERROR BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::error)
#define FREEGO_FATAL BOOST_LOG_SEV(FreeGo::instance()->lg, boost::log::trivial::fatal)
