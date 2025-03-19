/*
 * test_xor.h
 *
 *  Created on: Feb 25, 2025
 *      Author: luisferreira
 */

#ifndef INC_BENCHMARKTEST_TEST_XOR_H_
#define INC_BENCHMARKTEST_TEST_XOR_H_

double test_xor_performance_dataset(double * data, uint32_t numValues, uint32_t numDataRepeats, double *stddev);
uint32_t test_xor_compressed_size_dataset(double * data, uint32_t numValues);
uint32_t test_xor_error_dataset(double * data, uint32_t numValues);

#endif /* INC_BENCHMARKTEST_TEST_XOR_H_ */
