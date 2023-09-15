//
// Created by cnting on 2023/9/14.
//

#ifndef OPENGLSTUDY_FRAGMENTSHADER_H
#define OPENGLSTUDY_FRAGMENTSHADER_H

/**
 * 顶点着色器
 * gl_Position是OpenGL内置的
 */
static const char *vertexShader = "#version 300 es\n"
                                  "layout (location = 0) in vec4 aPosition;//输入的顶点坐标，会在程序指定将数据输入到该字段\n"
                                  "void main() {\n"
                                  "   //直接把传入的坐标值作为传入渲染管线。gl_Position是OpenGL内置的\n"
                                  "   gl_Position = aPosition;\n"
                                  "   gl_PointSize = 50.0;\n"
                                  "}";
/**
 * 片段着色器
 */
static const char *fragmentShader = "#version 300 es\n"
                                    "precision mediump float;\n"
                                    "out vec4 FragColor;\n"
                                    "void main() {\n"
                                    "  FragColor = vec4(1.0,0.5,0.5,1.0);\n"
                                    "}";
/**
 * 使用uniform传单一颜色值
 * uniform表示是渲染管线中的全局常量
 */
static const char *fragmentShaderWithUniform = "#version 300 es\n"
                                               "precision mediump float;\n"
                                               "//接收客户端程序传入的颜色值\n"
                                               "uniform vec4 uTextColor;\n"
                                               "out vec4 FragColor;\n"
                                               "void main() {\n"
                                               "  FragColor = uTextColor;\n"
                                               "}";
/**
 * 顶点着色器，带颜色属性
 */
static const char *vertexShaderWithColor = "#version 300 es\n"
                                           "layout (location = 0) in vec4 aPosition;\n"
                                           "layout (location = 1) in vec4 aColor;\n"
                                           "out vec4 vTextColor;\n"
                                           "void main() {\n"
                                           "   //直接把传入的坐标值作为传入渲染管线。gl_Position是OpenGL内置的\n"
                                           "   gl_Position = aPosition;\n"
                                           "   gl_PointSize = 50.0;\n"
                                           "   vTextColor = aColor;\n"
                                           "}\n";
/**
 * 片段着色器，带颜色属性
 */
static const char *fragmentShaderWithColor = "#version 300 es\n"
                                             "precision mediump float;\n"
                                             "out vec4 FragColor;\n"
                                             "in vec4 vTextColor;\n"
                                             "void main() {\n"
                                             "  FragColor = vTextColor;\n"
                                             "}\n";


#endif //OPENGLSTUDY_FRAGMENTSHADER_H
