// Uncomment to get minimum footprint (suitable for 1-2 concurrent calls only)
//#define PJ_CONFIG_MINIMAL_SIZE

// Uncomment to get maximum performance
//#define PJ_CONFIG_MAXIMUM_SPEED

#include <pj/config_site_sample.h>

#define PJMEDIA_HAS_VIDEO	        1
#define PJMEDIA_VIDEO_DEV_HAS_SDL       1
#define PJMEDIA_VIDEO_DEV_HAS_DSHOW     1

#define PJMEDIA_HAS_LIBAVFORMAT     1
#define PJMEDIA_HAS_LIBAVCODEC     1
#define PJMEDIA_HAS_LIBAVUTIL     1
#define PJMEDIA_HAS_LIBSWSCALE     1
#define PJMEDIA_HAS_LIBAVDEVICE     1
#define PJMEDIA_HAS_FFMPEG	        1

