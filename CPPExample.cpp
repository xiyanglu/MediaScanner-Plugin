/* CPPExample.cpp
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

#include <iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "include/CMediaExtensionUtil.h"
#include "include/CFFProbe.h"

using namespace std;
using namespace android;

const char* filename = "/home/guohe/Videos/adele/Rolling_In_Deep.mp4";

struct SArgs
{
    CFFProbe* p_ffthumb;
    char filename[256];
};

void *func( void *p )
{
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();

    printf( "****Run thread : Pid = %d, Tid = %lu.\n", pid, tid );
    SArgs* p_args = (SArgs*)p;
    printf( "Filename : %s\n", p_args->filename );

    CFrameData frame_data;
    CFFProbe probe;
    int res = probe.ReadFrame( filename, 0, &frame_data );
    printf( "Res = %d\n", res );
    int height = frame_data.mi_height;
    int width = frame_data.mi_width;
    int type = frame_data.mi_type;
    int rotation = frame_data.mi_rotation;
    uint32_t size = frame_data.mul_size;

    printf( "[%d] width : %d, \t height : %d, \t type : %d,\t rotation : %d,\t size : %ul \n", tid, width, height, type, rotation, size );

    return NULL;
}

void TestFFProbe()
{
    int res = R_OK;

    printf( "*****************RunFFProbe****************\n" );
    CFFProbe *FFProbe = CFFProbe::GetInstance();
    char info[1024];

    for ( int i = 0; i < 20; ++i )
    {
        CProbeInfo *pProbeInfo = new CProbeInfo();
        res = FFProbe->RunFFProbe( filename, info, 1024, pProbeInfo );

        printf( "Res = %d\n", res );
        printf( "Mime : %s, Audio Codec : %s, Video Codec : %s\n", pProbeInfo->mcp_mime, pProbeInfo->mcv_video_codec_name, pProbeInfo->mcv_audio_codec_name );

        res = FFProbe->RunFFProbe( "/home/guohe/Videos/FIRST.CLASS.2.Ep10.Final.rmvb", info, 1024, pProbeInfo );

        printf( "Res = %d\n", res );
        printf( "Mime : %s, Audio Codec : %s, Video Codec : %s\n", pProbeInfo->mcp_mime, pProbeInfo->mcv_video_codec_name, pProbeInfo->mcv_audio_codec_name );
    }

    printf( "*****************GenThumb****************\n" );

    CFrameData frame_data;
    res = FFProbe->ReadFrame( "/home/guohe/Videos/FIRST.CLASS.2.Ep10.Final.rmvb", 100, &frame_data );
    printf( "Res = %d\n", res );

    int height = frame_data.mi_height;
    int width = frame_data.mi_width;
    int type = frame_data.mi_type;
    int rotation = frame_data.mi_rotation;
    uint32_t size = frame_data.mul_size;

    printf( "width : %d, \t height : %d, \t type : %d,\t rotation : %d,\t size : %ul \n", width, height, type, rotation, size );
}

int main()
{
    cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
    int res = MediaExtensionUtil::E_UNKNOWN;
    int fd = open( filename, O_RDONLY );
    res = MediaExtensionUtil::CheckExtensionType( fd );
    //res = MediaExtensionUtil::CheckExtensionType(filename);
    cout << "file type : " << res << endl;
    switch ( res )
    {
    case MediaExtensionUtil::E_UNKNOWN:
        cout << "Can not recognize file type." << endl;
        break;

    case MediaExtensionUtil::E_AUDIO_FILE:
        cout << "This is audio file." << endl;
        break;

    case MediaExtensionUtil::E_VIDEO_FILE:
        cout << "this is video file." << endl;
        break;
    }

    //////////////////////////////////////////////////////
    TestFFProbe();
    //////////////////////////////////////////////////////
/*
    //Thread Test
    pthread_t pid[10];
    SArgs args;
    strncpy( args.filename, filename, strlen( filename ) );
    args.filename[strlen( filename )] = '\0';
    args.p_ffthumb = CFFProbe::GetInstance();
    pthread_create( &pid[0], NULL, func, (void*)&args );
    pthread_create( &pid[1], NULL, func, (void*)&args );
    pthread_create( &pid[2], NULL, func, (void*)&args );
    pthread_create( &pid[3], NULL, func, (void*)&args );
    pthread_create( &pid[4], NULL, func, (void*)&args );
    pthread_create( &pid[5], NULL, func, (void*)&args );
    pthread_create( &pid[6], NULL, func, (void*)&args );
    pthread_create( &pid[7], NULL, func, (void*)&args );
    pthread_create( &pid[8], NULL, func, (void*)&args );
    pthread_create( &pid[9], NULL, func, (void*)&args );

    while ( 1 )
    {
        if ( getchar() == 'D' )
            break;
        else
            sleep( 1000 );
    }
    void *tret;
    for ( int i = 0; i < 10; ++i )
    {
        pthread_join( pid[i], &tret );
//        cout<<"Thread "<<i+1<<" exist wth code "<<*(int*)tret<<endl;
        printf( "Thread %d exist with code %d", i + 1, *(int*)tret );
    }
*/
    return 0;
}
