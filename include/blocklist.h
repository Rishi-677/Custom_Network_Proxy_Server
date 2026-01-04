#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include <string>
#include <set>

using namespace std;

// Load blocked sites from file
bool load_blocklist(const string &filename);

// Check if a host is blocked
bool is_blocked(const string &host);

#endif
