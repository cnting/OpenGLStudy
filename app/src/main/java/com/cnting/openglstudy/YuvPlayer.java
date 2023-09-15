package com.cnting.openglstudy;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by cnting on 2023/9/14
 */
public class YuvPlayer extends GLSurfaceView implements Runnable {
    private int surfaceWidth, surfaceHeight;

    public YuvPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
        setRenderer(new Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d("===>", "render onSurfaceCreated");
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                surfaceWidth = width;
                surfaceHeight = height;
            }

            @Override
            public void onDrawFrame(GL10 gl) {
            }
        });
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        super.surfaceCreated(holder);
        Log.d("===>", "surfaceCreated");
        new Thread(this).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        super.surfaceChanged(holder, format, w, h);
        surfaceWidth = w;
        surfaceHeight = h;
    }


    @Override
    public void run() {
//        drawPoints(getHolder().getSurface());
//        drawLine(getHolder().getSurface());
//        drawTriangle(getHolder().getSurface());
//        drawTriangleUniform(getHolder().getSurface());
//        drawTriangleWithColor(getHolder().getSurface());
        drawTriangleWithBufferObj(getHolder().getSurface());
//        drawSquare(getHolder().getSurface());
    }


    /**
     * 画点
     */
    private native void drawPoints(Surface surface);

    /**
     * 画线
     */
    private native void drawLine(Surface surface);

    /**
     * 画三角形
     */
    private native void drawTriangle(Surface surface);

    /**
     * 画三角形，使用uniform传递单一颜色值
     */
    private native void drawTriangleUniform(Surface surface);

    /**
     * 画三角形，传多个颜色值
     */
    private native void drawTriangleWithColor(Surface surface);

    /**
     * 画三角形，使用BufferObject
     */
    private native void drawTriangleWithBufferObj(Surface surface);

    /**
     * 画正方形
     */
    private native void drawSquare(Surface surface);
}
