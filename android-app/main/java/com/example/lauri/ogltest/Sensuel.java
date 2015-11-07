package com.example.lauri.ogltest;

/**
 * Created by lauri on 7.11.2015.
 */
public class Sensuel {
    private float[] or;

    public Sensuel() {
        or = new float[16];
        //or = 0.0f;
    }

    public void setOrientation(float[] orientation) {
        or = orientation;
    }

    public float[] getOrientation() {
        return or;
    }
}
