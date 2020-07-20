#include "GLTools.h"
#include "GLFrustum.h"
#include "GLMatrixStack.h"
#include "GLShaderManager.h"
#include "GLGeometryTransform.h"
#include <GLUT/GLUT.h>

// 着色管理器
GLShaderManager shaderManager;

// 几何变换的管道
GLGeometryTransform    transformPipeline;

// 矩阵堆栈，用于设置投影矩阵
GLMatrixStack          projectionMatrix;

// 矩阵堆栈，用于设置视图矩阵，模型矩阵，
GLMatrixStack          modelViewMatrix;

// 投影矩阵
GLFrustum              viewFrustum;

// 参考帧，用于生成视图变换矩阵，用于调整观察者的位置
// （通过moveForward调整观察者在z轴移动，默认的朝向是-z轴，所以向屏幕里面移动传正数值，向屏幕外即+z轴，需要传负数值）
GLFrame                cameraFrame;
// 参考帧，用于生成模型变换矩阵（平移旋转缩放）
GLFrame                objectFrame;

// 容器类（7种不同的图元对应7种容器对象）
GLBatch                pointBatch;
GLBatch                lineBatch;
GLBatch                lineStripBatch;
GLBatch                lineLoopBatch;
GLBatch                triangleBatch;
GLBatch                triangleStripBatch;
GLBatch                triangleFanBatch;

// 填充颜色
GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
// 边框颜色
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// 跟踪效果步骤，按空格切换
int nStep = 0;

/// 在窗口大小改变时，接收新的宽度&高度。
void changeSize(int w,int h) {
    // 修改视口
    glViewport(0, 0, w, h);
    
    // 设置透视投影
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    // 重新加载投影矩阵到矩阵堆栈projectionMatrix
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
}

//特殊键位处理（上、下、左、右移动）
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN:
            objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT:
            objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
            objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

// 空格切换图元
void keyPressFunc(unsigned char key, int x, int y) {
    if (key == 32) {
        nStep++;
        if(nStep > 6) {
            nStep = 0;
        }
    }
    glutPostRedisplay();
}

// 画边框和颜色
void drawWireFramedBatch(GLBatch* pBatch) {
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    /*-----------填充颜色部分-------------------*/
    // 使用平面着色器，给图形填充绿色
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    /*-----------边框部分-------------------*/
    // 开启多边形偏移
    glPolygonOffset(-1.0f, -1.0f);
    glEnable(GL_POLYGON_OFFSET_LINE);
    
    // 开启反锯齿
    glEnable(GL_LINE_SMOOTH);
    
    // 开启混合
    glEnable(GL_BLEND);
    // 混合方法
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //绘制线框几何黑色版 三种模式，实心，边框，点，可以作用在正面，背面，或者两面
    //通过调用glPolygonMode将多边形正面或者背面设为线框模式，实现线框渲染
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //设置线条宽度
    glLineWidth(2.5f);
    
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
         --transformPipeline.GetModelViewProjectionMatrix() 获取的
          GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();

    // 复原原本的设置
    //通过调用glPolygonMode将多边形正面或者背面设为全部填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_DEPTH_TEST);
}



void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    // 调用顶部载入单元矩阵
    modelViewMatrix.LoadIdentity();
    
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
     
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mCamera);
    
    M3DMatrix44f mObjectFrame;
    //只要使用 GetMatrix 函数就可以获取矩阵堆栈顶部的值，这个函数可以进行2次重载。用来使用GLShaderManager 的使用。或者是获取顶部矩阵的顶点副本数据
    objectFrame.GetMatrix(mObjectFrame);
     
    //矩阵乘以矩阵堆栈的顶部矩阵，相乘的结果随后简存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    // 传递矩阵和颜色到平面着色器
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    
    switch(nStep) {
        case 0:
            //设置点的大小
            glPointSize(4.0f);
            pointBatch.Draw();
            glPointSize(1.0f);
            break;
        case 1:
            //设置线的宽度
            glLineWidth(2.0f);
            lineBatch.Draw();
            glLineWidth(1.0f);
            break;
         case 2:
             glLineWidth(2.0f);
             lineStripBatch.Draw();
             glLineWidth(1.0f);
             break;
         case 3:
             glLineWidth(2.0f);
             lineLoopBatch.Draw();
             glLineWidth(1.0f);
             break;
         case 4:
             drawWireFramedBatch(&triangleBatch);
             break;
         case 5:
             drawWireFramedBatch(&triangleStripBatch);
             break;
         case 6:
             drawWireFramedBatch(&triangleFanBatch);
             break;
        default:
            break;
     }
    
     // 绘制完后，还原矩阵堆栈（单位矩阵）
     modelViewMatrix.PopMatrix();
    
     // 进行缓冲区交换
     glutSwapBuffers();
}

