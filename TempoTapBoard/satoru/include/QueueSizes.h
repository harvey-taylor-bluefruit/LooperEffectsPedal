#ifndef QUEUE_SIZES_H
#define QUEUE_SIZES_H

#include <stdint.h>

namespace QueueSizes
{
	static const uint32_t DefaultSize = 32u;
	//static const uint32_t Float = 64u;
}

// Type trait for queue sizes. Unless otherwise specified, they default to
// 32 elements, but it is possible to specify different queue sizes for different types.
template<typename T> struct QueueSize { static const uint32_t value = QueueSizes::DefaultSize; };

// Uncommenting the next line would, for example, cause "float" events have a queue size of QueueSizes::Float.
// template<> struct QueueSize<float> { static const uint32_t value = QueueSizes::Float };

#endif

