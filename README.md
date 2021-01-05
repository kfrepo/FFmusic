# FFmusic
基于 Android 平台的视频播放器，支持软解码和硬解码两种方式。
软解码基于 ffmpeg-3.3.9 库实现，硬解码利用 MediaCodec 实现。

解码后通过 OpenGL ES 绘制显示 YUV 数据，
利用 OpenSL ES 播放 PCM 数据，
同时集成了 SoundTouch 库，实现音频变速变调功能。


### 播放效果
![D7E5B659B8F1E1F83343F7EA9BF1D3FE](img//D7E5B659B8F1E1F83343F7EA9BF1D3FE.jpg)