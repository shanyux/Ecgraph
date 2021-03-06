﻿#ifndef _CONSISTENT_HASH_
#define _CONSISTENT_HASH_
//#include <bits/stdc++.h>
#include <map>
#include <vector>
#include <climits>
#include <cstdint>
#include <cassert>
#include <utility>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>
#include "graph_data.h"
#include "types.h"

using namespace boost::property_tree;
using namespace boost::gregorian;
using namespace boost;

//能快速查找一个key对应的机器node
//选取一个好的key hash函数
//
//只负责系统元数据的管理
//不负责系统数据的迁移

//实际系统中用到
#define MIN_VERTIECS_PER_NODE 10000

//测试用
//#define MIN_VERTIECS_PER_NODE 1

namespace ecgraph{

    class consistent_hash{

    private:
        //node_to_ring存放计算节点在ring环中的值到机器节点的映射。
        std::map<vertex_t, vertex_t> m_node_to_ring;//<vid,workid>

        //uint32_t worker_count;
        
        //m_noeds 存放计算节点列表， 以vertex_t表示一个计算节点。
        //std::vector<vertex_t> m_nodes;

        //图数据的元数据
        graph_data *m_graph_data;

        //每个计算节点所拥有的最小图结点数量
        graph_data::graph_size_t m_min_vertices_per_node;

    public:
        consistent_hash(std::vector<vertex_t> &machine_nodes, //一个vector一个图数据
                        graph_data *gd,
                        graph_data::graph_size_t min = MIN_VERTIECS_PER_NODE){
            
            //m_nodes = machine_nodes;
            m_graph_data = gd;
            m_min_vertices_per_node = min;
			init(machine_nodes);
        }


		consistent_hash(std::string json_filename, graph_data *gd ) {
			m_graph_data = gd;
			std::ifstream json_file(json_filename);
			assert(json_file);
			load_from_json(json_file);
		}
		consistent_hash() {
			m_graph_data = NULL;
			m_min_vertices_per_node = MIN_VERTIECS_PER_NODE;
		}

		void init(std::vector<vertex_t> &machine_nodes){
			if (!machine_nodes.empty()) {
				graph_data::graph_size_t each =
					m_graph_data->get_vertices_count() / machine_nodes.size();

				each = each > m_min_vertices_per_node ? each : m_min_vertices_per_node;

				//将计算节点映射到ring环中
				//node_to_ring 的key是一个ring环中的一个值, 
				//value保存的是计算节点的rank值
				for (vertex_t i = 1; i <= machine_nodes.size(); i++) {
					//if i*each is bigger than UINT_MAX, it is OK
					vertex_t ring_id = i*each - 1;
					
					if (ring_id < m_graph_data->get_vertices_count()) {
						m_node_to_ring[ring_id] = machine_nodes[i - 1];//m_node_to_ring只记录界限

					}
					if (ring_id>= m_graph_data->get_vertices_count()) {
						m_node_to_ring[m_graph_data->get_vertices_count() - 1] = machine_nodes[i - 1];
						break;
					}
					
				}
			}
		}

        vertex_t vertex_to_ring_value(vertex_t vertex_id){
        //this is the function that map the vertex to the ring
        //we just return itself, because it can reduce the computation
            return vertex_id;
        }


        //插入一个worker到对应的ring环id上去
        void split(std::pair<vertex_t, vertex_t> vertex_node_pair){
        //the worker node_id is too busy, so it should be splited
            
            //add the worker
            //TODO
			m_node_to_ring.insert(vertex_node_pair);//Map中的元素是自动按key升序排序

        }


        vertex_t operator()(vertex_t vertex_id){//返回工作节点
        //return node_id
			if (m_node_to_ring.empty()) {
				LOG_TRIVIAL(error) << "find worker error, this function should be used after initation";
				exit(0);
			}
            auto iter = m_node_to_ring.lower_bound(vertex_to_ring_value(vertex_id));//大于等于图结点的迭代器，函数lower_bound()在first和last中的前闭后开区间进行二分查找，返回大于或等于val的第一个元素位置
            if(iter!=m_node_to_ring.end()){
                return iter->second; //return the nearest worker
            }
            else{
                return m_node_to_ring.begin()->second;
            }
            
        }

