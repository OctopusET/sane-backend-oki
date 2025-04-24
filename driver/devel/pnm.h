/*
 * pnm.h
 * PNM image file writer.
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef PNM_H_
#define PNM_H_
#include <sys/types.h>
#include <stdint.h>

#include "boundary.h"

namespace odc {

struct PNMWriter
{
protected:
	odc::XY<int> size;	
	const char* path;	
	FILE* fp;	

	virtual int write_header() = 0;
public:
	PNMWriter();
	virtual ~PNMWriter();

	int open(const char* path, int w, int h);
	int open(FILE* fp, int w, int h);
	void close();
	virtual int write(uint8_t* data, size_t data_size);
};

struct PBMWriter : public PNMWriter
{
protected:
	virtual int write_header();
public:
	PBMWriter() {}
};

struct PGMWriter : public PNMWriter
{
protected:
	virtual int write_header();
public:
	PGMWriter() {}
};

struct PPMWriter : public PNMWriter
{
protected:
	virtual int write_header();
public:
	PPMWriter() {}
};

}; 

#endif /* PNM_H_ */
