# FFmusic
基于ffmpeg-3.3.9实现的视频播放器。

### 解码
软解码：使用 ffmpeg-3.3.9 库进行视频解码。
硬解码：利用 MediaCodec 实现高效的视频硬解码。

### 播放
解码后通过 OpenGL ES 绘制显示 YUV 数据，通过 OpenSL ES 播放 PCM 数据。
同时集成了 SoundTouch 库，实现音频变速变调功能。


### 播放效果
![D7E5B659B8F1E1F83343F7EA9BF1D3FE](img//D7E5B659B8F1E1F83343F7EA9BF1D3FE.jpg)