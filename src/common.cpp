
#include "stdafx.h"
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
using namespace cv;
string test_cpp()
{    
    string ver = "Hello world from c++";
    FREEGO_TRACE <<"Hello world from c++";
    FREEGO_DEBUG <<"Hello world from c++";
    FREEGO_INFO <<"Hello world from c++";
    FREEGO_WARN <<"Hello world from c++";
    FREEGO_ERROR <<"Hello world from c++";
    FREEGO_FATAL <<"Hello world from c++";

    VideoCapture cap; // open the default camera
    cap.open(0);
    if (!cap.isOpened()) // check if we succeeded
    {
        FREEGO_DEBUG<<"can not open camera or video file"<<endl;
        return "open camera failed";
    }        
    namedWindow("Video", 1);
    for (;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        imshow("Video", frame);
        if (waitKey(30) >= 0)
            break;
    }
    // Release the camera or video cap
    cap.release();
    destroyAllWindows();
    return ver;
}
