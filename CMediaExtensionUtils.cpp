/*
 * MediaExtensionUtils.cpp
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

#include <unistd.h>
#include "include/CMediaExtensionUtil.h"

#ifdef ANDROID
#include <utils/Log.h>
#ifndef LOG_NDEBUG
//#define LOG_NDEBUG 0
#endif
#ifndef LOG_TAG
    #define LOG_TAG "MediaExtensionUtil"
#endif
#define XLOG(...) ALOGD(__VA_ARGS__)
#else
#include <stdio.h>
#define XLOG printf
#endif

namespace android
{

static const char *VIDEO_EXTENSIONS[] =
{ "mp4", "3gp", "3gpp", "3g2", "3gpp2", "mpeg", "mkv", "webm", "avi", "mpg", "wmv", "mov", "rm", "rmvb", "asf", "divx", "flv", "f4v", "m2ts" };
static const char *AUDIO_EXTENSIONS[] =
{ "mp3", "m4a", "ogg", "mid", "wma", "aac", "wav", "amr", "mka", "flac", "mpga", "ac3", "dts", "eac3", "ape" };

const unsigned int MediaExtensionUtil::PATH_MAX_LEN = 256;

const char* MediaExtensionUtil::ParserExtension( const char* filename )
{
    const char* p_suffix = NULL;

    if ( filename == NULL || filename[0] == '\0' )
        return NULL;

    p_suffix = strrchr( filename, '.' );
    if ( p_suffix == NULL )
        return NULL;

    p_suffix++;

    return p_suffix;
}

int MediaExtensionUtil::Str2Lower( char* dst, const char* src, int len )
{
    if ( src == NULL || dst == NULL || len <= 0 )
        return -1;
    int i = 0;
    for ( i = 0; i < len; ++i )
    {
        if ( 'A' <= src[i] && src[i] <= 'Z' )
        {
            dst[i] = src[i] + 'a' - 'A';
        }
        else
        {
            dst[i] = src[i];
        }
    }
    dst[len] = '\0';
    return 0;
}

int MediaExtensionUtil::CheckExtensionType( const char* filename )
{
    const char *p_ext = NULL;
    p_ext = ParserExtension( filename );
    if ( p_ext == NULL || p_ext[0] == '\0' )
        return E_UNKNOWN;

    int i_len = strlen( p_ext );
    if ( i_len <= 0 || i_len > 5 )
        return E_UNKNOWN;

    XLOG( "SRC Extension : %s\n", p_ext );
    char cv_ext[i_len + 1];
    int res = Str2Lower( cv_ext, p_ext, i_len );
    if ( res != 0 )
        return E_UNKNOWN;
    XLOG( "Converted Extension : %s\n", cv_ext );

    static int i_vsize = sizeof( VIDEO_EXTENSIONS ) / sizeof( VIDEO_EXTENSIONS[0] );
    static int i_asize = sizeof( AUDIO_EXTENSIONS ) / sizeof( AUDIO_EXTENSIONS[0] );

    int i = 0;
    for ( i = 0; i < i_vsize; ++i )
    {
        if ( !strcasecmp( cv_ext, VIDEO_EXTENSIONS[i] ) )
        {
            return E_VIDEO_FILE;
        }
    }
    for ( i = 0; i < i_asize; ++i )
    {
        if ( !strcasecmp( cv_ext, AUDIO_EXTENSIONS[i] ) )
        {
            return E_AUDIO_FILE;
        }
    }

    return E_UNKNOWN;
}

int MediaExtensionUtil::CheckExtensionType( int fd )
{
    char* filename = new char[PATH_MAX_LEN];
    memset(filename, 0, PATH_MAX_LEN);
    getFileName( fd, filename );
    int res = E_UNKNOWN;
    if ( filename != NULL && filename[0] != '\0' )
    {
        XLOG( "Filename : %s\n", filename );
        res = CheckExtensionType( filename );
    }

    delete[] filename;

    return res;
}

int MediaExtensionUtil::getFileName( int fd, char*& filename )
{
    if ( fd )
    {
        char symName[40] =
        { 0 };
        if ( filename == NULL )
        {
            filename = new char[PATH_MAX_LEN];
            if ( filename == NULL )
                return -1;
            memset( filename, 0, PATH_MAX_LEN );
        }
        snprintf( symName, sizeof( symName ), "/proc/%d/fd/%d", getpid(), fd );

        if ( readlink( symName, filename, ( PATH_MAX_LEN - 1 ) ) != -1 )
        {
            return 0;
        }
    }

    return -1;
}

}

