/*
 * tracer.h
 * Tracer API.
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef TRACER_H_
#define TRACER_H_
#include <stdio.h>
#include <errno.h>

#ifdef ENABLE_DBGTOOL
#else 
#endif /*DISABLE_DBGTOOL*/


#ifdef	__cplusplus
extern "C"
{
#endif

enum ODC_Trace_Level
{
	TL_FATAL=0,
	TL_ERROR,
	TL_WARNING,
	TL_INFO_H,
	TL_INFO,
	TL_INFO_L,
	TL_MESSAGE,
	TL_DEBUG,
	TL_TRACE1,
	TL_TRACE2,
	TL_TRACE3,
	TL_TRACE4,
	TL_TEMPORARY	
};

extern int ODC_Trace_max_level;
void ODC_Trace_Init(int tracelevel, FILE* out);
int ODC_Trace_Ena(int level);
void ODC_Trace_Msg(
		enum ODC_Trace_Level level,
		const char* id,
		const char* file,
		unsigned long line,
		const char* func,
		int error_no,
		const char * msg, ...);
void ODC_Trace_Dump(
		enum ODC_Trace_Level level,
		const char* id,
		const char* file,
		unsigned long line,
		const char* func,
		const void* data,
		size_t size);
#ifdef	__cplusplus
};
#endif

#define T_INIT(LEVEL, OUTFP) {\
	if ((LEVEL) < TL_WARNING) \
		ODC_Trace_Init(TL_WARNING, (OUTFP));\
	else \
		ODC_Trace_Init((LEVEL), (OUTFP));\
}
#define T_LEVEL() (ODC_Trace_max_level)

#ifdef ENABLE_DBGTOOL
#define T_TRACE(LEVEL, ID, ERRNO, ...) \
	if(ODC_Trace_Ena((LEVEL))) {ODC_Trace_Msg((LEVEL), ID, __FILE__, __LINE__, __func__, (ERRNO), __VA_ARGS__);}
#define T_TRACE_DUMP(LEVEL, ID, DATA, SIZE) \
	if(ODC_Trace_Ena((LEVEL))) {ODC_Trace_Dump((LEVEL), ID, __FILE__, __LINE__, __func__, DATA, SIZE);}

#define T_FATAL_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_FATAL, ID, DATA, SIZE)
#define T_ERR_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_ERROR, ID, DATA, SIZE)
#define T_WRN_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_WARNING, ID, DATA, SIZE)
#define T_INFO_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_INFO_L, ID, DATA, SIZE)
#define T_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_TRACE4, ID, DATA, SIZE)

#define T_TMP(ID, ERRNO, ...)	ODC_Trace_Msg(TL_TEMPORARY, ID, __FILE__, __LINE__, __func__, (ERRNO), __VA_ARGS__)

#define T_FATAL(ID, ERRNO, ...) T_TRACE(TL_FATAL, ID, ERRNO, __VA_ARGS__)
#define T_ERR(ID, ERRNO, ...) T_TRACE(TL_ERROR, ID, ERRNO, __VA_ARGS__)
#define T_WARN(ID, ERRNO, ...) T_TRACE(TL_WARNING, ID, ERRNO, __VA_ARGS__)
#define T_INFO_H(ID, ...) T_TRACE(TL_INFO_H, ID, (0), __VA_ARGS__)
#define T_INFO(ID, ...) T_TRACE(TL_INFO, ID, (0), __VA_ARGS__)
#define T_INFO_L(ID, ...) T_TRACE(TL_INFO_L, ID, (0), __VA_ARGS__)
#define T_MESSAGE(ID, ...)	T_TRACE(TL_MESSAGE, ID, (0), __VA_ARGS__)
#define T_DBG(ID, ...) T_TRACE(TL_DEBUG, ID, (0), __VA_ARGS__)
#define T_TRC1(ID, ...) T_TRACE(TL_TRACE1, ID, (0), __VA_ARGS__)
#define T_TRC2(ID, ...) T_TRACE(TL_TRACE2, ID, (0), __VA_ARGS__)
#define T_TRC3(ID, ...) T_TRACE(TL_TRACE3, ID, (0), __VA_ARGS__)
#define T_TRC4(ID, ...) T_TRACE(TL_TRACE4, ID, (0), __VA_ARGS__)
#define T_ENA_FATAL() (ODC_Trace_Ena(TL_FATAL))
#define T_ENA_ERR() (ODC_Trace_Ena(TL_ERROR))
#define T_ENA_WARN() (ODC_Trace_Ena(TL_WARNING))
#define T_ENA_INFO_H() (ODC_Trace_Ena(TL_INFO_H))
#define T_ENA_INFO() (ODC_Trace_Ena(TL_INFO))
#define T_ENA_INFO_L() (ODC_Trace_Ena(TL_INFO_L))
#define T_ENA_MESSAGE() (ODC_Trace_Ena(TL_MESSAGE))
#define T_ENA_DBG() (ODC_Trace_Ena(TL_DEBUG))
#define T_ENA_TRC1() (ODC_Trace_Ena(TL_TRACE1))
#define T_ENA_TRC2() (ODC_Trace_Ena(TL_TRACE2))
#define T_ENA_TRC3() (ODC_Trace_Ena(TL_TRACE3))
#define T_ENA_TRC4() (ODC_Trace_Ena(TL_TRACE4))
#else 
#define T_TRACE(LEVEL, ID, ERRNO, ...) \
	if(ODC_Trace_Ena((LEVEL))) {ODC_Trace_Msg((LEVEL), ID, NULL, 0, NULL, (ERRNO), __VA_ARGS__);}
