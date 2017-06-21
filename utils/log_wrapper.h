#ifndef _LOG_WRAPPER_
#define _LOG_WRAPPER_
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>

enum log_state {
	fatal = 0,
	error = 1,
	warn = 2,
	info = 3,
	total = 4,
};
static const char * log_header[total]=
{
	"FATAL", "ERROR", "WARNING", "INFO"
};

class LOG_TRIVIAL {
	enum log_state level;//枚举类型变量
public:
	std::stringstream state;
	LOG_TRIVIAL(enum log_state level_in)//枚举类型变量初始化
		:level(level_in){}
	
	template<typename T>
	LOG_TRIVIAL & operator<< (T value){//<<重载
		state<< value;
		return *this;
	}
	const std::string get_current_time(){
		auto now = std::chrono::system_clock::now();//当前时间time_point
		std::time_t t = std::chrono::system_clock::to_time_t(now);//time_point转换成time_t秒
		std::string ts = std::ctime(&t);//ctime把日期和时间转换为字符串
		ts.resize(ts.size()-1);//ctime最后的以空格结束
		return ts;
	}	
	~LOG_TRIVIAL(){
		std::stringstream final;
		final << get_current_time();
		final << "<" <<log_header[level] <<"> ";
		final << state.str() <<std::endl;
		#ifdef LOG
		std::ofstream log_file("./dx.log", std::ios::out | std::ios::app );//以写文件模式打开，每次写文件均定位到文件末尾
		log_file<<final.str();
		#endif
		std::cerr <<final.str();
	}
	

};
#endif
