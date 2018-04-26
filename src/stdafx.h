#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <cstdio>
#include <cstdlib>

#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
//#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/property_tree/ptree.hpp> 
#include <boost/property_tree/json_parser.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>

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
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"

#pragma warning(disable:4818 4819)
template <typename T>
struct FGST
{
    FGST(const FGST&) = delete;
    FGST& operator=(const FGST&) = delete;
protected:
    FGST(){}
public:
    typedef T object_type;

    static object_type* instance()
    {
      static object_type obj;
      return &obj;
    }
};
