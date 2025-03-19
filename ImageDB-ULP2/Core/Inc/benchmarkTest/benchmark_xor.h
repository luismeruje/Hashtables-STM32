/*
 * benchmark_xor.h
 *
 *  Created on: Feb 25, 2025
 *      Author: luisferreira
 */

#ifndef INC_BENCHMARKTEST_BENCHMARK_XOR_H_
#define INC_BENCHMARKTEST_BENCHMARK_XOR_H_
#include "benchmark_from_uart.h"

int benchmark_performance_encode_UART_xor(BenchmarkSettings *settings);
int benchmark_compressed_size_UART_xor(BenchmarkSettings *settings);
int benchmark_error_UART_xor(BenchmarkSettings *settings);

#endif /* INC_BENCHMARKTEST_BENCHMARK_XOR_H_ */
