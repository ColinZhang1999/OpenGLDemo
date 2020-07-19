#include "GLShaderManager.h"
#include "GLTools.h"
#include <GLUT/GLUT.h>

//定义一个，着色管理器
GLShaderManager shaderManager;
//简单的批次容器，是GLTools的一个简单的容器类。
GLBatch triangleBatch;

// 正方形边长
GLfloat blockSize = 0.2f;
// 正方形的4个点坐标
GLfloat vVerts[] = {
        -blockSize,-blockSize,0.0f,
        blockSize,-blockSize,0.0f,
        blockSize,blockSize,0.0f,
        -blockSize,blockSize,0.0f
};

// 步长
GLfloat stepSize = 0.025f;
GLfloat xPos = 0;
GLfloat yPos = 0;

// 旋转角度
GLfloat rotate = 0;

/// 在窗口大小改变时，接收新的宽度&高度。
void changeSize(int w,int h) {
    glViewport(0, 0, w, h);
}

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    // 定义2个矩阵（平移矩阵，旋转矩阵）
    M3DMatrix44f mTransfromMatrix, mRotationMartix;

    // 平移(xPos, yPos)
    m3dTranslationMatrix44(mTransfromMatrix, xPos, yPos, 0.0f);

    // 旋转(rotate)
    m3dRotationMatrix44(mRotationMartix, m3dDegToRad(rotate), 0.0f, 0.0f, 1.0f);

    // 将旋转和移动的矩阵结果 合并到mFinalTransform （矩阵相乘）
    M3DMatrix44f mFinalTransform;
    m3dMatrixMultiply44(mFinalTransform, mTransfromMatrix, mRotationMartix);

    // 正方形填充颜色
    GLfloat vRed[] = {1.0, 0.0, 0.0, 1.0f};
    
    //将矩阵结果 提交给固定着色器（平面着色器）中绘制
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);

    //提交着色器
    triangleBatch.Draw();
    
    glutSwapBuffers();
}

void specialKeys(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP:
            yPos += stepSize;
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN:
            yPos -= stepSize;
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT:
            xPos -= stepSize;
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
            xPos += stepSize;
            glutPostRedisplay();
            break;
        case GLUT_KEY_PAGE_UP:
            rotate += 5;
            glutPostRedisplay();
            break;
        case GLUT_KEY_PAGE_DOWN:
            rotate -= 5;
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

void setupRC() {
    //设置清屏颜色（背景颜色，白色）
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        
    // 没有着色器，在OpenGL 核心框架中是无法进行任何渲染的。初始化一个渲染管理器。这里使用固定管线着色器
    shaderManager.InitializeStockShaders();
        
    //修改为GL_TRIANGLE_FAN ，4个顶点
    triangleBatch.Begin(GL_TRIANGLE_FAN, 4);
    triangleBatch.CopyVertexData3f(vVerts);
    triangleBatch.End();
}

int main(int argc,char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小、窗口标题
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle");
    
    // 注册窗口改变事件
    glutReshapeFunc(changeSize);
    // 注册显示函数，当需要重新绘制的时候，会调用
    glutDisplayFunc(renderScene);
    // 注册键盘输入事件
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
