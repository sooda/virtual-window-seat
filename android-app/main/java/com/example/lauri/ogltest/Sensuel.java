package com.example.lauri.ogltest;

import android.opengl.Matrix;

/**
 * Created by lauri on 7.11.2015.
 */
public class Sensuel {
    private float[] or;
    public float testx;
    public float testy;

    public Sensuel() {
        or = new float[16];
        Matrix.setIdentityM(or, 0);
        testx = 0.0f;
        testy = 0.0f;
        //or = 0.0f;
    }

    public void setOrientation(float[] orientation) {
        or = orientation;
    }
    public void updateOrientation(float[] delta) {
        Matrix.multiplyMM(or, 0, delta, 0, or, 0);
    }

    public float[] getOrientation() {
        return or;
    }
}
