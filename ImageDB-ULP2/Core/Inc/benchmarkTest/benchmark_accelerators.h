/*
 * benchmark_accelerators.h
 *
 *  Created on: Sep 2, 2024
 *      Author: luisferreira
 */

#ifndef INC_BENCHMARK_ACCELERATORS_H_
#define INC_BENCHMARK_ACCELERATORS_H_

#include "benchmark_from_uart.h"

void benchmark_YCbCr_encode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *batchSizes, uint32_t numBatchSizes, uint32_t numIterations);

/*
 * @brief Benchmark the performance in pixels/second of JPEG accelerator encoding using DMA to feed it data.
 *
 */

void benchmark_performance_JPEG_accelerator_DMA(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization);

/*
 * @brief Benchmark the performance in pixels/second of JPEG accelerator decoding using DMA to feed it data.
 *
 */
void benchmark_performance_JPEG_accelerator_DMA_decode(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations);

/*
 * @brief Benchmark the performance in pixels/second of JPEG accelerator encoding using CPU to feed data, with interrupts
 *
 */
void benchmark_performance_JPEG_accelerator_IT(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization);

/*
 * @brief Benchmark the performance in pixels/second of JPEG accelerator decoding using CPU to feed it data, with interrupts.
 *
 */
void benchmark_performance_JPEG_accelerator_IT_decode(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations);

/*
 * @brief Benchmark the performance in pixels/second of JPEG encoding using CPU only. No accelerator.
 *
 */
void benchmark_performance_JPEG_encode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations, uint8_t timeQuantization);

/*
 * @brief Benchmark the performance in pixels/second of JPEG decoding using CPU only. No accelerator.
 *
 */
void benchmark_performance_JPEG_decode_CPU(uint32_t *imageHeights, uint32_t * imageWidths, uint32_t *numRepeatImage, uint32_t numSizes, uint32_t *quality, uint32_t numQuality, uint32_t numIterations);

/*
 * @brief
 */
void benchmark_raw_JPEG_accelerator_encode_performance_dataset(BenchmarkSettings *settings);


#endif /* INC_BENCHMARK_ACCELERATORS_H_ */
