#include <thread>
#include <memory>
#include <assert.h>
#include "AsyncLogger.h"
#include "CurrentThread.h"
#include "Logger.h"

using namespace inet;

static std::unique_ptr<AsyncLogger> asyncLogger;
static std::once_flag g_once_flag;

thread_local char t_time[64]; //当前线程的时间戳字符串的日期和时间
thread_local time_t t_lastSecond; //当前线程的最新的日志消息的秒数


Logger::LogLevel InitLogLevel()
{
    if (getenv("LGG_DEBUF"))
        return Logger::LogLevel::DEBUG;
    else    
        return Logger::LogLevel::INFO;
}

// static value log level
Logger::LogLevel g_LogLevel = InitLogLevel();

Logger::LogLevel Logger::getGlobalLogLevel()
{
    return g_LogLevel;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_LogLevel = level;
}

// 默认输出函数，输出至标准输出
void DefaultOutput(const char *msg, int len)
{
    fwrite(msg, 1, len, stdout);
}

// 默认冲刷函数，冲刷标准输出流
void DefaultFlush()
{
	fflush(stdout);
}

std::string Logger::m_log_file_basename = "./log";

void OnceInit()
{
	asyncLogger = std::make_unique<AsyncLogger>(Logger::getLogFileName(), 1024*1024*50);//rollsize=100MB
	asyncLogger->start();
}

void AsyncOutput(const char* logline, int len)
{
	std::call_once(g_once_flag, OnceInit);
	asyncLogger->append(logline, len);
}

// 全局变量：输出函数
Logger::OutputFunc g_output = AsyncOutput;

// 全局变量：冲刷函数
Logger::FlushFunc  g_flush = DefaultFlush;


// helper class for known string length at compile time(编译时间）
class T
{
public:
	T(const char* str, unsigned len)
		:str_(str),
		len_(len)
	{
	}

	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
	s.append(v.str_, v.len_);
	return s;
}

//日志等级字符串数组，用于输出的
const char* g_loglevel_name[static_cast<int>(Logger::LogLevel::NUM_LOG_LEVELS)] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

//多个级别，输出就有对应的输出。
Logger::Logger(const char* FileName, int line, LogLevel level, const char* funcName) :
    m_impl(level, FileName,line)
{
	m_impl.m_stream << funcName << ' ';
}

Logger::Logger(const char* file, int line) :
    m_impl(LogLevel::INFO, file, line)
{
}
Logger::Logger(const char* file, int line, LogLevel level) :
    m_impl(level,file,line)
{
}
Logger::Logger(const char* file, int line, bool toAbort) :
    m_impl(toAbort ? LogLevel::FATAL:LogLevel::ERROR, file, line)
{
}

Logger::~Logger()
{
	m_impl.finish();
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
}


Logger::Impl::Impl(LogLevel level, const std::string& file, int line) : 
    m_time(TimeStamp::now()),
    m_stream(),
	m_level(level),
	m_line(line),
	m_filename(file)
{
	formatTime();	//时间输出
	CurrentThread::tid();	//更新线程
	m_stream << T(CurrentThread::tidString(), CurrentThread::tidStringLength());	//输出线程id
	m_stream << T(g_loglevel_name[static_cast<int>(m_level)], 6);	//日志等级的字符串是定长的
}

void Logger::Impl::formatTime()
{
	int64_t microSecondsSinceEpoch = m_time.microSecondsSinceEpoch();
	//get s
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / TimeStamp::kMicroSecondsPerSecond);
	//get ms
	int microseconds = static_cast<time_t>(microSecondsSinceEpoch % TimeStamp::kMicroSecondsPerSecond);
	if (seconds != t_lastSecond) {	//秒数不相等，说明也要格式化秒数
		t_lastSecond = seconds;
		//struct tm tm_time;
		//gmtime_r(&seconds, &tm_time);//这是转换为格林尼治标准时间，和北京时间不一样
		struct tm tm_time;
		memset(&tm_time, 0, sizeof(tm_time));
		localtime_r(&seconds,&tm_time);

		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
	}
	char buf[12] = { 0 };
	int lenMicro =sprintf(buf, ".%06d ", microseconds);
	m_stream << T(t_time, 17) << T(buf, lenMicro);
}

void Logger::Impl::finish()
{
	m_stream << " - " << m_filename << ':' << m_line << '\n';
}

// 设置输出函数
void Logger::setOutput(OutputFunc out)
{
	g_output = out;
}
// 设置冲刷函数
void Logger::setFlush(FlushFunc flush)
{
	g_flush = flush;
}