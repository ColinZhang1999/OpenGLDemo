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

//容器类
// 球
GLTriangleBatch     sphereBatch;
// 环
GLTriangleBatch     torusBatch;
// 圆柱
GLTriangleBatch     cylinderBatch;
// 锥
GLTriangleBatch     coneBatch;
// 磁盘
GLTriangleBatch     diskBatch;
// 立方体
GLBatch             cubeBatch;

// 填充颜色
GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
// 边框颜色
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// 跟踪效果步骤
int nStep = 0;

/// 在窗口大小改变时，接收新的宽度&高度。
void changeSize(int w,int h) {
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    // 重新加载投影矩阵
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //调用顶部载入单元矩阵
    modelViewMatrix.LoadIdentity();
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
        if(nStep > 5) {
            nStep = 0;
        }
    }
    glutPostRedisplay();
}

// 画边框和颜色
void drawWireFramedBatch(GLBatchBase* pBatch) {
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 画填充颜色
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    // 画边框
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
    
    //压栈
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
     
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    //5.判断你目前是绘制第几个图形
    switch(nStep) {
        case 0:
            drawWireFramedBatch(&sphereBatch);
            break;
        case 1:
            drawWireFramedBatch(&torusBatch);
            break;
        case 2:
            drawWireFramedBatch(&cylinderBatch);
            break;
        case 3:
            drawWireFramedBatch(&coneBatch);
            break;
        case 4:
            drawWireFramedBatch(&diskBatch);
            break;
        case 5:
            drawWireFramedBatch(&cubeBatch);
            break;
    }
    
     //还原到以前的模型视图矩阵（单位矩阵）
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

    //4.利用三角形批次类构造图形对象
    // 球
    /*
      gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);
     参数1：sphereBatch，三角形批次类对象
     参数2：fRadius，球体半径
     参数3：iSlices，从球体底部堆叠到顶部的三角形带的数量；其实球体是一圈一圈三角形带组成
     参数4：iStacks，围绕球体一圈排列的三角形对数
     
     建议：一个对称性较好的球体的片段数量是堆叠数量的2倍，就是iStacks = 2 * iSlices;
     绘制球体都是围绕Z轴，这样+z就是球体的顶点，-z就是球体的底部。
     */
    gltMakeSphere(sphereBatch, 3.0, 10, 20);
    
    // 环面
    /*
     gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     参数1：torusBatch，三角形批次类对象
     参数2：majorRadius,甜甜圈中心到外边缘的半径
     参数3：minorRadius,甜甜圈中心到内边缘的半径
     参数4：numMajor,沿着主半径的三角形数量
     参数5：numMinor,沿着内部较小半径的三角形数量
     */
    gltMakeTorus(torusBatch, 3.0f, 0.75f, 15, 15);
    
    // 圆柱
    /*
     void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     参数1：cylinderBatch，三角形批次类对象
     参数2：baseRadius,底部半径
     参数3：topRadius,头部半径
     参数4：fLength,圆形长度
     参数5：numSlices,围绕Z轴的三角形对的数量
     参数6：numStacks,圆柱底部堆叠到顶部圆环的三角形数量
     */
    gltMakeCylinder(cylinderBatch, 2.0f, 2.0f, 3.0f, 15, 2);
    
    //锥
    /*
     void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     参数1：cylinderBatch，三角形批次类对象
     参数2：baseRadius,底部半径
     参数3：topRadius,头部半径
     参数4：fLength,圆形长度
     参数5：numSlices,围绕Z轴的三角形对的数量
     参数6：numStacks,圆柱底部堆叠到顶部圆环的三角形数量
     */
    //圆柱体，从0开始向Z轴正方向延伸。
    //圆锥体，是一端的半径为0，另一端半径可指定。
    gltMakeCylinder(coneBatch, 2.0f, 0.0f, 3.0f, 13, 2);
    
    // 磁盘
    /*
    void gltMakeDisk(GLTriangleBatch& diskBatch, GLfloat innerRadius, GLfloat outerRadius, GLint nSlices, GLint nStacks);
     参数1:diskBatch，三角形批次类对象
     参数2:innerRadius,内圆半径
     参数3:outerRadius,外圆半径
     参数4:nSlices,圆盘围绕Z轴的三角形对的数量
     参数5:nStacks,圆盘外网到内围的三角形数量
     */
    gltMakeDisk(diskBatch, 1.5f, 3.0f, 13, 3);
    // 立方体
    //选择一个在每个方向到原点距离都为20个单位长度的立方体。
    gltMakeCube(cubeBatch, 2);
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
    // 点击空格时，调用的函数
    glutKeyboardFunc(keyPressFunc);
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
