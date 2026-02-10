#include <cuda_runtime.h>
#include <iostream>
#include <stdexcept>
#include <string>

#pragma once

#define CUDA_CHECK(val) { cuda_check_error((val), __FILE__, #val, __LINE__); }

void cuda_check_error(cudaError_t error_code, const char* const file, const char* const function, const int line)
{
	if (error_code == cudaSuccess) return;

	std::string msg = std::string("CUDA ERROR at " ) + file + ":" + std::to_string(line) + ":" + function + ":\n" + cudaGetErrorString(error_code) + "\n";

	std::cerr << msg;
	throw std::runtime_error(msg);
}