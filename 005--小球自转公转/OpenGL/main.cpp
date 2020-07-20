#include "StopWatch.h"
#include "GLTools.h"
#include "GLFrustum.h"
#include "GLMatrixStack.h"
#include "GLShaderManager.h"
#include "GLGeometryTransform.h"
#include <GLUT/GLUT.h>

//定义一个，着色管理器
GLShaderManager shaderManager;

// 几何变换的管道
GLGeometryTransform    transformPipeline;
// 矩阵堆栈
GLMatrixStack          projectionMatrix;
// 矩阵堆栈
GLMatrixStack          modelViewMatrix;

// 投影矩阵
GLFrustum              viewFrustum;

// 视图参考帧
GLFrame                cameraFrame;
// 模型参考帧
GLFrame                objectFrame;

// 地板
GLBatch                floorBatch;
// 大球
GLTriangleBatch        torusBatch;
// 小球
GLTriangleBatch        sphereBatch;


// 绿色
GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
// 红色
GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
// 蓝色
GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };


GLfloat vTranparent[] = { 0.0f, 0.0f, 0.0f, .0f };

/// 在窗口大小改变时，接收新的宽度&高度。
void changeSize(int w,int h) {
//    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
    // 重新加载投影矩阵
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
}

//特殊键位处理（上、下、左、右移动）
void specialKeys(int key, int x, int y) {
    float linear = 0.1f;
    // 旋转5度
    float angular = float(m3dDegToRad(5.0f));
    
    switch (key) {
        case GLUT_KEY_UP:
            cameraFrame.MoveForward(linear);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN:
            cameraFrame.MoveForward(-linear);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT:
            cameraFrame.RotateWorld(angular, 0, 1, 0);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
             cameraFrame.RotateWorld(-angular, 0, 1, 0);
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //2.基于时间动画
    static CStopWatch rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 60.0f;
     
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

    modelViewMatrix.PushMatrix();
    // 模型变换
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
      
    // 绘制地板
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    floorBatch.Draw();
    
    // 绘制大球
    // 1. 获取光源位置
    M3DVector4f vLightPos = {0.0f,10.0f,5.0f,1.0f};
    // 平移
    modelViewMatrix.Translate(0.0f, 0.0f, -4.0f);
    // 旋转
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    
    // 划线
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.5f);
    
    // 8.指定合适的着色器(点光源着色器)
    shaderManager.UseStockShader(
                                 GLT_SHADER_POINT_LIGHT_DIFF,
                                 transformPipeline.GetModelViewMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightPos,
                                 vRed);
    torusBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    
    // 小球
    glLineWidth(2.0f);
    modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF,transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(),vLightPos, vBlue);
    sphereBatch.Draw();
       
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    
    
    glDisable(GL_DEPTH_TEST);
    
    // 进行缓冲区交换
    glutSwapBuffers();
    
    // 重新刷
    glutPostRedisplay();
}

void setupRC() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    shaderManager.InitializeStockShaders();

    // 设置变换管线以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

    //3. 设置地板顶点数据
    floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
    }
    floorBatch.End();
    
    // 4.设置大球模型
    // gltMakeSphere(torusBatch, 0.4f, 40, 80);
    gltMakeSphere(torusBatch, 0.4f, 20, 40);
    
    // 5. 设置小球球模型
    gltMakeSphere(sphereBatch, 0.2f, 8, 16);
    
}

int main(int argc,char *argv[]) {
    //初始化GLUT库,这个函数只是传说命令参数并且初始化glut库
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小、窗口标题
    glutInitWindowSize(800, 600);
    glutCreateWindow("几何图形");
    
    // 注册窗口改变事件
    glutReshapeFunc(changeSize);
    // 注册显示函数，当需要重新绘制的时候，会调用
    glutDisplayFunc(renderScene);
    // 特殊键位函数（上下左右）
    glutSpecialFunc(specialKeys);

    GLenum status = glewInit();
    if (GLEW_OK != status) {
        printf("GLEW Error:%s\n",glewGetErrorString(status));
        return 1;
    }
    
    //设置我们的渲染环境
    setupRC();
    
    // 开启事件循环
    glutMainLoop();

    return  0;
}
