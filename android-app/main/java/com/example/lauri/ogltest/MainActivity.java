package com.example.lauri.ogltest;

import android.app.ActionBar;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.media.MediaPlayer;

public class MainActivity extends AppCompatActivity implements SensorEventListener{

    private GLSurfaceView glview;
    private SensorManager mSensorManager;
    private Sensor mGravity;
    private Sensor mMagnetic;
    private Sensor mGyro;
    private float[] grav;
    private float[] mag;
    private Sensuel sens;
    private float[] rotation;

    private static final float NS2S = 1.0f / 1000000000.0f;
    private final float[] deltaRotationVector = new float[4];
    private float timestamp;


    private MediaPlayer mMediaPlayer = null;

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
        //mGravity = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        //mMagnetic = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        mGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);

        rotation = new float[16];
        grav = new float[3];
        mag = new float[3];

        sens = new Sensuel();

        mMediaPlayer = new MediaPlayer();

        try {

            AssetFileDescriptor afd = getResources().openRawResourceFd(R.raw.testvideo);
            mMediaPlayer.setDataSource(
                    afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
            //mMediaPlayer.setDataSource(afd.getFileDescriptor());
            afd.close();
        } catch (Exception e) {
            Log.e("Shit Activity", e.getMessage(), e);
        }

        glview = new AeroSurfaceView(this, sens, mMediaPlayer);
        setContentView(glview);
    }

    @Override
    protected void onResume() {
        super.onResume();
        //mSensorManager.registerListener(this, mGravity, SensorManager.SENSOR_DELAY_GAME);
        //mSensorManager.registerListener(this, mMagnetic, SensorManager.SENSOR_DELAY_GAME);
        mSensorManager.registerListener(this, mGyro, SensorManager.SENSOR_DELAY_GAME);

    }

        @Override
    public void onAccuracyChanged(Sensor tmpSensor, int tmpInt) {

    }

    @Override
    public void onSensorChanged(SensorEvent event){
        /*if(event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            grav = event.values;
            sens.testx = grav[1];
            //sens.testy = grav[1];
        }

        if(event.sensor.getType() == Sensor.TYPE_GRAVITY) {
            float[] asdf = event.values;
            //sens.testx = asdf[2];
            //sens.testy = asdf[2];
        }

        else if(event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD)
            mag = event.values;
        SensorManager.getRotationMatrix(rotation, new float[16], grav, mag);
        sens.setOrientation(rotation);
        float[] ori = new float[3];

        SensorManager.getOrientation(rotation, ori);
        sens.testx = ori[0];*/
        if (timestamp != 0) {
            final float dT = (event.timestamp - timestamp) * NS2S;
            // Axis of the rotation sample, not normalized yet.
            float axisY = event.values[0];
            float axisX = -event.values[1];
            float axisZ = event.values[2];

            //Log.e("Axiz", "Axisx: " + axisX + " Axisy: " + axisY + " Axisz: " + axisZ);

            // Calculate the angular speed of the sample
            float omegaMagnitude = (float)Math.sqrt(axisX * axisX + axisY * axisY + axisZ * axisZ);

            // Normalize the rotation vector if it's big enough to get the axis
            // (that is, EPSILON should represent your maximum allowable margin of error)
            if (omegaMagnitude > 0.01f) {
                axisX /= omegaMagnitude;
                axisY /= omegaMagnitude;
                axisZ /= omegaMagnitude;
            }

            // Integrate around this axis with the angular speed by the timestep
            // in order to get a delta rotation from this sample over the timestep
            // We will convert this axis-angle representation of the delta rotation
            // into a quaternion before turning it into the rotation matrix.
            float thetaOverTwo = omegaMagnitude * dT / 2.0f;
            float sinThetaOverTwo = (float)Math.sin(thetaOverTwo);
            float cosThetaOverTwo = (float)Math.cos(thetaOverTwo);
            deltaRotationVector[0] = sinThetaOverTwo * axisX;
            deltaRotationVector[1] = sinThetaOverTwo * axisY;
            deltaRotationVector[2] = sinThetaOverTwo * axisZ;
            deltaRotationVector[3] = cosThetaOverTwo;
        }
        timestamp = event.timestamp;
        float[] deltaRotationMatrix = new float[16];
        SensorManager.getRotationMatrixFromVector(deltaRotationMatrix, deltaRotationVector);
        sens.updateOrientation(deltaRotationMatrix);
        // User code should concatenate the delta rotation we computed with the current rotation
        // in order to get the updated rotation.
        // rotationCurrent = rotationCurrent * deltaRotationMatrix;
    }


}