void setupRC() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    shaderManager.InitializeStockShaders();

    // 设置变换管线以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
    cameraFrame.MoveForward(-15.0f);
    
    // 定义三个点
    GLfloat vCoast[] = {
        3,3,0,
        0,3,0,
        3,0,0
    };
    
    // 画点
    pointBatch.Begin(GL_POINTS, 3);
    pointBatch.CopyVertexData3f(vCoast);
    pointBatch.End();
    
    // 画线
    lineBatch.Begin(GL_LINES, 3);
    lineBatch.CopyVertexData3f(vCoast);
    lineBatch.End();
    
    // 画连续线段
    lineStripBatch.Begin(GL_LINE_STRIP, 3);
    lineStripBatch.CopyVertexData3f(vCoast);
    lineStripBatch.End();
    
    // 画闭合线段
    lineLoopBatch.Begin(GL_LINE_LOOP, 3);
    lineLoopBatch.CopyVertexData3f(vCoast);
    lineLoopBatch.End();
    
    // 3个三角形，构成金字塔形状
    GLfloat vPyramid[12][3] = {
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,

        2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,

        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,

        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f
    };
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();

    // 三角形扇形--六边形
    GLfloat vPoints[100][3];
    int nVerts = 0;
    // 半径
    GLfloat r = 3.0f;
    // 原点(x,y,z) = (0,0,0);
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;
    
    // M3D_2PI 就是2Pi 的意思，就一个圆的意思。 绘制圆形
    for (GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        // 数组下标自增（每自增1次就表示一个顶点）
        nVerts++;
        /*
         弧长=半径*角度,这里的角度是弧度制,不是平时的角度制
         既然知道了cos值,那么角度=arccos,求一个反三角函数就行了
         */
        // x点坐标 cos(angle) * 半径
        vPoints[nVerts][0] = float(cos(angle)) * r;
        // y点坐标 sin(angle) * 半径
        vPoints[nVerts][1] = float(sin(angle)) * r;
        // z点的坐标
        vPoints[nVerts][2] = -0.5f;
    }

    // 结束扇形 前面一共绘制7个顶点（包括圆心）
    // 添加闭合的终点
    // 课程添加演示：屏蔽177-180行代码，并把绘制节点改为7.则三角形扇形是无法闭合的。
    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0;
    vPoints[nVerts][2] = 0.0f;
    
    // 加载！GL_TRIANGLE_FAN 以一个圆心为中心呈扇形排列，共用相邻顶点的一组三角形
    triangleFanBatch.Begin(GL_TRIANGLE_FAN, 8);
    triangleFanBatch.CopyVertexData3f(vPoints);
    triangleFanBatch.End();
       
    // 三角形条带，一个小环或圆柱段
    // 顶点下标
    int iCounter = 0;
    // 半径
    GLfloat radius = 3.0f;
    // 从0度~360度，以0.3弧度为步长
    for (GLfloat angle = 0.0f; angle <= (2.0f*M3D_PI); angle += 0.3f) {
        //或许圆形的顶点的X,Y
        GLfloat x = radius * sin(angle);
        GLfloat y = radius * cos(angle);
        
        //绘制2个三角形（他们的x,y顶点一样，只是z点不一样）
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5;
        iCounter++;
        
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5;
        iCounter++;
    }
    
    //结束循环，在循环位置生成2个三角形
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5;
    iCounter++;
    
    vPoints[iCounter][0] = vPoints[1][0];
    vPoints[iCounter][1] = vPoints[1][1];
    vPoints[iCounter][2] = 0.5;
    iCounter++;
    
    // GL_TRIANGLE_STRIP 共用一个条带（strip）上的顶点的一组三角形
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();
}

int main(int argc,char *argv[]) {
    //初始化GLUT库,这个函数只是传说命令参数并且初始化glut库
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小、窗口标题
    glutInitWindowSize(800, 600);
    glutCreateWindow("图元绘制");
    
    
    // 注册窗口改变事件
    glutReshapeFunc(changeSize);
    // 注册显示函数，当需要重新绘制的时候，会调用
    glutDisplayFunc(renderScene);
    // 点击空格时，调用的函数
    glutKeyboardFunc(keyPressFunc);
    // 特殊键位函数（上下左右）
    glutSpecialFunc(specialKeys);

    GLenum status = glewInit();
    if (GLEW_OK != status) {
        printf("GLEW Error:%s\n",glewGetErrorString(status));
        return 1;
    }
    
    //设置渲染环境
    setupRC();
    
    // 开启事件循环
    glutMainLoop();

    return  0;
}
