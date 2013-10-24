/*
 * Utils.h
 *
 *  Created on: Oct 24, 2013
 *      Author: Jonas Kunze
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/time.h>

long getCurrentMicroSeconds() {
	struct timeval curr;
	struct timezone tz;
	gettimeofday(&curr, &tz);
	return curr.tv_sec * 1000000l + curr.tv_usec;
}

#endif /* UTILS_H_ */
