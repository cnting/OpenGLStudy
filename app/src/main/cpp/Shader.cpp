//
// Created by cnting on 2023/9/14.
//

#include "Shader.h"
#include "mylog.h"

Shader::Shader(const char *vertexShader, const char *fragmentShader) {
    this->vertexShader = vertexShader;
    this->fragmentShader = fragmentShader;
}

GLint Shader::initShader(const char *source, int type) {
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

int Shader::use() {
    ////着色器对象：表示一段具体的着色器代码的抽象
    ////着色器程序：表示整个图形渲染管线的着色器程序集合

    //创建和编译顶点着色器、片段着色器对象
    GLint vsh = initShader(vertexShader, GL_VERTEX_SHADER);
    GLint fsh = initShader(fragmentShader, GL_FRAGMENT_SHADER);

    //创建着色器程序对象
    program = glCreateProgram();
    if (program == 0) {
        LOGE("glCreateProgram failed");
        return 0;
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
        return 0;
    }
    LOGE("glLinkProgram success");

    //使用着色器程序
    glUseProgram(program);
    return program;
}

void Shader::release() {
    glDeleteProgram(program);
}
