#ifndef _TYPES_
#define _TYPES_
#include <climits>
namespace ecgraph{
	typedef char byte_t;
	#ifdef COMPACT_GRAPH
	typedef unsigned int vertex_t;
	typedef double weight_t;
	const vertex_t VERTEX_MIN = 0;
	const vertex_t VERTEX_MAX = UINT_MAX;
	#else
	typedef unsigned long vertex_t;
	typedef double weight_t;
	const vertex_t VERTEX_MIN = 0;
	const vertex_t VERTEX_MAX = ULONG_MAX;
	#endif
	
	#ifdef WEIGHT_EDGE	

	#pragma pack(1)//内存对齐
	typedef struct {
		vertex_t src;
		vertex_t dst;
		weight_t value;
	}edge_t;
	#pragma pack()

	#else
	#pragma pack(1)
	typedef struct {
		vertex_t src;
		vertex_t dst;
	}edge_t;
	#pragma pack()
	#endif
}
#endif
