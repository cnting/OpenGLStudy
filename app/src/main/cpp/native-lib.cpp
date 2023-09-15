#include <jni.h>
#include <string>
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "mylog.h"
#include "Shader.h"
#include "FragmentShader.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>

/**
 * EGL是OpenGL ES与显示设备的桥梁，让OpenGL ES绘制的内容能够在呈现当前设备上
 * EGLDisplay：对实际显示设备的抽象
 *
 * EGLSurface：是一个存放「辅助缓冲」的图像数据（颜色缓冲、模板缓冲、深度缓冲）的容器。
 *
 * EGLContext：存储OpenGL ES绘图的一些状态信息。
 * EGLContext是线程相关的，一个线程只有绑定了一个EGLContext之后，才可以使用OpenGL es进行绘制。
 * 当然不同的EGLContext就维护了不同组的状态机。
 * 另外同一个EGLContext也可以被不同线程共享，但是不能同时被不同线程绑定。
 */

int eglConfig(JNIEnv *env, jobject surface, EGLDisplay *display, EGLSurface *winSurface) {
    //1.获取原始窗口
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    *display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (*display == EGL_NO_DISPLAY) {
        LOGE("egl display failed");
        return EGL_FALSE;
    }
    //2.初始化egl Display的连接，后两个参数是指针，分别用来返回EGL主次版本号
    if (EGL_TRUE != eglInitialize(*display, 0, 0)) {
        LOGE("eglInitialize failed");
        return EGL_FALSE;
    }
    //返回的EGL帧缓存配置
    EGLConfig eglConfig;
    //配置数量
    EGLint configNum;
    //期望的EGL帧缓存配置列表,配置为一个key一个value的形式
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };
    //返回一个和 期望的EGL帧缓存配置列表 匹配的EGL帧缓存配置列表，存储在eglConfig中
    if (EGL_TRUE != eglChooseConfig(*display, configSpec, &eglConfig, 1, &configNum)) {
        LOGE("eglChooseConfig failed");
        return EGL_FALSE;
    }
    //创建EGLSurface，最后一个参数为属性信息，0表示不需要属性
    *winSurface = eglCreateWindowSurface(*display, eglConfig, nwin, 0);
    if (*winSurface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return EGL_FALSE;
    }
    //渲染上下文EGLContext关联的帧缓冲配置列表，EGL_CONTEXT_CLIENT_VERSION表示这里是配置EGLContext的版本
    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    //通过Display和上面获取到的的EGL帧缓存配置列表创建一个EGLContext， EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(*display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return EGL_FALSE;
    }
    //将EGLContext和当前线程以及draw和read的EGLSurface关联，关联后，当前线程就成为了OpenGL es的渲染线程
    if (EGL_TRUE != eglMakeCurrent(*display, *winSurface, *winSurface, context)) {
        LOGE("eglMakeCurrent failed");
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

/**
 * 画三角形
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangle(JNIEnv *env, jobject thiz, jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShader, fragmentShader);
    shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    //三个点
    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };
    //六个点
//    static float triangleVer[] = {
//            0.8f, 0.8f, 0.0f,
//            0.0f, 0.8f, 0.0f,
//            0.4f, 0.4f, 0.0f,
//            -0.8f, 0.5f, 0.0f,
//            -0.4f, 0.8f, 0.0f,
//            -0.8f, 0.8f, 0.0f,
//    };

    //告诉OpenGL如何解析传入的顶点属性数组
    //index：表示着色器中要接收数据的变量的引用（被in修饰的变量）
    //size：每个顶点属性需要用多个数组元素表示
    //type：每个数组元素的格式是什么
    //normalized：是否归一化，即是否需要将数据范围映射到-1到1区间
    //stride：步长，表示前一个顶点属性的起始位置到下一个顶点属性的起始位置在数组中有多少字节，如果传0，表示顶点属性数据是紧密挨着的。具体看这里：https://juejin.cn/post/7134356782452834334#comment
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    //打开着色器中的apos这个变量
    glEnableVertexAttribArray(0);
    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    //将颜色缓冲清空为什么颜色，参数为对应的RGBA值。这里只是设置状态
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //指定要设置的缓冲区，就是真正将颜色缓冲设置为glClearColor指定的值
    //GL_COLOR_BUFFER_BIT:颜色缓冲
    //GL_DEPTH_BUFFER_BIT:深度缓冲
    //GL_STENCIL_BUFFER_BIT:模板缓冲
    glClear(GL_COLOR_BUFFER_BIT);


    //绘制三角形的指令。是真正启动整个图形渲染管线工作的按钮
    //第一个参数是图元类型，第二个参数是从传入的顶点属性数组的第几个元素开始绘制，第三个参数表示绘制多少个顶点属性数组元素。
    int count = sizeof(triangleVer) / sizeof(triangleVer[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);

    //绘制指令处理完成，窗口显示，交换前后缓冲区
    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();

}
/**
 * 画点
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawPoints(JNIEnv *env, jobject thiz, jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display = nullptr;
    EGLSurface winSurface = nullptr;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShader, fragmentShader);
    shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    //定义顶点属性数组
    static float pointsVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
            0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, pointsVer);
    glEnableVertexAttribArray(0);
    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, 4);

    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();
}
/**
 * 画线
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawLine(JNIEnv *env, jobject thiz, jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display = nullptr;
    EGLSurface winSurface = nullptr;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShader, fragmentShader);
    shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    //定义顶点属性数组
    static float lineVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
            0.4f, 0.8f, 0.0f,
    };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, lineVer);
    glEnableVertexAttribArray(0);
    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //指定线的宽度
    glLineWidth(30);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawSquare(JNIEnv *env, jobject thiz, jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShader, fragmentShader);
    shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    //画正方形，但是因为坐标是按照比例系数传的并且手机屏幕是长方形的，会展示成长方形，要用矩阵
    static float triangleVer[] = {
            0.8f, 0.8f, 0.0f,
            0.8f, -0.8f, 0.0f,
            -0.8f, 0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
    };
    int apos = 0;

    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    glEnableVertexAttribArray(apos);
    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();
}
/**
 * 使用uniform传递单一颜色值
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleUniform(JNIEnv *env, jobject thiz,
                                                          jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShader, fragmentShaderWithUniform);
    int program = shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    glEnableVertexAttribArray(0);

    //传入颜色
    static float color[] = {
            //RGBA
            0.0f, 1.0f, 0.0f, 1.0f
    };
    int colorLocation = glGetUniformLocation(program, "uTextColor");
    //v表示向量vec，基本格式是glUniform + n维向量 + 向量元素数据类型 + v
    glUniform4fv(colorLocation, 1, color);
    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();
}
/**
 *  画三角形，传多个颜色值
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleWithColor(JNIEnv *env, jobject thiz,
                                                            jobject surface) {
    /******** EGL配置start ********/
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    Shader shader(vertexShaderWithColor, fragmentShaderWithColor);
    shader.use();
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,  //顶点
            1.0, 0.0, 0.0,      //颜色

            -0.8f, -0.8f, 0.0f, //顶点
            0.0, 1.0, 0.0,      //颜色

            0.0f, 0.8f, 0.0f,  //顶点
            0.0, 0.0, 1.0,      //颜色
    };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, triangleVer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, triangleVer + 3);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /******** 将数据传入图形渲染管线 end******/

    /******** 将图像渲染到屏幕 start******/
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/

    shader.release();
}
/**
 * 使用VBO
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleWithVBO(JNIEnv *env, jobject thiz,
                                                          jobject surface) {
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }

    Shader shader(vertexShaderWithColor, fragmentShaderWithColor);
    shader.use();

    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,  //顶点
            1.0, 0.0, 0.0,      //颜色

            -0.8f, -0.8f, 0.0f, //顶点
            0.0, 1.0, 0.0,      //颜色

            0.0f, 0.8f, 0.0f,  //顶点
            0.0, 0.0, 1.0,      //颜色
    };

    ////CPU先把数组数据传送到GPU的vbo缓冲区

    //定义vbo的id数组，因为可能需要创建多个vbo
    unsigned int VBOs[1];
    //创建vbo，将创建好的vbo的id存放到VBOs数组中
    glGenBuffers(1, VBOs);
    //绑定vbo缓冲到上下文
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    //将顶点数据存入到vbo缓冲区
    //参数target:具体的Buffer Object种类
    //参数size:传入的数据长度
    //参数data:具体的数据指针
    //参数usage:数据的访问模式，常用的是指定修改频率模式，告诉OpenGL我们对数据的修改频率。
    //访问频率模式有：STREAM(几乎每次访问都被修改)、STATIC(只会修改一次)、DYNAMIC(数据会被多次修改)
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVer), triangleVer, GL_STATIC_DRAW);
    //指定如何解析顶点属性数组，注意这里最后一个参数传的不是原数组地址，而是在vbo缓冲区中的相对地址
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *) (3 * 4));
    //打开着色器中layout为0的输入变量
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //清屏
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //绘制三角形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    //交换前后缓冲区
    eglSwapBuffers(display, winSurface);

    //解绑VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //删除VBO，即清空缓冲区
    glDeleteBuffers(1, VBOs);

    shader.release();
}
/**
 * 使用EBO
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleWithEBO(JNIEnv *env, jobject thiz,
                                                          jobject surface) {
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }

    Shader shader(vertexShaderWithColor, fragmentShaderWithColor);
    shader.use();

    float vertices[] = {
            0.5f, 0.5f, 0.0f,   // 右上角
            1.0, 0.0, 0.0,//右上角颜色

            0.5f, -0.5f, 0.0f,  // 右下角
            0.0, 0.0, 1.0,//右下角颜色

            -0.5f, -0.5f, 0.0f, // 左下角
            0.0, 1.0, 0.0,//左下角颜色

            -0.5f, 0.5f, 0.0f,   // 左上角
            0.5, 0.5, 0.5,//左上角颜色
    };
    unsigned int indices[] = {
            0, 1, 3, // 第一个三角形
            1, 2, 3  // 第二个三角形
    };

    ////EBO缓存的是顶点的索引
    unsigned int EBO;
    //创建EBO缓冲对象
    glGenBuffers(1, &EBO);
    //绑定EBO缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //给EBO缓冲对象传入索引数据
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //解析顶点属性数据
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, vertices);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, vertices + 3);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //通过顶点索引绘制图元，注意这里已经绑定了EBO，所以最后一个参数传入的是数据在EBO中内存中的起始地址偏移量
    //第二个参数：表示要绘制多少个顶点
    //第三个参数：顶点索引的类型，必须是GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT其中一个种。
    //第四个参数：索引数组的指针
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
    eglSwapBuffers(display, winSurface);

    //解绑EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &EBO);

    shader.release();
}
/**
 * 使用VAO缓存VBO的顶点状态，画第一个三角形
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleWithVAO(JNIEnv *env, jobject thiz,
                                                          jobject surface) {
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }

    Shader shader(vertexShaderWithColor, fragmentShaderWithColor);
    shader.use();

    static float triangleVer[] = {
            0.0f, 0.8f, 0.0f,//顶点
            1.0, 0.0, 0.0,//颜色

            0.8f, 0.8f, 0.0f,//顶点
            0.0, 1.0, 0.0,//颜色

            0.0f, 0.0f, 0.0f,//顶点
            0.0, 0.0, 1.0,//颜色
    };
    static float triangleVer1[] = {
            0.0f, -0.8f, 0.0f,//顶点
            1.0, 0.0, 0.0,//颜色

            -0.8f, -0.8f, 0.0f,//顶点
            0.0, 1.0, 0.0,//颜色

            0.0f, 0.0f, 0.0f,//顶点
            0.0, 0.0, 1.0,//颜色
    };

    unsigned int VBOs[2];
    unsigned int VAOs[2];

    //创建2个VAO
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    //绑定第一个VAO，从此在解绑VAO之前的所有对VBOs[0]的操作都会记录在VAO内部
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVer), triangleVer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *) (3 * 4));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //解绑第一个VAO
    glBindVertexArray(0);


    //绑定第二个VAO
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVer1), triangleVer1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void *) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void *) (3 * 4));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //解绑第二个VAO
    glBindVertexArray(0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //绑定第一个VAO
    glBindVertexArray(VAOs[0]);
    //画第一个三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    eglSwapBuffers(display, winSurface);

    glDeleteBuffers(1, VBOs);
    glDeleteVertexArrays(2, VAOs);
    shader.release();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangleWithVAOAndVBOAndEBO(JNIEnv *env, jobject thiz,
                                                                      jobject surface) {
    EGLDisplay display;
    EGLSurface winSurface;
    if (EGL_TRUE != eglConfig(env, surface, &display, &winSurface)) {
        return;
    }

    Shader shader(vertexShaderWithColor, fragmentShaderWithColor);
    shader.use();

    float vertices[] = {
            0.5f, 0.5f, 0.0f,   // 右上角
            1.0, 0.0, 0.0,//右上角颜色

            0.5f, -0.5f, 0.0f,  // 右下角
            0.0, 0.0, 1.0,//右下角颜色

            -0.5f, -0.5f, 0.0f, // 左下角
            0.0, 1.0, 0.0,//左下角颜色

            -0.5f, 0.5f, 0.0f,   // 左上角
            0.5, 0.5, 0.5,//左上角颜色
    };
    unsigned int indices[] = {
            0, 1, 3, // 第一个三角形
            1, 2, 3  // 第二个三角形
    };

    unsigned int VAO;
    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    //依次绑定VAO、VBO、EBO，顺序不能错
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, vertices);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, vertices + 3);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //解绑顺序和绑定要相反
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //绑定VAO，状态已经被缓存，不用在绑定VBO和EBO
    glBindVertexArray(VAO);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
    eglSwapBuffers(display, winSurface);
    glBindVertexArray(0);

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    shader.release();
}