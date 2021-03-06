#ifndef _PAGERANK_H_
#define _PAGERANK_H_
#include "utils/buffer.h"
#include "utils/config.h"
#include "utils/log_wrapper.h"
#include "utils/types.h"
#include "utils/type_utils.h"
#include "core/engine.h"
#include "core/update.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iomanip>

class pagerank: public engine<update_weight_double_t >{
private:
	ecgraph::weight_t init_array;
	ecgraph::weight_t init_weight;           //初始权值

	//收敛标记
	std::vector<bool> *update_bitset;
	
	//辅助数组，用来存放临时的结果
    std::vector<ecgraph::weight_t > *aux_array; //auxiliary array, 辅助数组

	ecgraph::weight_t temp;

	//用来编辑收敛的结点个数
	long long m_updated_num;
public:
	void init(){
		init_weight = 0.15 / get_gobal_graph_vertices_num();
		//init_array.degree = 0;
    	//init_array.res = init_weight;
    	init_array = 0;

		update_bitset = new std::vector<bool>(get_graph_vertices_num(), false);//每个分区结点数据

    	aux_array = new std::vector<ecgraph::weight_t>(get_graph_vertices_num(), init_array); //auxiliary array

		//初始化result数组
		for (auto &item : result) {//更新值vector
			item = init_weight;
		}
		//初始化degree数组
		for (auto& item : degree) {
			item = 0;
		}

		m_updated_num = 0;
		//show_graph_info();
	}

	void clear(){
		//std::cout <<"update_bitset"<< std::hex << (long long)update_bitset<<std::endl;
		//std::cout << "aux_array" << std::hex << (long long)aux_array<<std::endl;
		delete update_bitset;
		delete aux_array;
	}
	void scatter() {
		//init_read();
		LOG_TRIVIAL(info) << "worker("
			<< get_rank() <<") scatter ...";
		ecgraph::edge_t edge;
		update_weight_double_t update;

		if (super_step() == 0){//get every vertex's degree
			#ifdef MY_DEBUG
			LOG_TRIVIAL(info) << "worker("
				<< get_rank() << "scatter super step: "<<super_step();
			#endif

			//统计度
			int count = 0;
			int rank = get_rank();
			while( get_next_edge(edge) ){
				count++;
				
				degree[get_local_graph_vertices_offset(edge.src)] += 1;			
				/*if (count >= 5500000 && rank != 1 &&rank != 2 ) {
					LOG_TRIVIAL(info) << "compute(" 
						<< get_rank() << ") count "<<count;
				}*/
			}
			/*LOG_TRIVIAL(info) << "worker("
				<< get_rank() << ") degree "<<count;
			for (auto iter = degree.begin(); iter != degree.end();iter ++) {
				LOG_TRIVIAL(info) << "worker("
					<< get_rank() << ") degree " 
					<< iter - degree.begin()<<" "
					<< *iter;
			}*/

		}
		else{
			#ifdef MY_DEBUG
			LOG_TRIVIAL(info) << "worker("
				<< get_rank() << ") scatter super step: " << super_step();
			#endif

			long long local_address;
			//int count;
			while( get_next_edge(edge) ){
				//if ((*update_bitset)[edge.dst ] == false){//未收敛
				/*count++;
				if (count == 100) {
					LOG_TRIVIAL(info) <<"count "<< count;
				}*/
					local_address = get_local_graph_vertices_offset(edge.src);
					update.id = edge.dst;
					if (degree[local_address] == 0) {
						LOG_TRIVIAL(warn) << "worker("<<get_rank()<<") index "
										<<local_address <<" degree 0";
						exit(0);
					}
					update.update_value =result[local_address]/ degree[local_address];
					/*LOG_TRIVIAL(info) << "worker(" << get_rank() << ") update.id " << update.id
						<< " update.update_value " << update.update_value
						<< " result "<< typeid(result[local_address]).name();*/
					add_update(update, false);
				//}
			}
		}
	}

	bool gather(){
		//LOG_TRIVIAL(info)<<"gather ...";
		update_weight_double_t update;
		ecgraph::weight_t temp;
		if (super_step() != 0){
			//int pos = 0;
			#ifdef MY_DEBUG
			LOG_TRIVIAL(info) << "gather super step: " << super_step();
			#endif
			long long local_address;
			while (get_update(update)){
				local_address = get_local_graph_vertices_offset(update.id);
				//if ((*update_bitset)[local_address] == false){
					//update the new array
					(*aux_array)[local_address] += update.update_value ;
				//}
					//测试用
					//LOG_TRIVIAL(info) << "worker(" << get_rank() << ") update.id " << update.id
					//	<< " update.update_value " << update.update_value;
			}
			for (auto iter = aux_array->begin(); iter != aux_array->end(); iter++){
				local_address = iter - aux_array->begin();
				
				//已经收敛，不处理，继续
				if ((*update_bitset)[local_address] == true) {
					continue;
				}

				temp = init_weight + 0.85 * (*iter);
				if (fabs(result[local_address] - temp ) < 0.00000000001){
					(*update_bitset)[local_address] = true;
					m_updated_num ++ ;	
				}	
				else{
					result[iter - aux_array->begin()] = temp;
				}
				*iter =0.0;
			}
			LOG_TRIVIAL(info)<<"gather "<< get_gobal_graph_vertices_num() - m_updated_num
				<<" / "<< get_gobal_graph_vertices_num();
        	if ( m_updated_num == get_gobal_graph_vertices_num()){ //all bits are 1
            	LOG_TRIVIAL(info) << "convergence and exit";
				set_convergence();
            	return true;
        	}
			return false;
		}
		return false;
	}
	void output(){
    	std::ofstream out("pagerank_output_"+std::to_string(get_rank())+".csv", std::ios::out);
    	auto begin = result.begin();
		LOG_TRIVIAL(info) << "worker(" << get_rank() << ") in output";
    	for (auto iter = begin; iter != result.end(); iter++){
        	out << get_gobal_graph_vid(iter -begin)<<" "
            	<<std::fixed<<std::setprecision(16)
            	<<*iter<<std::endl;
    	}
		
	}       
};

/*int main(int argc, char * argv[]){
	 if(argc != 3){
        std::cout<<"Usage: execute_file filename iterator_times "
                    << std::endl;
        return 0;
    }
	std::stringstream record;
	std::string filename;
	int niter;

	record.clear();
	record<< argv[1];
	record>> filename;

	record.clear();
	record<< argv[2];
	record>> niter;

	pagerank pr(filename, niter);
	pr.run();
	return 0;
}
*/
#endif