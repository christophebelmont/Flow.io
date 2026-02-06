/**
 * @file RunningMedianFilter.cpp
 * @brief Implementation file.
 */
#include "RunningMedianFilter.h"
#include <stdlib.h>

RunningMedianFilter::RunningMedianFilter(size_t windowSize) : window_(windowSize) {
    if (window_ == 0) return;
    buffer_ = (float*)malloc(sizeof(float) * window_);
    scratch_ = (float*)malloc(sizeof(float) * window_);
    for (size_t i = 0; i < window_; ++i) buffer_[i] = 0.0f;
}

RunningMedianFilter::~RunningMedianFilter() {
    if (buffer_) free(buffer_);
    if (scratch_) free(scratch_);
}

static void insertionSort(float* arr, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        float key = arr[i];
        int j = (int)i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

SensorReading RunningMedianFilter::apply(const SensorReading& in) {
    if (!in.valid || window_ == 0 || !buffer_ || !scratch_) return in;

    buffer_[index_] = in.value;
    index_ = (index_ + 1) % window_;
    if (count_ < window_) count_++;

    for (size_t i = 0; i < count_; ++i) scratch_[i] = buffer_[i];
    insertionSort(scratch_, count_);

    SensorReading out = in;
    size_t mid = count_ / 2;
    if (count_ % 2 == 0 && count_ > 1) {
        out.value = (scratch_[mid - 1] + scratch_[mid]) * 0.5f;
    } else {
        out.value = scratch_[mid];
    }
    return out;
}
