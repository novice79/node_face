
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include "napi-thread-safe-callback.hpp"
#include "common.h"
#include "face.h"
using namespace dlib;
using namespace std;
#pragma warning( disable : 4503 )
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------
std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel>& img
);
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
void buffer_delete_callback(Napi::Env env, uchar* data, int* hint) {
    // FREEGO_TRACE <<"in buffer_delete_callback, hint="<<*hint<<endl;
//   delete reinterpret_cast<vector<unsigned char> *> (the_vector);
}
// #define FACE_DOWNSAMPLE_RATIO 4
#define FACE_DOWNSAMPLE_RATIO 2
#define SKIP_FRAMES 2
int test(Napi::Function &cb)
{
    auto callback = std::make_shared<ThreadSafeCallback>(cb);
    std::thread t([=]() {
        cv::VideoCapture cap(0);
        cv::Mat im, original_im;
        cv::Mat im_small, im_display;
        string face_trait;
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("lm.dat") >> pose_model;
        anet_type net;
        deserialize("net.dat") >> net;
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
            std::vector<matrix<rgb_pixel>> faces_to_net;
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
                full_object_detection shape = pose_model(cimg, r);
                //////////////////////////////////
                matrix<rgb_pixel> face_chip;
                extract_image_chip(cimg, get_face_chip_details(shape,150,0.25), face_chip);
                faces_to_net.push_back(move(face_chip));
                //////////////////////////////////
                shapes.push_back(shape);
                // Custom Face Render
                render_face(im, shape);
            }
            // face_trait = "";
            // if(1 == face_count)
            // {
            //     // This call asks the DNN to convert each face image in faces into a 128D vector.
            //     // In this 128D vector space, images from the same person will be close to each other
            //     // but vectors from different people will be far apart.  So we can use these vectors to
            //     // identify if a pair of images are from the same person or from different people.  
            //     std::vector<matrix<float,0,1>> face_descriptors = net(faces_to_net);
            //     stringstream traits;
            //     serialize(face_descriptors[0], traits);
            //     face_trait = traits.str();
            // }
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
int test0()
{
    try
    {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            cerr << "Unable to connect to camera" << endl;
            return 1;
        }

        image_window win;

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        // Grab and process frames until the main window is closed by the user.
        while (!win.is_closed())
        {
            // Grab a frame
            cv::Mat temp;
            if (!cap.read(temp))
            {
                break;
            }
            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            cv_image<bgr_pixel> cimg(temp);

            // Detect faces
            std::vector<rectangle> faces = detector(cimg);
            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i)
                shapes.push_back(pose_model(cimg, faces[i]));

            // Display it all on the screen
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));
        }
    }
    catch (serialization_error &e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl
             << e.what() << endl;
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
    }
}

// ----------------------------------------------------------------------------------------
std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel>& img
)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently. They are also randomly
    // mirrored left to right.
    thread_local dlib::rand rnd;

    std::vector<matrix<rgb_pixel>> crops; 
    for (int i = 0; i < 100; ++i)
        crops.push_back(jitter_image(img,rnd));

    return crops;
}
// ----------------------------------------------------------------------------------------