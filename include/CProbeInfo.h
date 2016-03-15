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

#ifndef __C_PROBEINFO_H__
#define __C_PROBEINFO_H__

#include <stdint.h>
#include <string.h>

#define MAX_CODEC_NAME_LEN 64
#define CODEC_TYPE_UNKNOWN 0
#define CODEC_TYPE_AUDIO   1
#define CODEC_TYPE_VIDEO   2

enum pic_format
{
    eYUV_420_Planer,
    eRGB_565,
    eRGB_24,
    eRGB_32,
};

namespace android
{

class CStreamInfo
{
public:
    // general info:
    uint32_t mul_index;
    char mcv_codec_name[MAX_CODEC_NAME_LEN];
    uint32_t mul_codec_type;
    uint32_t mul_bitrate;

    // video info:
    uint32_t mul_width;
    uint32_t mul_height;
    double md_display_aspect_ratio;
    double md_frame_rate;
    uint32_t mul_rotation;

    // audio info:
    uint32_t mul_channels;

    CStreamInfo() :
            mul_index( 0 ), mul_codec_type( CODEC_TYPE_UNKNOWN ), mul_bitrate( 0 ), mul_width( 0 ), mul_height( 0 ), md_display_aspect_ratio( 0.0 ), md_frame_rate( 0.0 ), mul_rotation( 0 ), mul_channels(
                    0 )
    {
        mcv_codec_name[0] = '\0';
    }

    CStreamInfo& operator=( const CStreamInfo& si )
    {
        if ( this != &si )
        {
            mul_index = si.mul_index;
            mul_codec_type = si.mul_codec_type;
            mul_bitrate = si.mul_bitrate;
            mul_width = si.mul_width;
            mul_height = si.mul_height;
            md_display_aspect_ratio = si.md_display_aspect_ratio;
            md_frame_rate = si.md_frame_rate;
            mul_rotation = si.mul_rotation;
            mul_channels = si.mul_channels;

            strcpy( mcv_codec_name, si.mcv_codec_name );
        }
        return *this;
    }

    ~CStreamInfo()
    {
    }
};

class CProbeInfo
{
public:
    double md_duration;
    uint32_t mul_overall_bitrate;
    uint32_t mul_num_audio;
    uint32_t mul_num_video;
    uint32_t mul_num_image;
    uint32_t mul_max_width;
    uint32_t mul_max_height;
    uint32_t mul_max_videorate;
    uint32_t mul_max_audiorate;
    uint32_t mul_rotation;

    bool mb_has_audio;
    bool mb_has_video;

    double md_dispaly_aspect_ratio;
    double md_frame_rate;
    char mcv_video_codec_name[MAX_CODEC_NAME_LEN];
    char mcv_audio_codec_name[MAX_CODEC_NAME_LEN];
    int mi_track;
    char* mcp_artist;
    char* mcp_album;
    char* mcp_title;
    char* mcp_genre;
    char* mcp_mime;

    uint32_t mul_stream_num;
    CStreamInfo* mp_StreamInfo;
    uint32_t mul_preferred_video_index;
    uint32_t mul_preferred_audio_index;

    CProbeInfo() :
            md_duration( 0.0 ), mul_overall_bitrate( 0 ), mul_num_audio( 0 ), mul_num_video( 0 ), mul_num_image( 0 ), mul_max_width( 0 ), mul_max_height( 0 ), mul_max_videorate( 0 ), mul_max_audiorate(
                    0 ), mul_rotation( 0 ), mb_has_audio( false ), mb_has_video( false ), md_dispaly_aspect_ratio( 0.0 ), md_frame_rate( 0.0 ), mi_track( 0 ), mcp_artist( NULL ), mcp_album( NULL ), mcp_title(
            NULL ), mcp_genre( NULL ), mcp_mime( NULL ), mul_stream_num( 0 ), mp_StreamInfo( NULL ), mul_preferred_video_index( 0 ), mul_preferred_audio_index( 0 )
    {
        printf("CProbeInfo Construction.\n");
    }

    ~CProbeInfo()
    {
        printf("CProbeInfo Destruction.\n");
        mcv_video_codec_name[0] = '\0';
        mcv_audio_codec_name[0] = '\0';
        if ( mp_StreamInfo )
            delete[] mp_StreamInfo;

        if ( mcp_artist )
        {
            delete[] mcp_artist;
            mcp_artist = NULL;
        }

        if ( mcp_album )
        {
            delete[] mcp_album;
            mcp_album = NULL;
        }

        if ( mcp_title )
        {
            delete[] mcp_title;
            mcp_title = NULL;
        }

        if ( mcp_genre )
        {
            delete[] mcp_genre;
            mcp_genre = NULL;
        }

        if ( mcp_mime )
        {
            delete[] mcp_mime;
            mcp_mime = NULL;
        }
    }

    void Reset()
    {
        md_duration = 0.0;
        mul_overall_bitrate = 0;
        mul_num_audio = 0;
        mul_num_video = 0;
        mul_num_image = 0;
        mul_max_width = 0;
        mul_max_height = 0;
        mul_max_videorate = 0;
        mul_max_audiorate = 0;
        mul_rotation = 0;
        mb_has_audio = false;
        mb_has_video = false;
        md_dispaly_aspect_ratio = 0.0;
        md_frame_rate = 0.0;
        mi_track = 0;
        if ( mcp_artist )
            mcp_artist[0] = '\0';
        if ( mcp_album )
            mcp_album[0] = '\0';
        if ( mcp_title )
            mcp_title[0] = '\0';
        if ( mcp_genre )
            mcp_genre[0] = '\0';
        if ( mcp_mime )
            mcp_mime[0] = '\0';

        if ( mp_StreamInfo )
        {
            delete[] mp_StreamInfo;
            mp_StreamInfo = NULL;
        }
        mul_stream_num = 0;

        mul_preferred_video_index = 0;
        mul_preferred_audio_index = 0;

        mcv_video_codec_name[0] = '\0';
        mcv_audio_codec_name[0] = '\0';
    }

};

class CFrameData
{
public:
    int mi_width;
    int mi_height;
    int mi_rotation;
    int mi_type;
    uint32_t mul_size;
    uint8_t* m_data;

    CFrameData()
    {
        mi_width = 0;
        mi_height = 0;
        mi_rotation = 0;
        mi_type = 0;
        mul_size = 0;
        m_data = 0;
    }

    CFrameData( int i_width, int i_height, int i_rotation, int i_type, uint32_t ul_size, uint8_t* data )
    {
        mi_width = i_width;
        mi_height = i_height;
        mi_type = i_type;
        mi_rotation = i_rotation;
        mul_size = ul_size;
        data = new uint8_t[ul_size];
        memcpy( m_data, data, ul_size );
    }

    ~CFrameData()
    {
        if ( m_data )
        {
            delete[] m_data;
        }
    }

    CFrameData& operator=( const CFrameData& si )
    {
        if ( this != &si )
        {
            mi_width = si.mi_width;
            mi_height = si.mi_height;
            mi_type = si.mi_type;
            mi_rotation = si.mi_rotation;
            mul_size = si.mul_size;

            memcpy( m_data, si.m_data, mul_size );
        }
        return *this;
    }
};

}

#endif /* __C_PROBEINFO_H__ */
