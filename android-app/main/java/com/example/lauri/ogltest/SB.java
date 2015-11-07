package com.example.lauri.ogltest;

import android.opengl.GLES20;
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

    private static final String TAG = "Skybox";

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
                    //"uniform sampler2D texture;" +
                    "varying vec2 UV;" +
                    "void main() {" +
                    //"gl_FragColor = texture2D(texture, UV);}";
                    "gl_FragColor = vec4(0.5, UV.y, UV.x, 1.0);}";

    static float cVertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    public SB() {
        ByteBuffer cb = ByteBuffer.allocateDirect(cVertices.length * 4);
        cb.order(ByteOrder.nativeOrder());
        vertexBuffer = cb.asFloatBuffer();
        vertexBuffer.put(cVertices);
        vertexBuffer.position(0);

        int tmpVertShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(tmpVertShader, vertShaderText);
        GLES20.glCompileShader(tmpVertShader);

        int tmpFragShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(tmpFragShader, fragShaderText);
        GLES20.glCompileShader(tmpFragShader);

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, tmpVertShader);
        GLES20.glAttachShader(mProgram, tmpFragShader);
        GLES20.glLinkProgram(mProgram);
        GLES20.glUseProgram(mProgram);

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");

    }

    public void draw(float[] mvp) {
        GLES20.glUseProgram(mProgram);

        mMVPHandle = GLES20.glGetUniformLocation(mProgram, "vMVP");

        mPosHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        mUVHandle = GLES20.glGetAttribLocation(mProgram, "vUV");

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
