package com.example.lauri.ogltest;

import android.app.ActionBar;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends AppCompatActivity implements SensorEventListener{

    private GLSurfaceView glview;
    private SensorManager mSensorManager;
    private Sensor mGravity;
    private Sensor mMagnetic;
    private float[] grav;
    private float[] mag;
    private Sensuel sens;
    private float[] rotation;

    @Override
    protected void onCreate(Bundle savedInstanceState)  {
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);

        if (Build.VERSION.SDK_INT < 16) {
            //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
            //        WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            View decorView = getWindow().getDecorView();
            int uiOpt = View.SYSTEM_UI_FLAG_FULLSCREEN;
            decorView.setSystemUiVisibility(uiOpt);
            ActionBar actionBar = getActionBar();
            if (actionBar != null)
                actionBar.hide();
        }

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mGravity = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mMagnetic = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);

        rotation = new float[16];
        grav = new float[3];
        mag = new float[3];

        sens = new Sensuel();

        glview = new AeroSurfaceView(this, sens);
        setContentView(glview);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(this, mGravity, SensorManager.SENSOR_DELAY_GAME);
        mSensorManager.registerListener(this, mMagnetic, SensorManager.SENSOR_DELAY_GAME);

    }

        @Override
    public void onAccuracyChanged(Sensor tmpSensor, int tmpInt) {

    }

    @Override
    public void onSensorChanged(SensorEvent event){
        if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
            grav = event.values;
        else if(event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD)
            mag = event.values;
        SensorManager.getRotationMatrix(rotation, new float[16], grav, mag);
        sens.setOrientation(rotation);
    }
}
