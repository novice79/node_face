
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

using namespace dlib;
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
int test1()
{
    return 3;
}
#define FACE_DOWNSAMPLE_RATIO 4
#define SKIP_FRAMES 2
int test()
{
    cv::VideoCapture cap(0);
    cv::Mat im;
    cv::Mat im_small, im_display;

    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor pose_model;
    deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
    int count = 0;
    std::vector<rectangle> faces;
    // Grab a frame
    cap >> im;

    // Resize image for face detection
    cv::resize(im, im_small, cv::Size(), 1.0 / FACE_DOWNSAMPLE_RATIO, 1.0 / FACE_DOWNSAMPLE_RATIO);

    // Change to dlib's image format. No memory is copied.
    cv_image<bgr_pixel> cimg_small(im_small);
    cv_image<bgr_pixel> cimg(im);

    // Detect faces on resize image
    if (count % SKIP_FRAMES == 0)
    {
        faces = detector(cimg_small);
    }

    // Find the pose of each face.
    std::vector<full_object_detection> shapes;
    for (unsigned long i = 0; i < faces.size(); ++i)
    {
        // Resize obtained rectangle for full resolution image.
        rectangle r(
            (long)(faces[i].left() * FACE_DOWNSAMPLE_RATIO),
            (long)(faces[i].top() * FACE_DOWNSAMPLE_RATIO),
            (long)(faces[i].right() * FACE_DOWNSAMPLE_RATIO),
            (long)(faces[i].bottom() * FACE_DOWNSAMPLE_RATIO));

        // Landmark detection on full sized image
        full_object_detection shape = pose_model(cimg, r);
        shapes.push_back(shape);
        // Custom Face Render
        render_face(im, shape);
        if(0 == i)
        {
            imwrite( "aaa.jpg", im );
        }
    }
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
        while(!win.is_closed())
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
    catch(serialization_error& e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }
}