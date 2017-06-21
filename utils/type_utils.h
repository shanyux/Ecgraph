#ifndef _TYPE_UTILS_
#define _TYPE_UTILS_
#include <iostream>
#include <string>
#include <cstring>
#include "types.h"
namespace ecgraph{
		class format_utils {
		public:
			static unsigned long split_size_bytes(){
				//return 2*sizeof(vertex_t) + sizeof(weight_t);
				return sizeof(edge_t);
			}
			static void read_edge(const char * buf,
									vertex_t & src,
									vertex_t & dst,
									weight_t & value)
			{
				//src = *(vertex_t *) buffer;
				//dst = *(vertex_t *) (buffer + sizeof(vertex_t));
				//value = *(weight_t *)(buffer + 2*sizeof(vertex_t) );
				edge_t * edge;
				edge = (edge_t *)buf;
				src = edge->src;
				dst = edge->dst;
				#ifdef WEIGHT_EDGE
				value = edge->value;
				#endif
			}
			static void read_edge(const char * buf,
									edge_t & edge)
			{
				//src = *(vertex_t *) buffer;
				//dst = *(vertex_t *) (buffer + sizeof(vertex_t));
				//value = *(weight_t *)(buffer + 2*sizeof(vertex_t) );
				edge = *(edge_t *)buf;
			}
			static void write_edge( char *buffer,
									vertex_t& src,
									vertex_t& dst,
									weight_t& value)
			{
				*(vertex_t *)buffer = src;
				*(vertex_t *)(buffer + sizeof(vertex_t)) = dst;
				#ifdef WEIGHT_EDGE
				*(weight_t *)(buffer + 2*sizeof(vertex_t))= value;
				#endif
			}
			
			static void write_edge(char *buffer,
									edge_t & edge)
			{
				//*(vertex_t *)buffer = src;
				//*(vertex_t *)(buffer + sizeof(vertex_t)) = dst;
				//*(weight_t *)(buffer + 2*sizeof(vertex_t))= value;
				memcpy(buffer, &edge, sizeof(edge_t));
			}
			
	};
}
#endif