		//获得一个计算node上的点的范围
		std::pair<vertex_t, vertex_t> get_vertices_on_node(vertex_t node_id) {
			if (m_node_to_ring.empty()) { //map <vid,workid>
				LOG_TRIVIAL(error) << "error happened, this function should be used after initation";
				exit(0);
			}

			vertex_t start = m_node_to_ring.rbegin()->first + 1;
			if (start >= m_graph_data->get_vertices_count()) {
				start = 0;
			}
			vertex_t end = m_node_to_ring.begin()->first;
			for (auto iter = m_node_to_ring.begin(); iter != m_node_to_ring.end(); iter++) {
				if (iter->second == node_id && iter != m_node_to_ring.begin()) {
					end = iter->first;
					iter--;
					start = iter->first + 1;
					break;
				}
			}
			return std::make_pair(start, end);
		}


		//从输入流中读取，cin, ifstream, istringstream
		void load_from_json(std::istream &in) {//最基本类
			ptree pt, node_to_ring;
			read_json(in, pt);
			try {
				node_to_ring = pt.get_child("hash_ring_info");
			}
			catch (boost::property_tree::ptree_bad_path) {
				LOG_TRIVIAL(error) << "no path hash_ring_info";
				return;
			}

			try {
				m_min_vertices_per_node = node_to_ring.get<int>("min_vertices_per_node");
			}
			catch(boost::property_tree::ptree_bad_path){
				LOG_TRIVIAL(error) << "no path min_vertices_per_node";
				return;
			}

			m_node_to_ring.clear();//m_node_to_ring是一个map，清除Map集合的所有内容

			std::stringstream type_convert;
			vertex_t ring_id, node_id;
			for (auto item : node_to_ring.get_child("ring_to_node")) {
				type_convert.clear();
				type_convert << item.first;//key是string
				type_convert >> ring_id;//简单类型转换

				type_convert.clear();//value的类型是tree需要转换string
				type_convert << item.second.data();//.c_str()   是生成以‘\0’结束的字符串；data()    生成的字符串没有‘\0’；
				type_convert >> node_id;

				m_node_to_ring[ring_id] = node_id;
			}
		}


		void load_from_jsonfile(std::string json_filename) {
			std::ifstream json_file(json_filename);
			if (!json_file) {
				LOG_TRIVIAL(error) << "the file" << json_filename << "can not be opened";
				return;
			}
			load_from_json(json_file);

		}


		void load_from_json(std::string json_string) {
			ptree node_to_ring, pt;
			std::istringstream ss(json_string);
			load_from_json(ss);
		}


		std::string save() {
			ptree root, pt, pt_child1;
			std::stringstream ss;

			std::stringstream type_convert;
			std::string _key;

			for (auto &item : m_node_to_ring) {
				type_convert.clear();
				type_convert << item.first;////
				type_convert >> _key;

				pt_child1.put(_key, item.second);
			}
			pt.add_child("ring_to_node", pt_child1);
			pt.put("min_vertices_per_node", m_min_vertices_per_node);

			root.add_child("hash_ring_info", pt);



			write_json(ss, root);
			return ss.str();//str()成员函数的使用可以让istringstream对象返回一个string字符串
		}
		void dump(std::string filename) {
			std::ofstream dump_file(filename);
			if (!dump_file) {
				LOG_TRIVIAL(info) << "The file can not be opened";	
				return;
			}
			dump_file << save();
		}

		//计算集群大小，不含控制节点
		int worker_size() {
			return m_node_to_ring.size();
		}


		void get_workers(std::vector<vertex_t> &machines) {
			machines.clear();
			for (auto & item : m_node_to_ring) {
				machines.push_back(item.second);
			}
		}

		//获取图信息, 返回字符串形式
		std::string get_graph_info() {
			if (m_graph_data == NULL) { return ""; }
			return m_graph_data->save();
		}

		//获取图信息指针
		ecgraph::graph_data *get_graphdata_ptr() {
			return m_graph_data;
		}
    };

}

#endif
