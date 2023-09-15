//
// Created by cnting on 2023/9/14.
//

#ifndef OPENGLSTUDY_SHADER_H
#define OPENGLSTUDY_SHADER_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>

class Shader {
private:
    GLint program;
    //顶点着色器
    const char *vertexShader;
    //片段着色器
    const char *fragmentShader;

    GLint initShader(const char *source, int type);

public:
    Shader(const char *vertexShader, const char *fragmentShader);

    int use();

    void release();

};


#endif //OPENGLSTUDY_SHADER_H
