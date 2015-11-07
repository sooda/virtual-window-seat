package com.example.lauri.ogltest;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Created by lauri on 6.11.2015.
 */
public class Skybox implements SurfaceTexture.OnFrameAvailableListener {

    private FloatBuffer vertexBuffer;
    private int mPosHandle;
    private int mUVHandle;
    private static int mProgram;
    private int mMVPHandle;
    private int mTextureHandle;
    private int mTextureID;
    public MediaPlayer mMediaPlayer = null;
    private SurfaceTexture mSurface;
    private boolean updateSurface = false;

    private int skyboxTexture;

    private static final String TAG = "Skybox";
    private static int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

    private static final String vertShaderText =
            //"precision mediump float;" +
            "uniform mat4 vMVP;" +
            "attribute vec3 vPosition;" +
            "attribute vec2 vUV;" +
            "varying vec2 UV;" +
            "void main() {" +
            "gl_Position = vMVP * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);" +
            "UV = vUV;}";

    private static final String fragShaderText =
            "#extension GL_OES_EGL_image_external : require\n" +
            "precision mediump float;" +
            //"uniform sampler2D texture;" +
            "uniform samplerExternalOES sTexture;" +
            "varying vec2 UV;" +
            "void main() {" +
            "gl_FragColor = texture2D(sTexture, vec2(UV.x, 1.0-UV.y));}";
            //"gl_FragColor = vec4(0.5, UV.y, UV.x, 1.0);}";

    static float cVertices[] = {
        // Front
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,
        0.5f, -0.5f, -0.5f,  0.5f, 1.0f/3.0f,
        0.5f,  0.5f, -0.5f,  0.5f, 2.0f/3.0f,
        0.5f,  0.5f, -0.5f,  0.5f, 2.0f/3.0f,
        -0.5f,  0.5f, -0.5f,  0.25f, 2.0f/3.0f,
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,

        // Back
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f/3.0f,
        0.5f, -0.5f,  0.5f,  0.75f, 1.0f/3.0f,
        0.5f,  0.5f,  0.5f,  0.75f, 2.0f/3.0f,
        0.5f,  0.5f,  0.5f,  0.75f, 2.0f/3.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 2.0f/3.0f,
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f/3.0f,

        // Left
        -0.5f,  0.5f,  0.5f,  0.0f, 2.0f/3.0f,
        -0.5f,  0.5f, -0.5f,  0.25f, 2.0f/3.0f,
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f/3.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 2.0f/3.0f,

        // Right
        0.5f,  0.5f,  0.5f,  0.75f, 2.0f/3.0f,
        0.5f,  0.5f, -0.5f,  0.5f, 2.0f/3.0f,
        0.5f, -0.5f, -0.5f,  0.5f, 1.0f/3.0f,
        0.5f, -0.5f, -0.5f,  0.5f, 1.0f/3.0f,
        0.5f, -0.5f,  0.5f,  0.75f, 1.0f/3.0f,
        0.5f,  0.5f,  0.5f,  0.75f, 2.0f/3.0f,

        // Down
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,
        0.5f, -0.5f, -0.5f,  0.5f, 1.0f/3.0f,
        0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.25f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.25f, 1.0f/3.0f,

        // Up
        -0.5f,  0.5f, -0.5f,  0.25f, 2.0f/3.0f,
        0.5f,  0.5f, -0.5f,  0.5f, 2.0f/3.0f,
        0.5f,  0.5f,  0.5f,  0.5f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.5f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.25f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.25f, 2.0f/3.0f
    };

