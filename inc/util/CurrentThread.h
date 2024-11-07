#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include "Platform.h"

namespace inet
{
namespace CurrentThread
{
    extern thread_local int t_cachedTid;
    extern thread_local char t_tidString[32];
    extern thread_local int t_tidStringLength;

    inline pid_t gettid();
    void cacheTid();

    inline int tid() 
    {
        if (__builtin_expect(t_cachedTid ==0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }

	inline const char* tidString() // for logging
	{
		return t_tidString;
	}

	inline int tidStringLength() // for logging
	{
		return t_tidStringLength;
	}
}
}

#endif