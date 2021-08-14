#ifndef LOGGER_H
#define LOGGER_H
#pragma once

#define LOG_SIZE 400

void log(String msg);
void dump_log(byte* buffer_pointer, size_t buffer_size);
void init_logger();

#endif //LOGGER_H
