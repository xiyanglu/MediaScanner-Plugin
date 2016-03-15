/*
 * CFFProbe.cpp
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
#include <stdio.h>
#include <string.h>
#include "include/HMediaDefs.h"

#include "include/CFFProbe.h"
#include "include/HError.h"
#include "include/CFFUtils.h"

#ifdef ANDROID
#include <utils/Log.h>
#ifndef LOG_NDEBUG
//#define LOG_NDEBUG 0
#endif

#ifndef LOG_TAG
#define LOG_TAG "CFFProbe"
#endif
#define XLOG(...) ALOGD(__VA_ARGS__)
#else
#define XLOG printf
#endif

namespace android
{

#define DEF_METADATA_TYPE 0
#define HEAD_METADATA_TYPE 1
#define STREAM_METADATA_TYPE 2

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

typedef struct
{
    const char *format;
    const char *container;
} formatmap;

static formatmap FILE_FORMATS[] = {
        {"mpeg",                    MEDIA_MIMETYPE_CONTAINER_MPEG2PS  },
        {"mpegts",                  MEDIA_MIMETYPE_CONTAINER_MPEG2TS       },
        {"mov,mp4,m4a,3gp,3g2,mj2", MEDIA_MIMETYPE_CONTAINER_MPEG4    },
        {"matroska,webm",           MEDIA_MIMETYPE_CONTAINER_MATROSKA },
        {"asf",                     MEDIA_MIMETYPE_CONTAINER_ASF1      },
        {"rm",                      MEDIA_MIMETYPE_CONTAINER_RM       },
        {"flv",                     MEDIA_MIMETYPE_CONTAINER_FLV      },
        {"swf",                     MEDIA_MIMETYPE_CONTAINER_FLV      },
        {"avi",                     MEDIA_MIMETYPE_CONTAINER_AVI      },
        {"ape",                     MEDIA_MIMETYPE_CONTAINER_APE      },
        {"dts",                     MEDIA_MIMETYPE_AUDIO_DTS1      },
        {"flac",                    MEDIA_MIMETYPE_AUDIO_FLAC     },
        {"ac3",                     MEDIA_MIMETYPE_AUDIO_AC3          },
        {"mp3",                     MEDIA_MIMETYPE_AUDIO_MPEG         },
        {"wav",                     MEDIA_MIMETYPE_CONTAINER_WAV      },
        {"ogg",                     MEDIA_MIMETYPE_CONTAINER_OGG      },
        {"vc1",                     MEDIA_MIMETYPE_CONTAINER_VC1      },
        {"hevc",                    MEDIA_MIMETYPE_CONTAINER_HEVC     },
};

static CFFProbe g_FFProbe;

CFFProbe* CFFProbe::GetInstance()
{
    return &g_FFProbe;
}

CFFProbe::CFFProbe()
{
    CFFUtils::InitFFmpeg();
}

CFFProbe::~CFFProbe()
{
    CFFUtils::DeInitFFmpeg();
}

int CFFProbe::RunFFProbe( const char* ckp_filename, char* cp_buffer, int i_bufsize, CProbeInfo* p_info )
{
    XLOG( "RunFFProbe...\n" );

    int res = XR_OK;
    if ( !ckp_filename || ckp_filename[0] == '\0' )
    {
        return XR_ILLEGAL_PARAM;
    }

    if ( res < 0 )
    {
        return res;
    }

    XLOG( "Open input file: %s\n", ckp_filename );

    AVFormatContext* p_ctx = NULL;
    res = avformat_open_input( &p_ctx, ckp_filename, NULL, NULL );
    if ( res != 0 || p_ctx == NULL )
    {
        XLOG( "Open input failed::res : %d, p_ctx : %s", res, p_ctx == NULL ? "null" : "!null" );
        if ( p_ctx != NULL )
            XLOG( "streams : %d", p_ctx->nb_streams );
        return XR_FAIL;
    }

    res = avformat_find_stream_info( p_ctx, NULL );
    if ( res >= 0 )
    {
        res = AVDumpFormat( p_ctx, ckp_filename, cp_buffer, i_bufsize, p_info );
    }

    //Close Codecs
    if ( p_ctx )
    {
        unsigned int i;
        for ( i = 0; i < p_ctx->nb_streams; ++i )
        {
            if ( p_ctx->streams[i]->codec->codec_id != AV_CODEC_ID_NONE )
            {
                avcodec_close( p_ctx->streams[i]->codec );
            }
        }
        avformat_close_input( &p_ctx );
    }

    return res;
}

int CFFProbe::AVDumpFormat( AVFormatContext* p_ctx, const char* ckp_filename, char* cp_buf, int i_buf_size, CProbeInfo* p_info )
{
    int res = XR_OK;
    char c_info_buf[1024];
    char buf[512];
    int i_buf_remain_size = i_buf_size;

    if ( p_ctx->nb_streams == 0 )
    {
        return XR_FAIL;
    }

    memset( c_info_buf, 0, sizeof( c_info_buf ) );
    memset( buf, 0, sizeof( buf ) );

    sprintf( c_info_buf, "Input #0, %s, from '%s':\n", p_ctx->iformat->name, ckp_filename );
    size_t len = strlen( c_info_buf );
    strncpy( cp_buf, c_info_buf, len );
    i_buf_remain_size -= len;
    memset( c_info_buf, 0, sizeof( c_info_buf ) );

    SetProbeInfo( p_ctx, p_info );

    return res;
}

int CFFProbe::SetProbeInfo( AVFormatContext* p_ctx, CProbeInfo* p_info )
{
    XLOG( "SetProbeInfo>>>>>>\n" );

    int res = XR_OK;
    uint32_t ul_rotation = 0;

    uint32_t ul_max_width = 0;
    uint32_t ul_max_height = 0;
    uint32_t ul_max_videorate = 0;
    uint32_t ul_max_audiorate = 0;
    uint32_t ul_prefer_vid = 0;
    uint32_t ul_prefer_aid = 0;

    uint32_t ul_stream_index = 0;
    uint32_t ul_num_video = 0;
    uint32_t ul_num_audio = 0;

    if ( p_info != NULL )
    {
        p_info->Reset();
    }
    else
    {
        XLOG( "SetProbeInfo: ProbeInfo is NULL.\n" );
        return XR_ILLEGAL_PARAM;
    }

    const int ki = p_ctx->nb_streams;
    CStreamInfo *stream_infos = new CStreamInfo[ki];

    if ( p_ctx->metadata && av_dict_count( p_ctx->metadata ) != 1 )
    {
        AVDictionaryEntry *tag = NULL;
        while ( ( tag = av_dict_get( p_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX ) ) )
        {
            //XLOG( "<key, value>=<%s, %s>\n", tag->key, tag->value );
            if ( strcasecmp( "title", tag->key ) == 0 )
            {
                StrCopy( p_info->mcp_title, tag->value, 0 );
            }
            else if ( strcasecmp( "artist", tag->key ) == 0 )
            {
                StrCopy( p_info->mcp_title, tag->value, 0 );
            }
            else if ( strcasecmp( "track", tag->key ) == 0 )
            {
                p_info->mi_track = atoi( tag->value );
            }
            else if ( strcasecmp( "album", tag->key ) == 0 )
            {
                StrCopy( p_info->mcp_album, tag->value, 0 );
            }
            else if ( strcasecmp( "genre", tag->key ) == 0 )
            {
                StrCopy( p_info->mcp_genre, tag->value, 0 );
            }
        }   //while
    }   //if

    const char* mime = FindMatchingContainer( p_ctx->iformat->name );
    StrCopy( p_info->mcp_mime, mime, 0 );
    XLOG( "Get Mime : %s  ==> %s\n", mime, p_info->mcp_mime );

    unsigned int i = 0;
    for ( i = 0; i < p_ctx->nb_streams; ++i )
    {
        AVStream* p_st = p_ctx->streams[i];
        const char* ckp_codec_name;
        const int i_codec_type = p_st->codec->codec_type;
        if ( i_codec_type != AVMEDIA_TYPE_AUDIO && i_codec_type != AVMEDIA_TYPE_VIDEO )
        {
            //skip none video/audio stream
            continue;
        }

        //common metadata
        stream_infos[ul_stream_index].mul_index = i;
        stream_infos[ul_stream_index].mul_codec_type = i_codec_type;
        ckp_codec_name = avcodec_get_name( p_st->codec->codec_id );
        strcpy( stream_infos[ul_stream_index].mcv_codec_name, ckp_codec_name );

        if ( i_codec_type == AVMEDIA_TYPE_VIDEO )
        {
            if ( strcasecmp( ckp_codec_name, "mjpeg" ) == 0 )
            {
                //skip image stream
                continue;
            }
            ++ul_num_video;
            stream_infos[ul_stream_index].mul_bitrate = p_st->codec->bit_rate;
            stream_infos[ul_stream_index].mul_height = p_st->codec->height;
            stream_infos[ul_stream_index].mul_width = p_st->codec->width;
            stream_infos[ul_stream_index].md_frame_rate = av_q2d( p_st->avg_frame_rate );

            //get extra meta, eg. rotation
            if ( p_st->metadata && av_dict_count( p_st->metadata ) != 1 )
            {
                AVDictionaryEntry *tag = NULL;
                while ( ( tag = av_dict_get( p_st->metadata, "", tag, AV_DICT_IGNORE_SUFFIX ) ) )
                {
                    XLOG( "<key, value>=<%s, %s>\n", tag->key, tag->value );
                    if ( strcmp( "rotate", tag->key ) == 0 )
                    {
                        ul_rotation = (uint32_t)atoi( tag->value );
                        stream_infos[ul_stream_index].mul_rotation = ul_rotation;
                    }	//if
                }	//while
            }	//if

            if ( ul_max_videorate < stream_infos[ul_stream_index].mul_bitrate )
            {
                ul_max_videorate = stream_infos[ul_stream_index].mul_bitrate;
            }

            if ( ul_max_height < stream_infos[ul_stream_index].mul_height )
            {
                ul_max_height = stream_infos[ul_stream_index].mul_height;
            }

            if ( ul_max_width < stream_infos[ul_stream_index].mul_width )
            {
                ul_max_width = stream_infos[ul_stream_index].mul_width;
                ul_prefer_vid = i;
            }
        }    //if video
        else if ( p_st->codec->codec_type == AVMEDIA_TYPE_AUDIO )
        {
            ++ul_num_audio;
            stream_infos[ul_stream_index].mul_channels = p_st->codec->channels;

            if ( ul_max_audiorate < stream_infos[ul_stream_index].mul_bitrate )
            {
                ul_max_audiorate = stream_infos[ul_stream_index].mul_bitrate;
                ul_prefer_aid = i;
            }

            if ( p_st->metadata && av_dict_count( p_st->metadata ) != 1 )
            {
                AVDictionaryEntry *tag = NULL;
                while ( ( tag = av_dict_get( p_st->metadata, "", tag, AV_DICT_IGNORE_SUFFIX ) ) )
                {
                    XLOG( "<key, value>=<%s, %s>\n", tag->key, tag->value );
                }   //while
            }   //if

        }    //if audio

        ++ul_stream_index;
    }    //for streams

    //
    if ( p_info != NULL )
    {
        if ( ul_num_video > 0 )
        {
            FormatMime( E_VIDEO_FILE, p_info->mcp_mime );
        }
        else if ( ul_num_audio > 0 )
        {
            FormatMime( E_AUDIO_FILE, p_info->mcp_mime );
        }
        p_info->md_duration = ( (double)p_ctx->duration ) / AV_TIME_BASE;
        p_info->mul_overall_bitrate = p_ctx->bit_rate;
        p_info->mul_num_audio = ul_num_audio;
        p_info->mul_num_video = ul_num_video;
        p_info->mul_max_height = ul_max_height;
        p_info->mul_max_width = ul_max_width;
        p_info->mul_max_videorate = ul_max_videorate;
        p_info->mul_max_audiorate = ul_max_audiorate;
        p_info->mul_rotation = stream_infos[ul_prefer_vid].mul_rotation;
        p_info->mul_stream_num = p_ctx->nb_streams;

        strcpy( p_info->mcv_audio_codec_name, stream_infos[ul_prefer_aid].mcv_codec_name );
        strcpy( p_info->mcv_video_codec_name, stream_infos[ul_prefer_vid].mcv_codec_name );
        p_info->md_dispaly_aspect_ratio = ul_max_width * 1.0f / ul_max_height;
        p_info->md_frame_rate = stream_infos[ul_prefer_vid].md_frame_rate;

        p_info->mp_StreamInfo = new CStreamInfo[p_info->mul_stream_num];
        for ( i = 0; i < p_info->mul_stream_num; ++i )
        {
            p_info->mp_StreamInfo[i] = stream_infos[i];
        }

    }

    if ( stream_infos != NULL )
    {
        delete[] stream_infos;
        stream_infos = NULL;
    }

    return res;
}

const char *CFFProbe::FindMatchingContainer( const char *name )
{
    size_t i = 0;
    const char *container = NULL;

    for ( i = 0; i < NELEM( FILE_FORMATS ); ++i )
    {
        int len = strlen( FILE_FORMATS[i].format );
        if ( !strncasecmp( name, FILE_FORMATS[i].format, len ) )
        {
            container = FILE_FORMATS[i].container;
            break;
        }
    }
    return container;
}

int CFFProbe::StrCopy( char*& dst, const char* src, size_t size )
{
    int res = XR_OK;
    size_t len = 0;
    if ( !src )
        return XR_ILLEGAL_PARAM;

    if ( size <= 0 )
        len = strlen( src );
    else
        len = size;

    if ( !dst )
    {
        dst = new char[len + 1];
    }

    if ( !dst )
        return XR_OUTOFMEMORY;

    strncpy( dst, src, len );

    dst[len] = '\0';

    return res;
}

int CFFProbe::ReadFrame( const char* ckp_filename, int64_t ll_timestamp, CFrameData* p_frame_data )
{
    XLOG( "ReadFrame>>>>>>\n" );
//    pthread_t tid = pthread_self();
//    XLOG("%s::Thread ID : %u : 0x%x",__FUNCTION__, (unsigned int)tid, (unsigned int)tid);

    int res = 0;
    int i_vid = -1;
    AVFormatContext* p_ctx = NULL;
    AVCodec *p_avcodec = NULL;
    AVCodecContext *p_avctx = NULL;
    AVFrame* p_frame = NULL;

    if ( !ckp_filename || ckp_filename[0] == '\0' )
    {
        return XR_ILLEGAL_PARAM;
    }
    if ( res < 0 )
    {
        return res;
    }

    XLOG( "Open input file: %s\n", ckp_filename );
    res = avformat_open_input( &p_ctx, ckp_filename, NULL, NULL );
    if ( res != 0 || p_ctx == NULL )
    {
        XLOG( "Open input failed::res : %d, p_ctx : %s\n", res, p_ctx == NULL ? "null" : "!null" );
        if ( p_ctx != NULL )
            XLOG( "streams : %d", p_ctx->nb_streams );
        return XR_FAIL;
    }

    res = avformat_find_stream_info( p_ctx, NULL );
    if ( res < 0 )
    {
        res = XR_FAIL;
        goto fail2;
    }
    if ( p_ctx->nb_streams <= 0 )
    {
        res = XR_FAIL;
        goto fail2;
    }

    //--------------------------------find video stream-------------------------------
    uint i;
    for ( i = 0; i < p_ctx->nb_streams; ++i )
    {
        if ( p_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            i_vid = i;
        }
    } //for
    if ( i_vid == -1 )
    {
        XLOG( "No Video Stream.\n" );
        goto fail2;
    }
    //--------------------------------Init Decoder-------------------------------
    p_avctx = p_ctx->streams[i_vid]->codec;
    p_avcodec = avcodec_find_decoder( p_avctx->codec_id );
    if ( p_avcodec == NULL )
    {
        XLOG( "Can not find decoder.\n" );
        res = XR_FAIL;
        goto fail2;
    }

    res = avcodec_open2( p_avctx, p_avcodec, NULL );
    if ( res < 0 )
    {
        XLOG( "Open decoder failed.\n" );
        res = XR_FAIL;
        goto fail2;
    }

    res = GetDecodedFrame( p_frame, p_ctx, ll_timestamp, i_vid, p_frame_data );

    fail2:
    //Close Codecs
    if ( p_ctx )
    {
        unsigned int i;
        for ( i = 0; i < p_ctx->nb_streams; ++i )
        {
            if ( p_ctx->streams[i]->codec->codec_id != AV_CODEC_ID_NONE )
            {
                avcodec_close( p_ctx->streams[i]->codec );
            }
        }
        avformat_close_input( &p_ctx );
    }
    if ( p_avctx )
    {
        avcodec_close( p_avctx );
    }

    return res;
}

int CFFProbe::GetDecodedFrame( AVFrame*& p_frame, AVFormatContext* p_ctx, int64_t ll_timestamp, int i_stream_id, CFrameData* p_frame_data )
{
    int res = 0;
    AVCodecContext* p_codec_ctx = NULL;
    AVPacket packet;

    if ( p_ctx == NULL )
    {
        return XR_ILLEGAL_PARAM;
    }
    //--------------------------------Init Frame-------------------------------
    if ( p_frame == NULL )
    {
        p_frame = avcodec_alloc_frame();
    }
    if ( p_frame == NULL )
    {
        XLOG( "Allocate frame failed.\n" );
        return XR_OUTOFMEMORY;
    }

    //--------------------------------Seek Frame-------------------------------
    if ( ll_timestamp < 0 )
    {
        ll_timestamp = p_ctx->duration / 4;
    }
    p_codec_ctx = p_ctx->streams[i_stream_id]->codec;
    if ( ll_timestamp > 0 )
    {
        //res = av_seek_frame(p_ctx, i_stream_id, ll_timestamp, 0);
        res = av_seek_frame( p_ctx, -1, ll_timestamp, 0 ); //AV_SEEK_ANY
        if ( res < 0 )
        {
            XLOG( "[%s] seek to [% " PRId64 "] failed.\n ", __FUNCTION__, ll_timestamp );
            return XR_FAIL;
        }
        avcodec_flush_buffers( p_codec_ctx );
    }
    else
    {
        XLOG( "[%s] seek to [%" PRId64 "] failed.\n ", __FUNCTION__, ll_timestamp );
    }
    if ( ll_timestamp < 0 )
        ll_timestamp = 0;

    //--------------------------------Read Frame-------------------------------
    int i_got_pic = 0;
    while ( true )             //try 2 times
    {
        res = av_read_frame( p_ctx, &packet );
        if ( res < 0 )
        {
            XLOG( "AVReadFrame failed.\n" );
            break;
        }
        if ( packet.stream_index == i_stream_id )
        {
            avcodec_decode_video2( p_codec_ctx, p_frame, &i_got_pic, &packet );
            XLOG( "GotPic : %d\n", i_got_pic );
        }
        if ( i_got_pic && p_frame->key_frame )
        {
            //TODO Convert Picture
            unsigned char* rgb_buf = NULL;
            res = ConvertFrame( p_frame, rgb_buf );

            if ( res >= 0 )
            {
                int height = p_frame->height;
                int width = p_frame->width;
                int type = p_frame->pict_type;
                int format = p_frame->format;
                XLOG( "width : %d, \t height : %d, \t type : %d,\t format : %d \n", width, height, type, format );

                if ( p_frame_data == NULL )
                {
                    p_frame_data = new CFrameData();
                }
                p_frame_data->mi_height = p_frame->height;
                p_frame_data->mi_width = p_frame->width;
#if CONVERT_YUV2RGB
                p_frame_data->mi_type = eRGB_24;
                p_frame_data->mul_size = p_frame->width * p_frame->height * 3;
#else
                p_frame_data->mi_type = eYUV_420_Planer;
                p_frame_data->mul_size = p_frame->width * p_frame->height * 3 / 2;
#endif

                if ( rgb_buf != NULL )
                {
                    p_frame_data->m_data = (uint8_t*)rgb_buf;
                }
                else
                {
                    XLOG( "RGB Buffer is NULL.\n" );
                }
            }
            else
            {
                XLOG( "ConvertFrame Failed : [%d]\n", res );
            }
            break;
        }
        else if ( i_got_pic )
        {
            XLOG( "Got frame, but not key frame" );
        }
    }
    return res;
}

int CFFProbe::ConvertFrame( AVFrame* p_frame, unsigned char*& p_data )
{
    int res = 0;
    int width = 0;
    int height = 0;

    if ( p_frame == NULL )
    {
        return XR_ILLEGAL_PARAM;
    }

    width = p_frame->width;
    height = p_frame->height;

#if CONVERT_YUV2RGB
    CColorConvert color_convert;

    unsigned char * py = NULL;
    unsigned char * pu = NULL;
    unsigned char * pv = NULL;

    p_data = new unsigned char[width * height * 3];
    if ( p_data == NULL )
    {
        return R_OUTOFMEMORY;
    }

    py = p_frame->data[0];
    pu = p_frame->data[1];
    pv = p_frame->data[2];
    bool b = color_convert.ConvertYUV420ToRgb32(py, pu, pv, p_data, width, height);
    res = b ? XR_OK : XR_FAIL;

#else
    int i_buf_size = width * height * 3 / 2;
    p_data = new unsigned char[i_buf_size];
    int offset = 0;
    int i = 0;
    int linesize = 0;
    unsigned char* p_tmp = NULL;

    FILE* f_yuv = fopen("frame.yuv", "wb");

    //y
    linesize = p_frame->linesize[0];
    p_tmp = p_frame->data[0];
    for ( i = 0; i < height; i++ )
    {
        fwrite(p_tmp, width, 1, f_yuv);
        memcpy( p_data + offset, p_tmp, width );
        p_tmp += linesize;
        offset += width;
    }

    //u
    linesize = p_frame->linesize[1];
    p_tmp = p_frame->data[1];
    int half_height = height / 2;
    int half_width = width / 2;
    for ( i = 0; i < half_height; i++ )
    {
        fwrite(p_tmp, half_width, 1, f_yuv);
        memcpy( p_data + offset, p_tmp, half_width );
        p_tmp += linesize;
        offset += half_width;
    }

    //v
    linesize = p_frame->linesize[2];
    p_tmp = p_frame->data[2];
    for ( i = 0; i < half_height; i++ )
    {
        fwrite(p_tmp, half_width, 1, f_yuv);
        memcpy( p_data + offset, p_tmp, half_width );
        p_tmp += linesize;
        offset += half_width;
    }

    fclose(f_yuv);
#endif

    return res;
}

int CFFProbe::FormatMime( int filetype, char*& cp_mime )
{
    int res = XR_OK;

    if ( cp_mime != NULL && cp_mime[0] != '\0' )
    {
        switch ( filetype )
        {
        case E_UNKNOWN:
            XLOG( "Can not recognize file type.\n" );
            res = XR_ILLEGAL_PARAM;
            break;

        case E_AUDIO_FILE:
            XLOG( "This is audio file.\n" );
            //Do Not Adjust "application*" For Ringstones
            if ( strncasecmp( "audio/", cp_mime, 6 ) && strncasecmp( "application/", cp_mime, 12 ) )
            {
                strncpy( cp_mime, "audio/", 6 );
                XLOG( "Adjust MIME ==> %s\n", cp_mime );
            }
            break;

        case E_VIDEO_FILE:
            XLOG( "This is video file.\n" );
            if ( strncasecmp( "video/", cp_mime, 6 ) && strncasecmp( "application/", cp_mime, 12 ) )
            {
                strncpy( cp_mime, "video/", 6 );
                XLOG( "Adjust MIME ==> %s\n", cp_mime );
            }

            break;
        default:
            XLOG( "Can not recognize file type.\n" );
            break;
        }             //switch
    }
    else
    {
        res = XR_ILLEGAL_PARAM;
    }
    return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------CColorConvert------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////////////////
CColorConvert::CColorConvert()
{
    mb_IsInitialized = false;
}

CColorConvert::~CColorConvert()
{
    mb_IsInitialized = false;
}

void CColorConvert::InitTable()
{
    long int crv, cbu, cgu, cgv;
    int i, ind;

    mb_IsInitialized = true;

    crv = 104597;
    cbu = 132201;
    cgu = 25675;
    cgv = 53279;

    for ( i = 0; i < 256; i++ )
    {
        m_crvTab[i] = ( i - 128 ) * crv;
        m_cbuTab[i] = ( i - 128 ) * cbu;
        m_cguTab[i] = ( i - 128 ) * cgu;
        m_cgvTab[i] = ( i - 128 ) * cgv;
        m_tab76309[i] = 76309 * ( i - 16 );
    }

    for ( i = 0; i < 384; i++ )
        m_clp[i] = 0;

    ind = 384;

    for ( i = 0; i < 256; i++ )
        m_clp[ind++] = i;

    ind = 640;

    for ( i = 0; i < 384; i++ )
        m_clp[ind++] = 255;
}

bool CColorConvert::ConvertYUV420ToRgb32(unsigned char *src0,
                                        unsigned char *src1,
                                        unsigned char *src2,
                                        unsigned char *dst_ori,
                                        int width,
                                        int height)
{
    int y1, y2, u, v;
    unsigned char *py1, *py2;
    int i, j, c1, c2, c3, c4;
    unsigned char *d1, *d2;

    if ( !src0 || !src1 || !src2 || !dst_ori )
        return false;

    if ( width < 0 || height < 0 )
        return false;

    if ( !mb_IsInitialized )
        InitTable();

    py1 = src0;
    py2 = py1 + width;

    d1 = dst_ori;
    d2 = d1 + 3 * width;

    for ( j = 0; j < height; j += 2 )
    {
        for ( i = 0; i < width; i += 2 )
        {
            u = *src1++;
            v = *src2++;

            c1 = m_crvTab[v];
            c2 = m_cguTab[u];
            c3 = m_cgvTab[v];
            c4 = m_cbuTab[u];

            //up-left
            y1 = m_tab76309[*py1++];

            *d1++ = m_clp[384 + ( ( y1 + c1 ) >> 16 )];
            *d1++ = m_clp[384 + ( ( y1 - c2 - c3 ) >> 16 )];
            *d1++ = m_clp[384 + ( ( y1 + c4 ) >> 16 )];

            //down-left
            y2 = m_tab76309[*py2++];
            *d2++ = m_clp[384 + ( ( y2 + c1 ) >> 16 )];
            *d2++ = m_clp[384 + ( ( y2 - c2 - c3 ) >> 16 )];
            *d2++ = m_clp[384 + ( ( y2 + c4 ) >> 16 )];

            //up-right
            y1 = m_tab76309[*py1++];
            *d1++ = m_clp[384 + ( ( y1 + c1 ) >> 16 )];
            *d1++ = m_clp[384 + ( ( y1 - c2 - c3 ) >> 16 )];
            *d1++ = m_clp[384 + ( ( y1 + c4 ) >> 16 )];

            //down-right
            y2 = m_tab76309[*py2++];
            *d2++ = m_clp[384 + ( ( y2 + c1 ) >> 16 )];
            *d2++ = m_clp[384 + ( ( y2 - c2 - c3 ) >> 16 )];
            *d2++ = m_clp[384 + ( ( y2 + c4 ) >> 16 )];
        }

        d1 += 3 * width;
        d2 += 3 * width;

        py1 += width;
        py2 += width;
    }

    return true;
}

}             //android name space
