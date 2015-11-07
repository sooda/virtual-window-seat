package com.example.lauri.ogltest;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


//import android.opengl.EGLConfig;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.TriggerEvent;
import android.media.MediaPlayer;
import android.opengl.Matrix;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

//import javax.microedition.khronos.opengles.GL10;

/**
 * Created by lauri on 6.11.2015.
 */
public class AeroRenderer implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
    private Skybox skybox;
    private Triangle triangle;
    private final float[] mProjMatrix = new float[16];
    private final float[] mViewMatrix = new float[16];
    private final float[] mvp = new float[16];
    private Sensuel sens;
    private Context con;
    private MediaPlayer mMediaPlayer = null;
    private boolean updateSurface = false;
    private static int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
    private SurfaceTexture mSurface;


    //private SB sb;

    public AeroRenderer(Sensuel sensuel, Context context) {
        sens = sensuel;
        //skybox = new Skybox(/*context*/);
        con = context;

        //sb = new SB();
    }

    @Override
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        // Set the background frame color
        skybox = new Skybox(con, mMediaPlayer);
        skybox.mMediaPlayer = mMediaPlayer;
        GLES20.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        GLES20.glEnable(GLES20.GL_DEPTH_TEST);
        //triangle = new Triangle();
    }

    @Override
    public void onDrawFrame(GL10 unused) {

        /*synchronized(this) {
            if (updateSurface) {
                mSurface.updateTexImage();
                updateSurface = false;
            }
        }*/
        // Redraw background color
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        //float[] mvp = new float[16];

        Matrix.setIdentityM(mvp, 0);
        //Matrix.setLookAtM(mViewMatrix, 0, 1.5f, 1.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        //Matrix.setLookAtM(mViewMatrix, 0, 0, 0, -3, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

        //mViewMatrix = sens.getOrientation();
        Matrix.multiplyMM(mViewMatrix, 0, sens.getOrientation(), 0, mvp, 0);
        //Matrix.setIdentityM(mViewMatrix, 0);
        //Matrix.rotateM(mViewMatrix, 0, 1.0f, (float)Math.sin(sens.testx), 0.0f, 0.0f);
        //Matrix.setLookAtM(mViewMatrix, 0,
        //        0, 0, 0,
                //(float)Math.sin(System.nanoTime()*0.000000001f), 0f, (float)Math.cos(System.nanoTime()*0.000000001f),
        //        0, 1, 0,
        //        0f, 0.0f, 1.0f);
        Matrix.multiplyMM(mvp, 0, mProjMatrix, 0, mViewMatrix, 0);
        //GLES20.glClearColor(Math.abs(mvp[1]), (float)Math.sin(System.nanoTime()*0.000001f), 0.0f, 1.0f);

        //sb.draw(mvp);
        if(skybox != null)
            skybox.draw(mvp);
        //triangle.draw(mvp);
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
        float ratio = (float) width / height;
        //Matrix.frustumM(mProjMatrix, 0, -ratio, ratio, -1, 1, 0.05f, 2f);
        Matrix.perspectiveM(mProjMatrix, 0, 60.0f, ratio, 0.01f, 10.0f);
        //check matrices and gl errors
    }

    public void setMediaPlayer(MediaPlayer player) {
        mMediaPlayer = player;
        //skybox = new Skybox(con, mMediaPlayer);
        //skybox.mMediaPlayer = player;

    }

    synchronized public void onFrameAvailable(SurfaceTexture surface) {
        updateSurface = true;
    }

}
