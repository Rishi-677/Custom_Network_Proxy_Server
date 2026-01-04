#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <cstddef>

void init_logger(const std::string &filename);
void log_event(const std::string &message);
void set_log_max_size(size_t bytes);

#endif
