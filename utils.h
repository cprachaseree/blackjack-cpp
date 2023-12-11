#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

#define LOG cout << "[" << __FILE__ << ":" << __LINE__ << "] "

string remove_whitespace(string str);
bool path_exists(string path);
vector<string> split_string(string &s, string delimiter);
void print_vector(vector<string> v);