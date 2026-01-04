#include "blocklist.h"
#include <fstream>
#include <iostream>

using namespace std;

// Internal container
static set<string> blocked_sites;

bool load_blocklist(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "[ERROR] Could not open blocklist file: "
             << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        if (!line.empty())
        {
            blocked_sites.insert(line);
        }
    }

    file.close();

    cout << "[INFO] Loaded " << blocked_sites.size()
         << " blocked sites" << endl;

    return true;
}

bool is_blocked(const string &host)
{
    for (const auto &rule : blocked_sites)
    {
        if (host == rule)
            return true;
    }
    for (const auto &rule : blocked_sites)
    {
        string suffix = "." + rule;
        if (host.size() > suffix.size() && host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0)
        {
            return true;
        }
    }
    return false;
}
