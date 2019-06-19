package com.wguet.ffmusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.wguet.myplayer.Demo;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.player.FFPlayer;
import com.wguet.myplayer.util.LogUtil;

public class MainActivity extends AppCompatActivity {


    private FFPlayer ffPlayer;
    private Button childThread;
    private Button mainThread;
    private Button startPlay;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mainThread = (Button) findViewById(R.id.mainThread);
        childThread = (Button) findViewById(R.id.childThread);
        startPlay = (Button) findViewById(R.id.buttons);

        ffPlayer = new FFPlayer();

        ffPlayer.setPreparedListener(new FFOnPreparedListener() {
            @Override
            public void onPrepared() {
                LogUtil.d("准备好 开始播放音频！");
                ffPlayer.start();
            }
        });


        startPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

//                ffPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                ffPlayer.setSource("/mnt/sdcard/Music/mydream.mp3");
                ffPlayer.prepared();
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


    }



}
