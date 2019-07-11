package com.wguet.ffmusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.player.FFPlayer;
import com.wguet.myplayer.util.LogUtil;


public class MainActivity extends AppCompatActivity {


    private FFPlayer ffPlayer;

    private Button startPlay;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        startPlay = (Button) findViewById(R.id.startbutton);

        ffPlayer = new FFPlayer();
        ffPlayer.setPreparedListener(new FFOnPreparedListener() {
            @Override
            public void onPrepared() {
                LogUtil.d("准备好 开始播放音频！");
                ffPlayer.start();
            }
        });

        ffPlayer.setFfOnLoadListener(new FFOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if(load) {
                    LogUtil.d("加载中...");
                }
                else {
                    LogUtil.d("播放中...");
                }
            }
        });

        ffPlayer.setFfOnPauseResumeListener(new FFOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause) {
                    LogUtil.d("暂停中...");
                }
                else {
                    LogUtil.d("播放中...");
                }
            }
        });

        startPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                ffPlayer.setSource("/mnt/sdcard/Music/mydream.mp3");
                ffPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                ffPlayer.prepared();
            }
        });



        findViewById(R.id.pausebutton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.pause();
            }
        });
        findViewById(R.id.resumebutton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.resume();
            }
        });

//        Demo demo;
//        demo = new Demo();
//        demo.testFFmpeg();
//        demo.setOnErrotListener(new Demo.OnErrotListener() {
//            @Override
//            public void onError(int code, String msg) {
//                Log.e("MainActivity", "code = " + code + " msg = " + msg);
//            }
//        });
//
//        demo.normalThread();
//        demo.mutexThread();
//
//        mainThread.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                demo.callbackFromC();
//            }
//        });
//
//        childThread.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                demo.callbackFromC();
//            }
//        });

        findViewById(R.id.finishbutton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                android.os.Process.killProcess(android.os.Process.myPid());
            }
        });
    }



}
