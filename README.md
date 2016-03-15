# MediaScanner-Plugin
    MediaScanner Plugin is developed based on ffmpeg, which is used to extract media metadata.  MediaScanner Plugin could running on Android/Linux/Windows.
    The basic purpose is to enhance Android MediaScanner supporting additional media files.

## Functions
- Extracting Media Metadata
- Extracting Video Frame

## Compile
**Android**

This plugin depends on ffmpeg.

Edite ffmpeg path in Android.mk ：

    FFMPEG_SRC_DIR := FFMPEG_PATH

Building out is libffscanner_plugin.so, you can find is in "out/android/" folder

**Linux/Windows**

As long as you have ffmpeg development environment.

## License
Copyright 2015 Guo He <lovenight@126.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
