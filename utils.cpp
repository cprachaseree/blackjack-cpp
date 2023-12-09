#include "utils.h"

using namespace std;


string remove_whitespace(string s)
{
    s.erase(remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

bool path_exists(string path)
{
    struct stat sb;
    return stat(path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR);
}

void print_vector(vector<string> v)
{
    LOG << "\n";
    for (string element : v)
    {
        cout << element << " ";
    }
    cout << endl;
}

vector<string> split_string(string &s, string delimiter)
{
    size_t last_pos = 0, next_pos = 0;
    vector<string> splitted_string;
    string token;
    while ((next_pos = s.find(delimiter, last_pos)) != string::npos)
    {
        token = s.substr(last_pos, next_pos - last_pos);
        splitted_string.push_back(token);
        last_pos = next_pos + 1;
    }
    return splitted_string;
}