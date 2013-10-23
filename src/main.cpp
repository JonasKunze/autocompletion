//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

#include <iostream>

#include "Node.h"

using namespace std;

int main() {
	char chars[] = { 'a', 'b' };
	PackedNode<sizeof(chars), 2, 3> n(chars, false, 1, 2);
	cout << sizeof(n) << endl;
	return 0;
}