#define T_TRACE_DUMP(LEVEL, ID, DATA, SIZE) \
	if(ODC_Trace_Ena((LEVEL))) {ODC_Trace_Dump((LEVEL), ID, NULL, 0, NULL, DATA, SIZE);}

#define T_FATAL_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_FATAL, ID, DATA, SIZE)
#define T_ERR_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_ERROR, ID, DATA, SIZE)
#define T_WRN_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_WARNING, ID, DATA, SIZE)
#define T_INFO_DUMP(ID, DATA, SIZE) T_TRACE_DUMP(TL_INFO_L, ID, DATA, SIZE)
#define T_DUMP(ID, DATA, SIZE)

#define T_TMP(ID, ERRNO, ...)	ODC_Trace_Msg(TL_TEMPORARY, ID, __FILE__, __LINE__, __func__, (ERRNO), __VA_ARGS__)

#define T_FATAL(ID, ERRNO, ...) T_TRACE(TL_FATAL, ID, ERRNO, __VA_ARGS__)
#define T_ERR(ID, ERRNO, ...) T_TRACE(TL_ERROR, ID, ERRNO, __VA_ARGS__)
#define T_WARN(ID, ERRNO, ...) T_TRACE(TL_WARNING, ID, ERRNO, __VA_ARGS__)
#define T_INFO_H(ID, ...) T_TRACE(TL_INFO_H, ID, (0), __VA_ARGS__)
#define T_INFO(ID, ...) T_TRACE(TL_INFO, ID, (0), __VA_ARGS__)
#define T_INFO_L(ID, ...) T_TRACE(TL_INFO_L, ID, (0), __VA_ARGS__)
#define T_MESSAGE(ID, ...)	T_TRACE(TL_MESSAGE, ID, (0), __VA_ARGS__)
#define T_DBG(ID, ...)
#define T_TRC1(ID, ...)
#define T_TRC2(ID, ...)
#define T_TRC3(ID, ...)
#define T_TRC4(ID, ...)
#define T_ENA_FATAL() (ODC_Trace_Ena(TL_FATAL))
#define T_ENA_ERR() (ODC_Trace_Ena(TL_ERROR))
#define T_ENA_WARN() (ODC_Trace_Ena(TL_WARNING))
#define T_ENA_INFO_H() (ODC_Trace_Ena(TL_INFO_H))
#define T_ENA_INFO() (ODC_Trace_Ena(TL_INFO))
#define T_ENA_INFO_L() (ODC_Trace_Ena(TL_INFO_L))
#define T_ENA_MESSAGE() (ODC_Trace_Ena(TL_MESSAGE))
#define T_ENA_DBG() (false)
#define T_ENA_TRC1() (false)
#define T_ENA_TRC2() (false)
#define T_ENA_TRC3() (false)
#define T_ENA_TRC4() (false)
#endif /*DISABLE_DBGTOOL*/


#endif /* TRACER_H_ */
