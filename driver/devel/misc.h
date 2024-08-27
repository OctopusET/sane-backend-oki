/*
 * misc.h
 *
 *  Created on: 2012/12/06
 *      Author: tanimoto
 */

#ifndef MISC_H_
#define MISC_H_

#include "libstatus.h"

namespace odc {

const char* strlibstatus(LibStatus status);


template <class T> void swap(T* a, T* b) {T c = *a; *a = *b; *b = c;}

int mm2pixel(int dpi, int mm);
int mm2pixel(double dpi, double mm);


}; 


#endif /* MISC_H_ */
