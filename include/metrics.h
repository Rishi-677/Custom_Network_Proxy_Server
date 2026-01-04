#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <cstddef>

void init_metrics(const std::string &metrics_file);

void record_allowed(const std::string &host, size_t bytes);
void record_blocked();

void stop_metrics();

#endif