    public Skybox(Context context, MediaPlayer mp) {
        mMediaPlayer = mp;
        ByteBuffer cb = ByteBuffer.allocateDirect(cVertices.length * 4);
        cb.order(ByteOrder.nativeOrder());
        vertexBuffer = cb.asFloatBuffer();
        vertexBuffer.put(cVertices);
        vertexBuffer.position(0);

        int texture[] = new int[1];
        GLES20.glGenTextures(1, texture, 0);

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;

        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.sbox1, options);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        checkGlError("glBindTexture");
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);

        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);

        bitmap.recycle();

        skyboxTexture = texture[0];

        //SURFACE TEXTURE SHIT
        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        mTextureID = textures[0];
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureID);
        checkGlError("glBindTexture mTextureID");

        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);

        mSurface = new SurfaceTexture(mTextureID);
        mSurface.setOnFrameAvailableListener(this);

        Surface surface = new Surface(mSurface);
        mMediaPlayer.setSurface(surface);
        mMediaPlayer.setScreenOnWhilePlaying(true);
        surface.release();

        try {
            mMediaPlayer.prepare();
        } catch (IOException t) {
            Log.e(TAG, "media player prepare failed");
        }

        synchronized(this) {
            updateSurface = false;
        }

        mMediaPlayer.start();
        //SURFACE TEXTURE SHIT

        int tmpVertShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(tmpVertShader, vertShaderText);
        GLES20.glCompileShader(tmpVertShader);
        checkGlError("glCompileShader");

        int tmpFragShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(tmpFragShader, fragShaderText);
        GLES20.glCompileShader(tmpFragShader);
        checkGlError("glCompileShader");

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, tmpVertShader);
        checkGlError("glAttachShader");
        GLES20.glAttachShader(mProgram, tmpFragShader);
        checkGlError("glAttachShader");
        GLES20.glLinkProgram(mProgram);
        checkGlError("glLinkProgram");
        GLES20.glUseProgram(mProgram);
        checkGlError("glUseProgram");

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");
        Log.e(TAG, "mMVPHandle: " + mMVPHandle);
        checkGlError("glGetUniformLocation");
        //mTextureHandle = GLES20.glGetUniformLocation(mProgram, "texture");
        //checkGlError("glGetUniformLocation");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        checkGlError("glGetAttribLocation");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");
        checkGlError("glGetAttribLocation");

    }

    public void draw(float[] mvp) {

        synchronized(this) {
            if (updateSurface) {
                mSurface.updateTexImage();
                updateSurface = false;
            }
        }

        GLES20.glUseProgram(mProgram);
        checkGlError("glUseProgram");


        int stride = (2 + 3) * 4;
        //positions
        vertexBuffer.position(0);
        GLES20.glEnableVertexAttribArray(mPosHandle);
        checkGlError("glEnableVertexAttribArray");
        GLES20.glVertexAttribPointer(mPosHandle, 3, GLES20.GL_FLOAT, false, stride, vertexBuffer);
        checkGlError("glVertexAttribPointer");
        //UVs
        vertexBuffer.position(3);
        GLES20.glEnableVertexAttribArray(mUVHandle);
        GLES20.glVertexAttribPointer(mUVHandle, 2, GLES20.GL_FLOAT, false, stride, vertexBuffer);
        checkGlError("glVertexAttribPointer");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureID);

        //GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        checkGlError("glActiveTexture");
        //GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, skyboxTexture);
        //checkGlError("glBindTexture");
        //Log.e(TAG, "mTextureHandle: " + mTextureHandle);
        //GLES20.glUniform1i(mTextureHandle, 0);

        //checkGlError("glUniform1i");

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");
        //Log.e(TAG, "mMVPHandle: " + mMVPHandle);
        checkGlError("glGetUniformLocation");
        //mTextureHandle = GLES20.glGetUniformLocation(mProgram, "texture");
        //checkGlError("glGetUniformLocation");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        checkGlError("glGetAttribLocation");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");
        checkGlError("glGetAttribLocation");

        /*for(int i = 0; i< 16; i++) {
            Log.e(TAG, "mvp: " + mvp[i]);
        }*/
        GLES20.glUniformMatrix4fv(mMVPHandle, 1, false, mvp, 0);
        checkGlError("glUniformMatrix4fv");

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 36);
        checkGlError("glDrawArrays");
        GLES20.glDisableVertexAttribArray(mUVHandle);

    }
    public static void checkGlError(String glOperation) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, glOperation + ": glError " + error);
            throw new RuntimeException(glOperation + ": glError " + error);
        }
    }

    synchronized public void onFrameAvailable(SurfaceTexture surface) {
        updateSurface = true;
    }

}
