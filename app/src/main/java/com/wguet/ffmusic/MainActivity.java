package com.wguet.ffmusic;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;

import androidx.appcompat.app.AppCompatActivity;

import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.wguet.myplayer.TimeInfoBean;
import com.wguet.myplayer.listener.FFOnCompleteListener;
import com.wguet.myplayer.listener.FFOnErrorListener;
import com.wguet.myplayer.listener.FFOnLoadListener;
import com.wguet.myplayer.listener.FFOnPauseResumeListener;
import com.wguet.myplayer.listener.FFOnPreparedListener;
import com.wguet.myplayer.listener.FFOnTimeInfoListener;
import com.wguet.myplayer.listener.FFOnVolumeDBListener;
import com.wguet.myplayer.player.FFPlayer;
import com.wguet.myplayer.util.LogUtil;
import com.wguet.myplayer.util.TimeUtil;

import java.io.File;
import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.Date;


/**
 * @author wangmh
 *
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private FFPlayer ffPlayer;

    private Button btStartPlay;
    private Button btPause;
    private Button btResume;
    private Button btStop;
    private Button btNext;

    private Button btFinish;
    private TextView tvPlayTime;

    private SeekBar timeSeek;

    private TextView volumeTv;
    private SeekBar volumeSeek;

    private Button leftBt, rightBt, stereoBt;

    private RadioGroup speedRadioGroup, pitchRadioGroup;

    private Button audioStartBt, audioStopBt;

    /**
     * 是否正在滑动进度条
     */
    private boolean isTimeSeek = false;

    private int currentTime;
    private int currentVolume;

    private MyHandler myHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initCallBack();

        initView();
        myHandler = new MyHandler(this);

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


    private void initView() {

        btStartPlay = (Button) findViewById(R.id.bt_start);
        btPause = (Button) findViewById(R.id.bt_pause);
        btResume = (Button) findViewById(R.id.bt_resume);
        btStop = (Button) findViewById(R.id.bt_stop);
        btFinish= (Button) findViewById(R.id.bt_finish);
        tvPlayTime = (TextView) findViewById(R.id.tv_playtime);
        btNext = (Button) findViewById(R.id.bt_next);

        timeSeek = findViewById(R.id.audio_seek_sb);

        volumeTv = findViewById(R.id.audio_volume_tv);
        volumeSeek = findViewById(R.id.audio_volume_sb);

        leftBt = findViewById(R.id.sound_channel_left_bt);
        rightBt = findViewById(R.id.sound_channel_right_bt);
        stereoBt = findViewById(R.id.sound_channel_stereo_bt);

        speedRadioGroup = findViewById(R.id.sound_speed_rg);
        pitchRadioGroup = findViewById(R.id.sound_pitch_rg);

        audioStartBt = findViewById(R.id.audio_start_bt);
        audioStopBt = findViewById(R.id.audio_stop_bt);

        //开始
        btStartPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //ffPlayer.setSource("/mnt/sdcard/Music/mydream.mp3");
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

        btStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.stop();
            }
        });

        //退出应用
        btFinish.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                android.os.Process.killProcess(android.os.Process.myPid());
            }
        });

        //下一首
        btNext.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                ffPlayer.playNext("http://ngcdn001.cnr.cn/live/zgzs/index.m3u8");
            }
        });

        timeSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                currentTime = ffPlayer.getDuration() * progress /100;
                //LogUtil.d(TAG, "timeSeek progress " + progress + " ffPlayer.getDuration() " + ffPlayer.getDuration() + " currentTime " + currentTime);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTimeSeek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                ffPlayer.seek(currentTime);
                isTimeSeek = false;
            }
        });

        volumeSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                ffPlayer.setVolume(progress);
                volumeTv.setText("音量 " + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        //声道控制
        leftBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.setMute(1);
            }
        });
        rightBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.setMute(0);
            }
        });
        stereoBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.setMute(2);
            }
        });

        speedRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                RadioButton radioButton = findViewById(checkedId);
                ffPlayer.setSpeed(Float.parseFloat(radioButton.getText().toString()));
            }
        });

        pitchRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                RadioButton radioButton = findViewById(checkedId);
                ffPlayer.setPitch(Float.parseFloat(radioButton.getText().toString()));
            }
        });

        File file = new File(Environment.getExternalStorageDirectory(), "/ffmusic");
        file.mkdir();
        audioStartBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmmss");
                String time = format.format(new Date());
                File file = new File(Environment.getExternalStorageDirectory(), "ffmusic/" + time + ".AAC");
                LogUtil.d(TAG, "audio record " + file.getAbsolutePath());
                ffPlayer.audioStartRecord(file);
            }
        });

        audioStopBt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ffPlayer.audioStopRecord();
            }
        });
    }


    private void initCallBack(){

        ffPlayer = new FFPlayer();
        ffPlayer.setPreparedListener(new FFOnPreparedListener() {
            @Override
            public void onPrepared() {
                LogUtil.d(TAG, "准备好 开始播放音频！");
                ffPlayer.start();
            }
        });

        ffPlayer.setFfOnLoadListener(new FFOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if(load) {
                    LogUtil.d(TAG, "加载中...");
                } else {
                    LogUtil.d(TAG, "播放中...");
                }
            }
        });

        ffPlayer.setFfOnPauseResumeListener(new FFOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause) {
                    LogUtil.d(TAG, "暂停中...");
                } else {
                    LogUtil.d(TAG, "播放中...");
                }
            }
        });

        ffPlayer.setFfOnTimeInfoListener(new FFOnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfoBean timeInfo) {

                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfo;
                myHandler.sendMessage(message);
            }
        });

        ffPlayer.setFfOnErrorListener(new FFOnErrorListener() {
            @Override
            public void onError(int code, final String msg) {
                LogUtil.e(TAG, "code:" + code + " msg:" +msg);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, msg, Toast.LENGTH_LONG).show();
                    }
                });
            }
        });

        ffPlayer.setFfOnCompleteListener(new FFOnCompleteListener() {
            @Override
            public void onComplete() {
                LogUtil.d(TAG, "播放结束!");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tvPlayTime.setText("00/00 播放结束");
                    }
                });
            }
        });

        //DB
        ffPlayer.setFfOnVolumeDBListener(new FFOnVolumeDBListener() {
            @Override
            public void onDbValue(int db) {
//                LogUtil.d(TAG, "onDbValue " + db);
            }
        });

    }

    private static class MyHandler extends Handler {

        private WeakReference<MainActivity> mInstance;
        public MyHandler(MainActivity activity){
            this.mInstance = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message message){
            super.handleMessage(message);

            MainActivity activity = mInstance == null ? null : mInstance.get();
            if (activity == null || activity.isFinishing()) {
                return;
            }
            switch (message.what){
                case 1:{
                    TimeInfoBean time = (TimeInfoBean) message.obj;
                    activity.tvPlayTime.setText(TimeUtil.secondsToDateFormat(time.getTotalTime(), time.getTotalTime())
                            + "/" + TimeUtil.secondsToDateFormat(time.getCurrentTime(), time.getTotalTime()));
                    int progress = time.getCurrentTime()*100/time.getTotalTime();
                    activity.timeSeek.setProgress(progress);
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }



}
