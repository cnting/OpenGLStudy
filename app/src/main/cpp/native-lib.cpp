#include <jni.h>
#include <string>
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "mylog.h"
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

GLint initSharder(const char *source, GLint type) {
    //创建着色器对象，type表示着色器类型。返回具有引用作用的整数
    GLint sh = glCreateShader(type);
    if (sh == 0) {
        LOGE("glCreateShader %d failed", type);
        return 0;
    }
    //加载着色器代码source
    glShaderSource(sh,
                   1, //shader数量
                   &source,
                   0 //代码长度，传0表示读到字符串结尾
    );
    //编译着色器对象
    glCompileShader(sh);

    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        LOGE("glCompileShader %d failed", type);
        LOGE("source %s", source);
        auto *infoLog = new GLchar[512];
        GLsizei length;
        glGetShaderInfoLog(sh, 512, &length, infoLog);

        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s", infoLog);
        return 0;
    }

    LOGE("glCompileShader %d success", type);
    return sh;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_openglstudy_YuvPlayer_drawTriangle(JNIEnv *env, jobject thiz, jobject surface) {
    /******** EGL配置start ********/
    //1.获取原始窗口
    ANativeWindow *nwin = ANativeWindow_fromSurface(env, surface);
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGE("egl display failed");
        return;
    }
    //2.初始化egl Display的连接，后两个参数是指针，分别用来返回EGL主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        LOGE("eglInitialize failed");
        return;
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
    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        LOGE("eglChooseConfig failed");
        return;
    }
    //创建EGLSurface，最后一个参数为属性信息，0表示不需要属性
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return;
    }
    //渲染上下文EGLContext关联的帧缓冲配置列表，EGL_CONTEXT_CLIENT_VERSION表示这里是配置EGLContext的版本
    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    //通过Display和上面获取到的的EGL帧缓存配置列表创建一个EGLContext， EGL_NO_CONTEXT表示不需要多个设备共享上下文
    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return;
    }
    //将EGLContext和当前线程以及draw和read的EGLSurface关联，关联后，当前线程就成为了OpenGL es的渲染线程
    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        LOGE("eglMakeCurrent failed");
        return;
    }
    /******** EGL配置end ******/

    /******** 加载着色器程序 start******/
    ////着色器对象：表示一段具体的着色器代码的抽象
    ////着色器程序：表示整个图形渲染管线的着色器程序集合

    //创建和编译顶点着色器、片段着色器对象
    GLint vsh = initShader(vertexSimpleShape, GL_VERTEX_SHADER);
    GLint fsh = initShader(fragSimpleShape, GL_FRAGMENT_SHADER);

    //创建着色器程序对象
    GLint program = glCreateProgram();
    if (program == 0) {
        LOGE("glCreateProgram failed");
        return;
    }
    //将着色器对象关联到着色器程序对象上
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接着色器程序
    glLinkProgram(program);

    //打印出链接异常信息
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0) {
        LOGE("glLinkProgram failed");
        return;
    }
    LOGE("glLinkProgram success");

    //使用着色器程序
    glUseProgram(program);
    /******** 加载着色器程序 end******/

    /******** 将数据传入图形渲染管线 start******/
    //定义顶点属性数组
    static float triangleVer[] = {
            0.8f, -0.8f, 0.0f,
            -0.8f, -0.8f, 0.0f,
            0.0f, 0.8f, 0.0f,
    };
    //指定接受三角形坐标的变量名
    GLuint apos = static_cast<GLuint>(glGetAttribLocation(program, "aPosition"));

    //告诉OpenGL如何解析传入的顶点属性数组
    //index：表示着色器中要接收数据的变量的引用（被in修饰的变量）
    //size：每个顶点属性需要用多个数组元素表示
    //type：每个数组元素的格式是什么
    //normalized：是否归一化，即是否需要将数据范围映射到-1到1区间
    //stride：步长，表示前一个顶点属性的起始位置到下一个顶点属性的起始位置在数组中有多少字节，如果传0，表示顶点属性数据是紧密挨着的。具体看这里：https://juejin.cn/post/7134356782452834334#comment
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, triangleVer);
    //打开着色器中的apos这个变量
    glEnableVertexAttribArray(apos);
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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    //窗口显示，交换双缓冲区
    eglSwapBuffers(display, winSurface);
    /******** 将图像渲染到屏幕 end******/


}