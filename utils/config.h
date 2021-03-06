#ifndef _CONFIG_
#define _CONFIG_
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>
#include "log_wrapper.h"
using namespace boost::property_tree;
using namespace boost::gregorian;
using namespace boost;

namespace ecgraph {
	class config {
	public:
		typedef std::map<std::string, std::string> _KV;//vector  type:1

	private:
		std::string m_config_filename;
		_KV m_conf;//配置文件，json的内容
	public:
		config() {
			//do nothing
			m_config_filename = "";
		}
		explicit config(std::string config_fname) {
			assert(config_fname != "");//断言空就结束
			if (!load(config_fname)) {
				exit(0);
			}
		}

		bool load(std::string config_fname) {
			m_config_filename = config_fname;

			std::ifstream config_file(m_config_filename);
			if (!config_file) {
				LOG_TRIVIAL(error) <<m_config_filename <<" can not be opened for read";
				return false;
			}
			load(config_file);
			return true;
		}
		bool load(std::istream &in) {//ifstream是从istream继承而来的
			ptree pt;
			read_json(in, pt);

			try {
				for (auto item : pt.get_child("graph_info")) {
					m_conf[item.first] = item.second.data();//map  生成一个const char*指针，指向字符数组（不必以空字符结束）。
				}
			}
			catch (boost::property_tree::ptree_bad_path) {
				LOG_TRIVIAL(error) << "there is no path graph_info";
				return false;
			}
			return true;
		}

		bool load_from_string(std::string config_string) {
			std::stringstream ss;
			ss << config_string;
			load(ss);
			return true;
		}

		//写到输出流
		void save(std::ostream &out) {
			ptree root, pt;
			for (auto item : m_conf) {
				pt.put(item.first, item.second);
			}
			root.add_child("graph_info", pt);
			write_json(out, root);

		}

		//保存到字符串
		std::string save() {
			std::ostringstream ss;
			save(ss);
			return ss.str();//streamstring在调用str()时，会返回临时的string对象
		}

		void dump(){
			if (m_config_filename == "") {
				LOG_TRIVIAL(warn) << "save to file failed, because the filename is empty";
				return;
			}
			std::ofstream write_config_file(m_config_filename);
			if (!write_config_file) {
				LOG_TRIVIAL(error) << "config file can not be opened for write";
				return;
			}
			save(write_config_file);
		}
		void dump(std::string config_filename) {
			std::ofstream write_config_file(config_filename);
			if (!write_config_file) {
				LOG_TRIVIAL(error) << "config file can not be opened for write";
				return;
			}
			save(write_config_file);
		}

        //~config(){
        //}
		void show(){
			for (auto item : m_conf) {
				std::cout << item.first << ": " << item.second << std::endl;
			}
		}
		std::string &operator[](std::string key){
			return m_conf[key];
		}

		bool check() {
			const int item_num = 9;
			std::vector<std::string> vec(9);
			vec[0] = "partition_id";
			vec[1] = "edges";
			vec[2] = "name";
			vec[3] = "partition_end_vid";
			vec[4] = "partition_start_vid";
			vec[5] = "type";
			vec[6] = "vertices";
			vec[7] = "partition_edges_num";
			vec[8] = "partition_mid_vid";

			for (size_t i = 0; i < vec.size(); i++) {//？？？？？头文件cstddef
				if (m_conf.find(vec[i]) == m_conf.end()) {
					LOG_TRIVIAL(error) << "in config file check, "
						<< vec[i]
						<< " is not found!";
					return false;
				}
			}
			return true;
		}
	};
}
#endif
