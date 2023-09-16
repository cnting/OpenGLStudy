package com.cnting.openglstudy;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
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
    private Bitmap liyinaiBitmap;
    private Bitmap liyinaiBitmap1;
    private Bitmap shiyuanmeiliBitmap;
    private Bitmap shiyuanmeiliBitmap1;

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
        liyinaiBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.liyingai);
        liyinaiBitmap1 = BitmapFactory.decodeResource(getResources(), R.drawable.liyingai1);
        shiyuanmeiliBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.shiyuanmeili);
        shiyuanmeiliBitmap1 = BitmapFactory.decodeResource(getResources(), R.drawable.shiyuanmeili2);
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
//        drawTriangleWithVBO(getHolder().getSurface());
//        drawTriangleWithEBO(getHolder().getSurface());
//        drawTriangleWithVAO(getHolder().getSurface());
//        drawTriangleWithVAOAndVBOAndEBO(getHolder().getSurface());
//        drawSquare(getHolder().getSurface());


//        drawTexture(shiyuanmeiliBitmap, getHolder().getSurface());
        drawTextureMixed(liyinaiBitmap1, shiyuanmeiliBitmap1, getHolder().getSurface());
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
     * 画三角形，使用VBO
     */
    private native void drawTriangleWithVBO(Surface surface);

    /**
     * 画三角形，使用EBO
     */
    private native void drawTriangleWithEBO(Surface surface);

    /**
     * 画三角形，使用VAO
     */
    private native void drawTriangleWithVAO(Surface surface);

    /**
     * 画三角形，使用VAO、VBO、EBO
     */
    private native void drawTriangleWithVAOAndVBOAndEBO(Surface surface);

    /**
     * 画正方形
     */
    private native void drawSquare(Surface surface);

    /**
     * 绘制纹理
     */
    private native void drawTexture(Bitmap bitmap, Surface surface);

    /**
     * 两个纹理混合
     */
    private native void drawTextureMixed(Bitmap bitmap1, Bitmap bitmap2, Surface surface);
}
