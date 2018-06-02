

#include "napi-thread-safe-callback.hpp"
#include "common.h"
#include "def.h"
#include "video.h"

using namespace std;


void draw_polyline(cv::Mat &img, const dlib::full_object_detection &d, const int start, const int end, bool isClosed = false)
{
    std::vector<cv::Point> points;
    for (int i = start; i <= end; ++i)
    {
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(255, 0, 0), 2, 16);
}

void render_face(cv::Mat &img, const dlib::full_object_detection &d)
{
    DLIB_CASSERT(
        d.num_parts() == 68,
        "\n\t Invalid inputs were given to this function. "
            << "\n\t d.num_parts():  " << d.num_parts());

    draw_polyline(img, d, 0, 16);        // Jaw line
    draw_polyline(img, d, 17, 21);       // Left eyebrow
    draw_polyline(img, d, 22, 26);       // Right eyebrow
    draw_polyline(img, d, 27, 30);       // Nose bridge
    draw_polyline(img, d, 30, 35, true); // Lower nose
    draw_polyline(img, d, 36, 41, true); // Left eye
    draw_polyline(img, d, 42, 47, true); // Right Eye
    draw_polyline(img, d, 48, 59, true); // Outer lip
    draw_polyline(img, d, 60, 67, true); // Inner lip
}
int g_i = 444;
shape_predictor sp68;
void buffer_delete_callback(Napi::Env env, uchar* data, int* hint) {
    // FREEGO_TRACE <<"in buffer_delete_callback, hint="<<*hint<<endl;
//   delete reinterpret_cast<vector<unsigned char> *> (the_vector);
}

int capture_video(Napi::Function &cb)
{
    auto callback = std::make_shared<ThreadSafeCallback>(cb);
    std::thread t([=]() {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())  
        {
            FREEGO_TRACE <<"打开摄像头失败！"<<endl;
            return;
        }        
        cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);

        cv::Mat im, original_im;
        cv::Mat im_small, im_display;
        string face_trait;
        frontal_face_detector detector = get_frontal_face_detector();        
        deserialize("lm68.dat") >> sp68;               
        int count = 0;
        std::vector<rectangle> faces;
        std::vector<uchar> original, filtered;
        // FREEGO_TRACE <<"in c++ thread\n";
        while (1)
        {
            // Grab a frame
            cap >> im;
            original_im = im.clone();
            // Resize image for face detection
            cv::resize(im, im_small, cv::Size(), 1.0 / FACE_DOWNSAMPLE_RATIO, 1.0 / FACE_DOWNSAMPLE_RATIO);
            // Change to dlib's image format. No memory is copied.
            cv_image<bgr_pixel> cimg_small(im_small);
            cv_image<bgr_pixel> cimg(im);
            faces = detector(cimg_small);
            // Detect faces on resize image
            // if (count % SKIP_FRAMES == 0)
            // {
            //     faces = detector(cimg_small);
            // }
            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            uint32_t face_count = faces.size();
            for (uint32_t i = 0; i < face_count; ++i)
            {
                // Resize obtained rectangle for full resolution image.
                rectangle r(
                    (long)(faces[i].left() * FACE_DOWNSAMPLE_RATIO),
                    (long)(faces[i].top() * FACE_DOWNSAMPLE_RATIO),
                    (long)(faces[i].right() * FACE_DOWNSAMPLE_RATIO),
                    (long)(faces[i].bottom() * FACE_DOWNSAMPLE_RATIO));
                // Landmark detection on full sized image
                full_object_detection shape = sp68(cimg, r);
                shapes.push_back(shape);
                // Custom Face Render
                render_face(im, shape);
            }           
            imencode(".jpg", original_im, original);
            imencode(".jpg", im, filtered);
            // Call back with result
            callback->call([&original, &filtered, face_count, &face_trait](Napi::Env env, std::vector<napi_value> &args) {
                // This will run in main thread and needs to construct the
                // arguments for the call
                args = {
                    Napi::Buffer<uchar>::New(env, (uchar*)&original[0], original.size(), buffer_delete_callback, &g_i ),
                    Napi::Buffer<uchar>::New(env, (uchar*)&filtered[0], filtered.size(), buffer_delete_callback, &g_i ),
                    Napi::Number::New(env, face_count)
                    // Napi::Buffer<uchar>::New(env, (uchar*)&face_trait[0], face_trait.size(), buffer_delete_callback, &g_i )
                };
            });
            // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
    });
    t.detach();
    return 0;
}
