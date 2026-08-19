// Uncomment to get minimum footprint (suitable for 1-2 concurrent calls only)
//#define PJ_CONFIG_MINIMAL_SIZE

// Uncomment to get maximum performance
//#define PJ_CONFIG_MAXIMUM_SPEED

#include <pj/config_site_sample.h>

#if defined(_WIN64)
/* third_party/msvc ships Win32 FFmpeg/SDL libs only; x64 uses DShow. */
#define PJMEDIA_HAS_VIDEO	        1
#define PJMEDIA_VIDEO_DEV_HAS_SDL       0
#define PJMEDIA_VIDEO_DEV_HAS_DSHOW     1
#define PJMEDIA_HAS_LIBAVFORMAT     0
#define PJMEDIA_HAS_LIBAVCODEC     0
#define PJMEDIA_HAS_LIBAVUTIL     0
#define PJMEDIA_HAS_LIBSWSCALE     0
#define PJMEDIA_HAS_LIBAVDEVICE     0
#define PJMEDIA_HAS_FFMPEG	        0
#else
#define PJMEDIA_HAS_VIDEO	        1
#define PJMEDIA_VIDEO_DEV_HAS_SDL       1
#define PJMEDIA_VIDEO_DEV_HAS_DSHOW     1
#define PJMEDIA_HAS_LIBAVFORMAT     1
#define PJMEDIA_HAS_LIBAVCODEC     1
#define PJMEDIA_HAS_LIBAVUTIL     1
#define PJMEDIA_HAS_LIBSWSCALE     1
#define PJMEDIA_HAS_LIBAVDEVICE     1
#define PJMEDIA_HAS_FFMPEG	        1
#endif

#ifndef PJSUA_MAX_CALLS
#define PJSUA_MAX_CALLS			1024
#endif
#undef PJSUA_MAX_CONF_PORTS
#define PJSUA_MAX_CONF_PORTS		1024
#undef PJSIP_MAX_DIALOG_COUNT
#define PJSIP_MAX_DIALOG_COUNT		2047
#undef PJSIP_MAX_TSX_COUNT
#define PJSIP_MAX_TSX_COUNT		4095
#undef PJ_IOQUEUE_MAX_HANDLES
#define PJ_IOQUEUE_MAX_HANDLES		2048

