
#include "common.h"

using namespace std;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

FreeGo* FreeGo::instance_ = 0;
FreeGo* FreeGo::instance()
{
	if(!instance_)
	{
		instance_ = new FreeGo;
		instance_->init();
	}
	return instance_;
}

void FreeGo::init()
{
    boost::log::add_file_log
    (
        boost::log::keywords::file_name = "cxx_%N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
		boost::log::keywords::auto_flush = true,
        boost::log::keywords::format = "[%TimeStamp%]: %Message%"
    );
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::trace
	);
	logging::add_common_attributes();
}

string test_cpp()
{    
    string ver = "Hello world from c++";
    FREEGO_TRACE <<"Hello world from c++";
    FREEGO_DEBUG <<"Hello world from c++";
    FREEGO_INFO <<"Hello world from c++";
    FREEGO_WARN <<"Hello world from c++";
    FREEGO_ERROR <<"Hello world from c++";
    FREEGO_FATAL <<"Hello world from c++";
    return ver;
}
