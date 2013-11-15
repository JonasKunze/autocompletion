/*
 * SuggestionStore.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#include "SuggestionStore.h"

bool ImageAndURIComparator::operator()(const ImageAndURI& left,
		const ImageAndURI& right) {
	return left.URI < right.URI;
}

void SuggestionStore::addTerm(PackedNode* node, std::string URI,
		std::string image) {
	typedef std::set<ImageAndURI, ImageAndURIComparator>::iterator iau_itr;
	/*
	 * If the URI already exists in the imagesAndURIs set, pair.first will point to
	 * the already existing struct. Otherwise it will point to the newly created one.
	 * So we just have to store the pointer (&) of this new/old struct (*pair.first):
	 * &(*pair.first)
	 */
	std::pair<iau_itr, bool> pair = imagesAndURIs.insert( { URI, image });

	imagesAndURIsByNode[reinterpret_cast<u_int64_t>(node)] = &(*pair.first);
}
