{
  "targets": [
    {
      "target_name": "addon",
      "cflags!": [ '-fno-rtti', "-fno-exceptions" ],
      "cflags_cc!": [ '-fno-rtti', "-fno-exceptions" ],
      "cflags_cc": [ "-std=c++17", "-Wno-deprecated" ],
      "sources": [ 
        "./src/addon.cc",
        "./src/common.cpp",
        "./src/worker.cpp",
        "./src/face.cpp",
        ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "/vcpkg/installed/x64-linux/include",
        "/vcpkg/installed/x64-linux/include/opencv4"
      ],
      'defines': [ 
        # '_HAS_EXCEPTIONS=1',
        # 'NAPI_DISABLE_CPP_EXCEPTIONS', 
      ],

      'variables': {
        'lib_dir%': '/vcpkg/installed/x64-linux/lib'
      },
      'libraries': [
        # for gcc/g++ order is important
        "-L<(lib_dir)"        
        ,'-lboost_date_time'
        ,'-lboost_regex'
        ,'-lboost_atomic'
        ,'-lboost_chrono'
        ,'-lboost_thread'   
        ,'-lboost_log_setup'    
        ,'-lboost_log'       
        ,'-lboost_locale'
        ,'-lboost_filesystem'
        ,'-lboost_system'

        ,'-ldlib'
        ,'-lopencv_calib3d'       
        ,'-lopencv_dnn'
        ,'-lopencv_features2d'
        ,'-lopencv_flann'
        ,'-lopencv_highgui'
        ,'-lopencv_imgcodecs'
        ,'-lopencv_imgproc'
        ,'-lopencv_ml'
        ,'-lopencv_objdetect'
        ,'-lopencv_photo'
        ,'-lopencv_stitching'
        ,'-lopencv_video'
        ,'-lopencv_videoio'
        ,'-lopencv_core'

        ,'-lwebp'
        ,'-lwebpdecoder'
        ,'-lwebpdemux'
        ,'-ljpeg'
        ,'-lpng'
        ,'-ltiff'
        ,'-ltiffxx'
        ,'-lturbojpeg'
        ,'-llzma'
        ,'-llapack'
        ,'-lf2c'

        ,'-lopenblas'
        ,'-lfftw3'
        ,'-lfftw3f'
        ,'-lfftw3l'  
        ,'-lz'      
        ,'-ldl'
      ],
      # "ldflags": [
      #     "-Wl,-z,defs"
      # ],
      'configurations': {
        'Release': {

        }
      }
    }
  ]
}
