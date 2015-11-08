package com.example.lauri.ogltest;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * Created by lauri on 6.11.2015.
 */
public class SB {

    private FloatBuffer vertexBuffer;
    private int mPosHandle;
    private int mUVHandle;
    private static int mProgram;
    private int mMVPHandle;
    private int mTextureHandle;
    private int skyboxTexture;

    private static final String TAG = "SB";

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
            "precision mediump float;" +
                    "uniform sampler2D texture;" +
                    "varying vec2 UV;" +
                    "void main() {" +
                    "gl_FragColor = texture2D(texture, vec2(UV.x, 1.0-UV.y));}";
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

    public SB(Context context) {
        ByteBuffer cb = ByteBuffer.allocateDirect(cVertices.length * 4);
        cb.order(ByteOrder.nativeOrder());
        vertexBuffer = cb.asFloatBuffer();
        vertexBuffer.put(cVertices);
        vertexBuffer.position(0);

        int texture[] = new int[1];
        GLES20.glGenTextures(1, texture, 0);

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;

        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.cloudysea, options);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        checkGlError("glBindTexture");
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);

        bitmap.recycle();

        skyboxTexture = texture[0];

        int tmpVertShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(tmpVertShader, vertShaderText);
        GLES20.glCompileShader(tmpVertShader);
        checkGlError("wat");

        int tmpFragShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(tmpFragShader, fragShaderText);
        GLES20.glCompileShader(tmpFragShader);
        checkGlError("wat");

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, tmpVertShader);
        GLES20.glAttachShader(mProgram, tmpFragShader);
        GLES20.glLinkProgram(mProgram);
        GLES20.glUseProgram(mProgram);
        checkGlError("wat");

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");
        checkGlError("wat");
        mTextureHandle = GLES20.glGetUniformLocation(mProgram, "texture");
        checkGlError("wat");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        checkGlError("wat");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");
        checkGlError("wat");

    }

    public void draw(float[] mvp) {
        GLES20.glUseProgram(mProgram);

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");
        checkGlError("wat");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        checkGlError("wat");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");
        checkGlError("wat");

        int stride = (2+3)*4;
        //positions
        vertexBuffer.position(0);
        GLES20.glEnableVertexAttribArray(mPosHandle);
        checkGlError("glEnableVertexAttribArray");
        GLES20.glVertexAttribPointer(mPosHandle, 3, GLES20.GL_FLOAT, false, stride, vertexBuffer);
        //UVs
        vertexBuffer.position(3);
        GLES20.glEnableVertexAttribArray(mUVHandle);
        GLES20.glVertexAttribPointer(mUVHandle, 2, GLES20.GL_FLOAT, false, stride, vertexBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, skyboxTexture);
        GLES20.glUniform1i(mTextureHandle, 0);

        GLES20.glUniformMatrix4fv(mMVPHandle, 1, false, mvp, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 36);
        GLES20.glDisableVertexAttribArray(mUVHandle);

    }
    public static void checkGlError(String glOperation) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, glOperation + ": glError " + error);
            throw new RuntimeException(glOperation + ": glError " + error);
        }
    }

}
