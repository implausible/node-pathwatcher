{
  "targets": [
    {
      "target_name": "pathwatcher",
      "sources": [
        "src/main.cc",
        "src/common.cc",
        "src/common.h",
        "src/handle_map.cc",
        "src/handle_map.h",
        "src/unsafe_persistent.h",
      ],
      "include_dirs": [
        "src",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
      },
      "conditions": [
        ['OS=="win"', {
          "sources": [
            "src/pathwatcher_win.cc",
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': 1, # /EHsc
              'WarnAsError': 'true',
            },
          },
          'msvs_disabled_warnings': [
            4018,  # signed/unsigned mismatch
            4244,  # conversion from 'type1' to 'type2', possible loss of data
            4267,  # conversion from 'size_t' to 'type', possible loss of data
            4530,  # C++ exception handler used, but unwind semantics are not enabled
            4506,  # no definition for inline function
            4577,  # 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed
            4996,  # function was declared deprecated
          ],
          'defines': [
            '_WIN32_WINNT=0x0600',
          ],
        }],  # OS=="win"
        ['OS=="linux"', {
          "sources": [
            "src/pathwatcher_linux.cc",
          ],
        }],  # OS=="linux"
        ['OS!="win" and OS!="linux"', {
          "sources": [
            "src/pathwatcher_unix.cc",
          ],
        }],  # OS~="unix"
        ['OS=="mac"', {
          "cflags+": ["-fvisibility=hidden"],
          "xcode_settings": {
            "GCC_SYMBOLS_PRIVATE_EXTERN": "YES", # -fvisibility=hidden
          }
        }]
      ],
    }
  ]
}
