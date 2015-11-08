package com.example.lauri.ogltest;

import android.content.Context;
import android.media.MediaPlayer;
import android.opengl.GLSurfaceView;

/**
 * Created by lauri on 6.11.2015.
 */
public class AeroSurfaceView extends GLSurfaceView {
    private final AeroRenderer mRenderer;
    private Sensuel sens;
    private MediaPlayer mMediaPlayer = null;

    public AeroSurfaceView(Context context, Sensuel sensuel, MediaPlayer player) {
        super(context);

        sens = sensuel;

        mMediaPlayer = player;

        setEGLContextClientVersion(2);
        mRenderer = new AeroRenderer(sens, context);
        mRenderer.setMediaPlayer(mMediaPlayer);

        setRenderer(mRenderer);
    }

    @Override
    public void onResume() {
        queueEvent(new Runnable(){
            public void run() {
                mRenderer.setMediaPlayer(mMediaPlayer);
            }});

        super.onResume();
    }
}
