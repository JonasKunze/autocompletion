//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

#include <iostream>
#include <string>

#include "Node.h"

using namespace std;

struct test {
	char chars[];
	char chars2[];
};

int main() {
	char chars[] = { 'a' };
	PackedNode *n = createNode(sizeof(chars), chars, false, 1024, 1024);
	cout << n->getFirstChildOffset() << endl;
	return 0;
}
