#ifndef __FOCUS_LOGGER_H__
#define __FOCUS_LOGGER_H__

#include "AsyncLogger.h"

///************************************************************************/
///* ���Ժ꣬��־�꣬������غ궨��                                       */
///************************************************************************/
#ifndef __FILENAME__
# if defined(_MSC_VER)
#  define __FILENAME__  \
	( strrchr(__FILE__,'\\') ? (strrchr(__FILE__,'\\')+1) : __FILE__ )
# elif defined(__unix__)
#  define __FILENAME__ __FILE__
# else
#  error unsupported compiler!
# endif
#endif

#ifndef __CURRENT_FUNCTION__
# if defined(__GNUC__)
#  define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
# elif defined(_MSC_VER)
#  define __CURRENT_FUNCTION__ __FUNCSIG__
# else
#  error unsupported compiler!
# endif
#endif

#ifndef DEBUGFMT
# define DEBUGFMT  "%s(%d)-%s"
# define DEBUGARGS __FILENAME__,__LINE__,__FUNCTION__
#endif

#if defined(_DEBUG) || defined(FS_DEBUG)
//# define LOG_DEBUG(fmt,...)  g_log.Debug("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__)
# define LOG_DEBUG(fmt,...)  \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Debug("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
		} else { \
			printf("[%8s] <" DEBUGFMT "> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvDEBUG], DEBUGARGS, ## __VA_ARGS__); \
		} \
	} while (0)
#else
# define LOG_DEBUG(fmt,...)
#endif

//#define LOG_INFO(fmt,...)  g_log.Info("<%s> " fmt, __FUNCTION__, ## __VA_ARGS__)
#define LOG_INFO(fmt,...)  \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Info("<%s> " fmt, __FUNCTION__, ## __VA_ARGS__); \
		} else { \
			printf("[%8s] <%s> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvINFO], __FUNCTION__, ## __VA_ARGS__); \
		} \
	} while (0)

//#define LOG_WARN(fmt,...)  g_log.Alarm("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__)
# define LOG_WARN(fmt,...)  \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Alarm("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
		} else { \
			fprintf(stderr, "[%8s] <" DEBUGFMT "> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvALARM], DEBUGARGS, ## __VA_ARGS__); \
		} \
	} while (0)

# define LOG_ALARM LOG_WARN

//#define LOG_ERROR(fmt,...)  g_log.Error("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__)
# define LOG_ERROR(fmt,...)  \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Error("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
		} else { \
			fprintf(stderr, "[%8s] <" DEBUGFMT "> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvERROR], DEBUGARGS, ## __VA_ARGS__); \
		} \
	} while (0)

/*
#define LOG_FATAL(fmt,...)  do \
	{ \
		g_log_fatal.Fatal("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
		g_log.Fatal("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
	} while (0)
*/
# define LOG_FATAL(fmt,...)  \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Fatal("<" DEBUGFMT "> " fmt, DEBUGARGS, ## __VA_ARGS__); \
		} else { \
			fprintf(stderr, "[%8s] <" DEBUGFMT "> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvFATAL], DEBUGARGS, ## __VA_ARGS__); \
		} \
	} while (0)

//#define LOG_FLUSH() g_log.Flush()
#define LOG_FLUSH() \
	do{ \
		if (g_log_ptr) { \
			g_log_ptr->Flush(); \
		} else { \
			fflush(stdout); \
		} \
	} while (0)

//FS_ASSERT,FS_ASSERT_MSG
#if defined(_DEBUG) || defined(FS_DEBUG)
# include <assert.h>
# define FS_ASSERT(c) do {if (!(c)){LOG_FATAL("(" #c "):Assert failed. " );assert(#c);exit(-1);}} while (0)
# define FS_ASSERT_MSG(c,m) do {if (!(c)){LOG_FATAL("(" #c "):Assert failed. Msg:%s.",(const char*)m);assert(#c);exit(-1);}} while (0)
#else
# define FS_ASSERT(c)
# define FS_ASSERT_MSG(c,m)
#endif

//////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) || defined(FS_DEBUG)
# define LOG_RecvEvt(fmt,...)  \
	do { \
		if (g_log_ptr) { \
			g_log_ptr->Debug("[recv] >%s< " fmt, __FUNCTION__, ## __VA_ARGS__); \
		} else { \
			printf("[%8s] [recv] >%s< " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvDEBUG], __FUNCTION__, ## __VA_ARGS__); \
		} \
	} while (0)

# define LOG_SendEvt(fmt,...)  \
	do { \
		if (g_log_ptr) { \
			g_log_ptr->Debug("[send] <%s> " fmt, __FUNCTION__, ## __VA_ARGS__); \
		} else { \
			printf("[%8s] [send] <%s> " fmt "\n", SMC_PSIP::g_LOGLEVELStr[SMC_PSIP::lvDEBUG], __FUNCTION__, ## __VA_ARGS__); \
		} \
	} while (0)
#else
# define LOG_RecvEvt(fmt,...)
# define LOG_SendEvt(fmt,...)
#endif

#endif //__ACD_LOGGER_H__
