/*
 * CFFUtils.cpp
 *
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

#include "include/CFFUtils.h"
#include "include/HError.h"

extern "C"
{
#include <string.h>
#include <pthread.h>
#include <libavutil/log.h>
#ifdef ANDROID
#include <libavformat/url.h>
#endif
}

#ifdef ANDROID
#include <utils/Log.h>
#include <cutils/properties.h>
#include "include/CFFSource.h"
#ifndef LOG_NDEBUG
//#define LOG_NDEBUG 0
#endif
#ifndef LOG_TAG
#define LOG_TAG "CFFUtils"
#endif
#define XLOG(...) ALOGE(__VA_ARGS__)
#else
#include <stdio.h>

#define XLOG printf
#endif

namespace android
{
////////////////////////////////////////////////////////////////////////////////////////////
static void sanitize( uint8_t *line )
{
    while ( *line )
    {
        if ( *line < 0x08 || ( *line > 0x0D && *line < 0x20 ) )
            *line = '?';
        line++;
    }
}

static void AVLogCallback( void* ptr, int level, const char* fmt, va_list vl )
{
    static int print_prefix = 1;
    static int count;
    static char prev[1024];
    char line[1024];

    if ( level > av_log_get_level() )
        return;
    av_log_format_line( ptr, level, fmt, vl, line, sizeof( line ), &print_prefix );

    if ( print_prefix && !strcmp( line, prev ) )
    {
        count++;
        return;
    }
    if ( count > 0 )
    {
        XLOG( "Last message repeated %d times\n", count );
        count = 0;
    }
    strcpy( prev, line );
    sanitize( (uint8_t *)line );

    XLOG( "%s", line );
}
////////////////////////////////////////////////////////////////////////////////////////////
FPLogCallback CFFUtils::mAVLogCallback = AVLogCallback;
pthread_mutex_t CFFUtils::ms_init_mutex = PTHREAD_MUTEX_INITIALIZER;
int CFFUtils::ms_ref_count = 0;

void CFFUtils::SetLogCallback( FPLogCallback fp )
{
    mAVLogCallback = fp;
}

int CFFUtils::InitFFmpeg()
{
    int res = XR_OK;
    bool debug_enabled = false;

    pthread_mutex_lock( &ms_init_mutex );

#ifdef ANDROID
    char value[PROPERTY_VALUE_MAX];
    if (property_get("debug.leui.ffmpeg", value, NULL)
            && (!strcmp(value, "1") || !strcasecmp(value, "true")))
    {
        ALOGI("set ffmpeg debug level to AV_LOG_DEBUG");
        debug_enabled = true;
    }
#endif

    if ( debug_enabled )
        av_log_set_level( AV_LOG_DEBUG );
    else
        av_log_set_level( AV_LOG_INFO );

    if ( ms_ref_count == 0 )
    {
        av_log_set_callback( mAVLogCallback );

        /* register all codecs, demux and protocols */
        avcodec_register_all();
        av_register_all();
        avformat_network_init();

#ifdef ANDROID
        /* register android source */
        //ffurl_register_protocol(CreateAndroidSource());
#endif

        if ( av_lockmgr_register( LockMgr ) )
        {
            XLOG( "could not initialize lock manager!" );
            res = XR_FAIL;
        }
    }

    // update counter
    ms_ref_count++;

    pthread_mutex_unlock( &ms_init_mutex );

    return res;
}

int CFFUtils::DeInitFFmpeg()
{
    int res = XR_OK;
    pthread_mutex_lock( &ms_init_mutex );

    // update counter
    ms_ref_count--;

    if ( ms_ref_count == 0 )
    {
        av_lockmgr_register( NULL );
        avformat_network_deinit();
    }

    pthread_mutex_unlock( &ms_init_mutex );
    return res;
}

int CFFUtils::LockMgr( void **mtx, enum AVLockOp op )
{
    switch ( op )
    {
    case AV_LOCK_CREATE:
        *mtx = (void *)av_malloc( sizeof(pthread_mutex_t) );
        if ( !*mtx )
            return 1;
        return !!pthread_mutex_init( (pthread_mutex_t *)( *mtx ), NULL );
    case AV_LOCK_OBTAIN:
        return !!pthread_mutex_lock( (pthread_mutex_t *)( *mtx ) );
    case AV_LOCK_RELEASE:
        return !!pthread_mutex_unlock( (pthread_mutex_t *)( *mtx ) );
    case AV_LOCK_DESTROY:
        pthread_mutex_destroy( (pthread_mutex_t *)( *mtx ) );
        av_freep( mtx );
        return 0;
    }
    return 1;
}

} //android namespace
