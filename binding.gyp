{
  "targets": [
    {
      "target_name": "addon",
      'msvs_precompiled_header': 'src/stdafx.h',
      'msvs_precompiled_source': 'src/stdafx.cpp',
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ 
        "./src/stdafx.cpp",
        "./src/addon.cc",
        "./src/common.cpp"
        ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "E:/cpp_libs/boost/win64/include/boost-1_66"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS', 'WIN32_LEAN_AND_MEAN' ],
      'variables': {
        'boost_lib%': 'E:/cpp_libs/boost/win64/lib'
      },
      'libraries': [
        '-l<(boost_lib)/libboost_system-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_date_time-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_regex-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_atomic-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_chrono-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_log_setup-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_thread-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_filesystem-vc140-mt-s-x64-1_66.lib'
        ,'-l<(boost_lib)/libboost_log-vc140-mt-s-x64-1_66.lib'
      ],
      'configurations': {
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {             
              # enable rtti
              'RuntimeTypeInfo': 'true',
              'ExceptionHandling': 1
              # ,'RuntimeLibrary': 2, # shared release
            }
          }
        }
      }
    }
  ]
}
