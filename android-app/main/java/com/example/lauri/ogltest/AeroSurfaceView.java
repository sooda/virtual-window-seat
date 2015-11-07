package com.example.lauri.ogltest;

import android.content.Context;
import android.opengl.GLSurfaceView;

/**
 * Created by lauri on 6.11.2015.
 */
public class AeroSurfaceView extends GLSurfaceView {
    private final AeroRenderer mRenderer;
    private Sensuel sens;

    public AeroSurfaceView(Context context, Sensuel sensuel) {
        super(context);

        sens = sensuel;

        setEGLContextClientVersion(2);
        mRenderer = new AeroRenderer(sens, context);

        setRenderer(mRenderer);
    }
}
