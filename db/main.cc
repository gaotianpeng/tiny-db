#include <iostream>
#include "tinydb/options.h"

using namespace std;
using namespace tinydb;

int main(int argc, char* argv[]) {
    cout << "hello tinydb" << endl;
    cout << CompressionType::kNoCompression << endl;
    Options opt;
    cout << opt.max_file_size << endl;
    return 0;
}