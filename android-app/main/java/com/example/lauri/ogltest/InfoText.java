package com.example.lauri.ogltest;

import java.nio.FloatBuffer;

/**
 * Created by lauri on 8.11.2015.
 */
public class InfoText {
    private FloatBuffer vertexBuffer;
    private int mPosHandle;
    private int mUVHandle;
    private static int mProgram;
    private int mMVPHandle;
    private int mTextureHandle;

    private static final String vertShaderText =
        "uniform mat4 vMVP;" +
        "attribute vec3 vPosition;" +
        "attribute vec2 vUV;" +
        "varying vec2 UV;" +
        "void main() {" +
        "gl_Position = vMVP * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);" +
        "UV = vUV;}";

    private static final String fragShaderText =
        //"#extension GL_OES_EGL_image_external : require\n" +
        "precision mediump float;" +
        "uniform sampler2D texture;" +
        //"uniform samplerExternalOES sTexture;" +
        "varying vec2 UV;" +
        "void main() {" +
        "gl_FragColor = texture2D(texture, UV);}";
    //"gl_FragColor = vec4(0.5, UV.y, UV.x, 1.0);}";

    static float cVertices[] = {

    };
}
