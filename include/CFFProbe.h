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

#ifndef __C_FFPROBE_H__
#define __C_FFPROBE_H__

#include "CProbeInfo.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

namespace android
{

class CFFProbe
{
public:
    CFFProbe();
    virtual ~CFFProbe();

    static CFFProbe* GetInstance();

    int RunFFProbe( const char* ckp_filename, char* cp_buffer, int i_bufsize, CProbeInfo* p_info );
    int ReadFrame( const char* ckp_filename, int64_t ll_index, CFrameData* p_frame_data );
    int FormatMime(int filetype, char*& cp_mime);

private:
    enum
    {
        E_UNKNOWN = 0, E_AUDIO_FILE = 1, E_VIDEO_FILE = 2,
    };

    int AVDumpFormat( AVFormatContext* p_ctx, const char* ckp_filename, char* p_buf, int i_buf_size, CProbeInfo* p_info );
    int SetProbeInfo( AVFormatContext* p_ctx, CProbeInfo* p_info );
    int StrCopy( char*& dst, const char* src, size_t size );
    const char *FindMatchingContainer( const char *name );

    int GetDecodedFrame( AVFrame*& p_frame, AVFormatContext* p_ctx, int64_t ll_timestamp, int i_stream_id, CFrameData* p_frame_data );
    int ConvertFrame( AVFrame* p_frame, unsigned char*& p_data );
};

class CColorConvert
{
private:
    long int m_crvTab[256];
    long int m_cbuTab[256];
    long int m_cguTab[256];

    long int m_cgvTab[256];
    long int m_tab76309[256];
    unsigned char m_clp[1024];

    bool mb_IsInitialized;

public:
    CColorConvert();
    ~CColorConvert();
    void InitTable();
    bool ConvertYUV420ToRgb32( unsigned char *src0, unsigned char *src1, unsigned char *src2, unsigned char *dst_ori, int width, int height );
};

} //android name space

#endif /* __C_FFPROBE_H__ */
