/*
 * odc_tracer.c
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#include "odc_tracer.h"


#define DUMP_BLK_SIZE (8)					/**< 表示単位（8バイト） */
#define DUMP_LINE_SIZE (DUMP_BLK_SIZE*2)	/**< 行表示単位（16バイト） */
#define DUMP_LIMIT (DUMP_LINE_SIZE*16)		/**< 表示最大行数（デフォルト） */


int ODC_Trace_max_level = 0;
size_t ODC_Trace_max_dumpsize = DUMP_LIMIT;
static FILE* ODC_Trace_output = NULL;

static const char* ODC_Trace_level_str[] = {
		"FATAL",
		"ERROR",
		"WARNING",
		"INFO",
		"INFO",
		"INFO",
		"MESSAGE",
		"DEBUG",
		"TRACE1",
		"TRACE2",
		"TRACE3",
		"TRACE4",
		"devel",
		"---",
		"---",
		"---",
};


#ifdef ENABLE_DBGTOOL
#define UNUSED_nodbg
#else /*DISABLE_DBGTOOL*/
#define UNUSED_nodbg __attribute__((unused))
#endif /*DISABLE_DBGTOOL*/

/**
 * トレーサー初期化
 * 出力先ファイルポインタにNULLを指定した場合は、標準エラー出力を用いる。
 * @param[in] tracelevel トレースレベル
 * @param[in] out 出力先ファイルポインタ
 */
void ODC_Trace_Init(int tracelevel, FILE* out)
{
	char* v;
	v = getenv("SCNDRV_DEBUG_LEVEL"); //Depreciated
	if (v) {
		int l = strtol(v, 0, 0);
		if (0 <= l && l <=255) {
			tracelevel = l;
			fprintf(stderr, "Setting scndrv trace level to %d.\n", tracelevel);
		}
	}
	v = getenv("DEBUG_LEVEL");
	if (v) {
		int l = strtol(v, 0, 0);
		if (0 <= l && l <=255) {
			tracelevel = l;
			fprintf(stderr, "Setting scndrv trace level to %d.\n", tracelevel);
		}
	}

	v = getenv("MAX_DUMP_SIZE");
	if (v) {
		int l = strtol(v, 0, 0);
		ODC_Trace_max_dumpsize = l;
		fprintf(stderr, "Setting max dump size to %zu.\n", ODC_Trace_max_dumpsize);
	}

	ODC_Trace_max_level = tracelevel;
	if (out != NULL) {
		ODC_Trace_output = out;
	} else {
		ODC_Trace_output = stderr;
	}
#ifdef ENABLE_DBGTOOL
#else /*DISABLE_DBGTOOL*/
//	{
//		int i;
//		for(i=0 ; i<16; i++)
//		fprintf(stderr,"%d >> '%s'\n", i, ODC_Trace_level_str[i]);
//	}
#endif /*DISABLE_DBGTOOL*/
}

/**
 * トレースレベル有効判定
 * 指定したレベルのトレースが有効か判定する。
 * @param[in] level トレースレベル
 * @return 判定結果
 * @retval 0 無効
 * @retval 1 有効
 */
int ODC_Trace_Ena(int level)
{
	////fprintf(stderr, "ODC_Trace_Ena level=%d, max=%d\n", level, ODC_Trace_max_level);
	return (ODC_Trace_max_level >= level);
}



/**
 * トレース出力
 *
 * @param[in] level レベル
 * @param[in] id 識別名
 * @param[in] file ファイル名（__FILE__）
 * @param[in] line 行番号（__LINE__）
 * @param[in] func 関数名（__func__）
 * @param[in] error_no エラー番号（errno）
 * @param[in] msg メッセージ（フォーマット）
 */
