/*
 * Utils.h
 *
 *  Created on: Oct 24, 2013
 *      Author: Jonas Kunze
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/time.h>
const u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };

class Utils {
public:
	static long getCurrentMicroSeconds() {
		struct timeval curr;
		struct timezone tz;
		gettimeofday(&curr, &tz);
		return curr.tv_sec * 1000000l + curr.tv_usec;
	}

	/**
	 * Returns the pointer to the position of the first byte where str1 and str2 do not match.
	 * If all characters match 8 will be returned
	 *
	 * @param str1 A byte array with maximum length of 8 bytes
	 * @param str2 A byte array with maximum length of 8 bytes
	 */
	static inline u_int8_t findFirstNonMatchingCharacter(const char* str1,
			const char* str2) {
		u_int64_t l1 = *((u_int64_t*) (str1));
		u_int64_t l2 = *((u_int64_t*) (str2));

		u_int64_t mask = (l1 ^ l2);
		if (mask == 0) {
			return 8;
		}

		return ffsl(mask) / 8;
	}
};
#endif /* UTILS_H_ */
