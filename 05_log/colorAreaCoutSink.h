
#include <iostream>
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/sinks/ansicolor_sink.h"

using namespace spdlog::sinks;

template <typename ConsoleMutex>
class ColorAreaCoutSink : public ansicolor_stdout_sink<ConsoleMutex> {
public:
    explicit ColorAreaCoutSink(int line = 1, int column = 1, spdlog::color_mode mode = spdlog::color_mode::automatic) 
        : ansicolor_stdout_sink<ConsoleMutex>(mode), line_(line), column_(column) {}
    
    void set_pattern(const std::string& pattern) override {
        // 简单实现：忽略模式，只输出原始消息
    }
    
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override {
        formatter_ = std::move(sink_formatter);
    }
    
    void log(const spdlog::details::log_msg& msg) override {
        // 保存当前光标位置
        std::cout << "\033[s";
        
        // 移动到目标位置
        std::cout << "\033[" << line_ << ";" << column_ << "H";
        
        // 清除行内容
        std::cout << "\033[2K";
        
        // 更新line和column
        line_ += std::count(msg.payload.begin(), msg.payload.end(), '\n');
        column_ += last_line(msg.payload).size();
        line_++;
        
        // 输出消息
        if (formatter_) {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);
            std::cout.write(formatted.data(), formatted.size());
        } else {
            std::cout.write(msg.payload.data(), msg.payload.size());
        }
        
        // 恢复光标位置
        std::cout << "\033[u";
        std::cout.flush();
    }
    
    void flush() override {
        std::cout.flush();
    }

	void clear() {
        // 清除整个屏幕并将光标移到左上角
        std::cout << "\033[2J\033[1;1H" << std::flush; 
    }
private:
	std::string last_line(const spdlog::string_view_t& buf) {
		//size_t newline_pos = std::find(buf.rbegin(), buf.rend(), '\n').base() - buf.begin();
		//return std::string(buf.begin() + newline_pos, buf.end() );
		return "";
	}
	#if 0
	std::string last_line(const spdlog::string_view_t& buf) {
        // 查找最后一个换行符
        size_t pos = buf.find_last_of('\n');
        if (pos == string_view_t::npos) {
            return std::string(buf.data(), buf.size());
        }
        return std::string(buf.data() + pos + 1, buf.size() - pos - 1);
    }
    #endif
    
private:
    int line_;
    int column_;
    std::unique_ptr<spdlog::formatter> formatter_;
};

// 定义具体类型别名
using ColorAreaCoutSink_mt = ColorAreaCoutSink<spdlog::details::console_mutex>;
