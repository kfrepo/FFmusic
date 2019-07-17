package com.wguet.ffmusic;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;

import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.wguet.myplayer.TimeInfoBean;
import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.listener.FFOnTimeInfoListener;
import com.wguet.myplayer.player.FFPlayer;
import com.wguet.myplayer.util.LogUtil;
import com.wguet.myplayer.util.TimeUtil;


public class MainActivity extends AppCompatActivity {

    private FFPlayer ffPlayer;

    private Button btStartPlay;
    private Button btPause;
    private Button btResume;
    private Button btFinish;
    private TextView tvPlayTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        viewInit();


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


    private void viewInit() {

        btStartPlay = (Button) findViewById(R.id.bt_start);
        btPause = (Button) findViewById(R.id.bt_pause);
        btResume = (Button) findViewById(R.id.bt_resume);
        btFinish= (Button) findViewById(R.id.bt_finish);
        tvPlayTime = (TextView) findViewById(R.id.tv_playtime);

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
                } else {
                    LogUtil.d("播放中...");
                }
            }
        });

        ffPlayer.setFfOnPauseResumeListener(new FFOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause) {
                    LogUtil.d("暂停中...");
                } else {
                    LogUtil.d("播放中...");
                }
            }
        });

        ffPlayer.setFfOnTimeInfoListener(new FFOnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfoBean timeInfo) {

                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfo;
                handler.sendMessage(message);
            }
        });

        btStartPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ffPlayer.setSource("/mnt/sdcard/Music/mydream.mp3");
                ffPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                ffPlayer.prepared();
            }
        });

        btPause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.pause();
            }
        });

        btResume.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.resume();
            }
        });

        btFinish.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                android.os.Process.killProcess(android.os.Process.myPid());
            }
        });
    }

    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message message){
            super.handleMessage(message);
            switch (message.what){
                case 1:{
                    TimeInfoBean time = (TimeInfoBean) message.obj;
                    tvPlayTime.setText(TimeUtil.secondsToDateFormat(time.getTotalTime(), time.getTotalTime())
                            + "/" + TimeUtil.secondsToDateFormat(time.getCurrentTime(), time.getTotalTime()));
                    break;
                }
                case 2:{
                    break;
                }
                default:{
                    break;
                }
            }

        }
    };


}