void ODC_Trace_Msg(
		enum ODC_Trace_Level level,
		const char* id,
		const char* file,
		unsigned long line,
		const char* func,
		int error_no,
		const char * msg, ...)
{
	char fmt[4096];

	pid_t pid = getpid();
	pthread_t tid =  pthread_self();

	int k, l;
#ifdef ENABLE_DBGTOOL
	struct timeval tv;
	struct timezone tz;
	struct tm tmv;
	gettimeofday(&tv, &tz);
	localtime_r(&tv.tv_sec, &tmv);
	k = sprintf(fmt, "[%04d/%02d/%02d %02d:%02d:%02d.%06d] %s(%d:%lx)[%s] ",
			1900+tmv.tm_year, tmv.tm_mon+1, tmv.tm_mday, tmv.tm_hour, tmv.tm_min, tmv.tm_sec, tv.tv_usec,
			ODC_Trace_level_str[level], pid, tid, id);
#else /*DISABLE_DBGTOOL*/
	//k = sprintf(fmt, "** %s[%d](%d) [%s] ** ", ODC_Trace_level_str[level], level, getpid(), id);
	k = sprintf(fmt, "** %s(%d:%lx) [%s] ** ", ODC_Trace_level_str[level], pid, tid, id);
#endif /*DISABLE_DBGTOOL*/
	l = strlen(msg);
	memcpy(&fmt[k], msg, l);
	while(l>0 && (fmt[k+l-1] == '\r' || fmt[k+l-1] == '\n') )
		l--;
	l += k;
#ifdef ENABLE_DBGTOOL
	if (error_no != 0) {
		char errmsg[1024];
		errmsg[0] = '\0';
		strerror_r(error_no,errmsg,1024);
		snprintf(&fmt[l], sizeof(fmt)-l, " [errno=%d, %s] -- %s:%lu:%s\n", error_no, errmsg, file, line, func);
	} else {
		snprintf(&fmt[l], sizeof(fmt)-l, " -- %s:%lu:%s\n", file, line, func);
	}
#else /*DISABLE_DBGTOOL*/
	if (error_no != 0) {
		char errmsg[1024];
		errmsg[0] = '\0';
		strerror_r(error_no,errmsg,1024);
		if (file != NULL && func != NULL) {
			snprintf(&fmt[l], sizeof(fmt)-l, " [errno=%d, %s] -- %s:%lu:%s\n", error_no, errmsg, file, line, func);
		} else {
			snprintf(&fmt[l], sizeof(fmt)-l, " [errno=%d, %s]\n", error_no, errmsg);
		}
	} else {
		if (file != NULL && func != NULL) {
			snprintf(&fmt[l], sizeof(fmt)-l, " -- %s:%lu:%s\n", file, line, func);
		} else {
			snprintf(&fmt[l], sizeof(fmt)-l, "\n");
		}
	}
#endif /*DISABLE_DBGTOOL*/

	va_list ap;
	va_start (ap, msg);
	vfprintf (ODC_Trace_output, fmt, ap);
	va_end (ap);
}



/**
 * ダンプ出力
 *
 * @param[in] level レベル
 * @param[in] id 識別名
 * @param[in] file ファイル名（__FILE__）
 * @param[in] line 行番号（__LINE__）
 * @param[in] func 関数名（__func__）
 * @param[in] data データ
 * @param[in] size サイズ
 */
void ODC_Trace_Dump(
		enum ODC_Trace_Level level,
		const char* id,
		const char* file UNUSED_nodbg,
		unsigned long line UNUSED_nodbg,
		const char* func UNUSED_nodbg,
		const void* data,
		size_t size)
{
	char buf[1024];
	char buf2[64];
	size_t mem_off;
	int i;
	int snip = 0;

	pid_t pid = getpid();
	pthread_t tid =  pthread_self();

#ifdef ENABLE_DBGTOOL
	fprintf(ODC_Trace_output, "%s(%d:%lx)[%s] data=%p, size=%lu -- %s:%lu:%s\n", ODC_Trace_level_str[level], pid, tid, id, data, size, file, line, func);
#else /*DISABLE_DBGTOOL*/
	fprintf(ODC_Trace_output, "** %s(%d:%lx) [%s] ** data=%p, size=%lu\n", ODC_Trace_level_str[level], pid, tid, id, data, size);
#endif /*DISABLE_DBGTOOL*/

	if (data == NULL) {
		return;
	}

	if (size > ODC_Trace_max_dumpsize) {
		size = ODC_Trace_max_dumpsize;
		snip = 1;
	}


	for(mem_off=0 ; mem_off<size ; mem_off+=DUMP_LINE_SIZE) {
		char *bp = &buf[0];
		char *bp2 = &buf2[0];

		for (i = 0; i < DUMP_LINE_SIZE; i++) {
			if (i > 0 && (i % DUMP_BLK_SIZE) == 0) {
				*bp = ' ';
				bp++;
				*bp2 = ' ';
				bp2++;
			}

			if (mem_off + i < size) {
				char * p = (char*) ((char*)data + mem_off + i);

				if (isprint(*p)) {
					*bp2 = *p;
					bp2++;
				} else {
					*bp2 = '.';
					bp2++;
				}

				if (i > 0) {
					*bp = ' ';
					bp++;
				}
				sprintf(bp, "%02x", (unsigned char) *p);
				bp += 2;
			} else {
				*bp2 = ' ';
				bp2++;
				if (i > 0) {
					*bp = ' ';
					bp++;
				}
				*bp = ' ';
				bp++;
				*bp = ' ';
				bp++;
			}
		}
		*bp = '\0';
		*bp2 = '\0';

		fprintf(ODC_Trace_output, "    %08x:%s:%s\n", (unsigned int)mem_off, buf, buf2);
	}
	if (snip) {
		fprintf(ODC_Trace_output, "    (snip)\n");
	}
}

