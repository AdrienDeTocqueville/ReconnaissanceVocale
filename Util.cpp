#include "Util.h"
#include <cstdlib>


int toInt(string _string)
{
    stringstream os(_string);
    int value;
    os >> value;

    return value;
}

double toDouble(string _string)
{
    stringstream os(_string);
    double value;
    os >> value;

    return value;
}

string toString(int _number)
{
    stringstream os;
    os << _number;

    return os.str();
}

string toStringd(double _number)
{
    stringstream os;
    os << _number;

    return os.str();
}

vector<string> ssplit(const string& _string, char _separation)
{
    string buf;
    istringstream ss(_string);
    vector<string> tokens;

    while (getline(ss, buf, _separation))
    {
        tokens.push_back(buf);
    }

    return tokens;
}

string askFile(string _default, string _description)
{
    cin.ignore();

    string input;

    cout << endl << "Fichier par default: " << _default << endl;
    cout << _description << ": ";   getline (cin, input);

    if (input.empty())  return _default;

    return input;
}

string askFile(string _default, string _description, string _extension)
{
    _default = removeExtension(_default) + _extension;
    return removeExtension(askFile(_default, _description)) + _extension;
}

string getExtension(string file)
{
    size_t pos = file.rfind(".");
    if (pos == std::string::npos)
        return "";

    return file.substr(pos, file.size()-1);
}

string removeExtension(string file)
{
    size_t pos = file.rfind(".");
    if (pos == std::string::npos)
        return file;

    return file.substr(0, pos);
}

string setExtension(string file, string _extension)
{
    return removeExtension(file) + _extension;
}
