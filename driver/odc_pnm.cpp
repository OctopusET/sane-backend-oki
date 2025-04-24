/*
 * odc_pnm.cpp
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "odc_boundary.h"
#include "odc_misc.h"

#include "odc_pnm.h"

#include "odc_tracer.h"
#define TRCID "pnm"
#define TRACE(LEVEL, ERRNO, ...) T_TRACE((LEVEL), TRCID, ERRNO, __VA_ARGS__)
#define FATAL(ERRNO, ...) T_FATAL(TRCID, ERRNO, __VA_ARGS__)
#define ERR(ERRNO, ...) T_ERR(TRCID, ERRNO, __VA_ARGS__)
#define WRN(ERRNO, ...) T_WARN(TRCID, ERRNO, __VA_ARGS__)
#define INFOh(...) T_INFO_H(TRCID, __VA_ARGS__)
#define INFO(...) T_INFO(TRCID, __VA_ARGS__)
#define INFOl(...) T_INFO_L(TRCID, __VA_ARGS__)
#define MESSAGE(...) T_MESSAGE(TRCID, __VA_ARGS__)
#define DBG(...) T_DBG(TRCID, __VA_ARGS__)
#define TRC(...) T_TRC1(TRCID, __VA_ARGS__)
#define TRC2(...) T_TRC2(TRCID, __VA_ARGS__)
#define TRC3(...) T_TRC3(TRCID, __VA_ARGS__)
#define TRC4(...) T_TRC4(TRCID, __VA_ARGS__)
#define TMP(...) T_TMP(TRCID, 0, __VA_ARGS__)
#define DUMP(DATA, SIZE) T_DUMP(TRCID, DATA, SIZE)

namespace odc {




PNMWriter::PNMWriter() : size()
{
	this->path = NULL;
	this->fp = NULL;
}
PNMWriter::~PNMWriter()
{
	this->close();
}

int PNMWriter::open(const char* path, int w, int h)
{
	if (this->fp) {
		WRN(0, "already opened.");
		return -1;
	}

	this->size.set(w, h);
	this->path = path;

	this->fp = fopen(this->path, "wb");
	if (this->fp == NULL) {
		ERR(errno, "file open error. (path=%s)", this->path);
		this->path = NULL;
		return -1;
	}

	if (this->write_header() < 0) {
		this->close();
		return -1;
	}

	return 0;
}

int PNMWriter::open(FILE* fp, int w, int h)
{
	if (this->path) {
		WRN(0, "already opened. (path=%s)", this->path);
		return -1;
	}
	if (this->fp) {
		WRN(0, "already opened.");
		return -1;
	}

	this->size.set(w, h);
	this->path = NULL;
	this->fp = fp;

	if (this->write_header() < 0) {
		this->close();
		return -1;
	}

	return 0;
}

void PNMWriter::close()
{
	if (this->fp) {
		if (fprintf(this->fp, "\n") < 0) {
			WRN(errno, "file write error.");
		}
		if (this->path) {
			fclose(this->fp);
		}
	}
	this->size.set(0, 0);
	this->fp = NULL;
	this->path = NULL;
}


/**
 * 画像データ出力
 * @param[in] data 画像データ
 * @param[in] data_size 画像データサイズ
 * @return 処理結果
 * @retval >=0 処理成功
 * @retval -1 処理失敗
 */
int PNMWriter::write(uint8_t* data, size_t data_size)
{
	TRC("data_size=%d", data_size);
	int err;
	size_t rc = fwrite(data, data_size, 1, this->fp);
	if ((err = ferror(this->fp)) != 0) {
		WRN(0, "file write error. (err=%d)", err);
		clearerr(this->fp);
		return -1;
	}
	return 0;
}



int PBMWriter::write_header()
{
	if (fprintf(this->fp, "P4\n%d %d\n", this->size.x, this->size.y) < 0) {
		return -1;
	}
	return 0;
}



int PGMWriter::write_header()
{
	if (fprintf(this->fp, "P5\n%d %d\n255\n", this->size.x, this->size.y) < 0) {
		return -1;
	}
	return 0;
}



int PPMWriter::write_header()
{
	if (fprintf(this->fp, "P6\n%d %d\n255\n", this->size.x, this->size.y) < 0) {
		return -1;
	}
	return 0;
}





}; /* namespace liboscnr */

