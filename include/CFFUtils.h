/*
 * Copyright 2015 Guo He <lovenight@126.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __C_FFUTILS_H__
#define __C_FFUTILS_H__

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <pthread.h>
#include <stdio.h>
}

namespace android
{

typedef void (*FPLogCallback)( void* ptr, int level, const char* fmt, va_list vl );

class CFFUtils
{
public:
    static int InitFFmpeg();
    static int DeInitFFmpeg();
    static void SetLogCallback( FPLogCallback );

protected:
    static int LockMgr( void **mtx, enum AVLockOp op );
    static FPLogCallback mAVLogCallback;

private:
    static pthread_mutex_t ms_init_mutex;
    static int ms_ref_count;
};

}
#endif /* __C_FFUTILS_H__ */
