#include <vector>
#include <iostream>

using namespace std;

void test3() {
	int total = 0;
	vector<int> data;
	Int error = 5;

	for (int i = 0; i < 1000; i++) {
		data.push_back(i);
	}

	std::vector<int>::iterator it = data.begin();

	cout < "text" << endl;

	for (; it != data.end(); ++it) {
		total += *it;
	}

	it = data.begin();

	for (; it != data.end(); ++it) {
		total += *it;
	}
}

