#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QFileDialog>

#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Volume.h"

class MainWindow;

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
        mainWindow = qobject_cast<MainWindow *>(this->parent()->parent()->parent());
    }
    ~GLWidget();

    enum CompositingMethod {
        MIP        = 0,
        AVERAGE    = 1,
        ALPHA      = 2
    };

public slots:
    void dataLoaded(Volume *volume);

    void setNumSamples(int numSamples);
    void setSampleRangeStart(double sampleRangeStart);
    void setSampleRangeEnd(double sampleRangeEnd);
    void setCompositingMethod(CompositingMethod m);
    void loadTransferFunctionImage();
    void setShading(bool enableShading);
    void setShadingThreshold(double thresh);

protected:

    void initializeGL();

    void paintGL();

    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    void initShaders();

    void loadTransferFunction1DTex(const QString &fileName);
    void initRayVolumeExitPosMapFramebuffer();
    void loadVolume3DTex();
    void precomputeGradients3DTex();

    void initVolumeBBoxCubeVBO();
    void drawVolumeBBoxCube(GLenum glFaceCullingMode, QOpenGLShaderProgram *shader);

    MainWindow * mainWindow;

    QOpenGLDebugLogger *logger;
    void printDebugMsg(const QOpenGLDebugMessage &msg) { qDebug() << qPrintable(msg.message()); }

    QOpenGLFunctions_3_3_Core *glf;

    QOpenGLShaderProgram *rayVolumeExitPosMapShader;
    QOpenGLShaderProgram *raycastShader;

    QOpenGLTexture *transferFunction1DTex;
    QOpenGLFramebufferObject *rayVolumeExitPosMapFramebuffer;
    QOpenGLTexture *volume3DTex;
    QOpenGLTexture *gradients3DTex;

    Volume *volume;
    std::vector<QVector3D> gradients;

    QOpenGLVertexArrayObject volumeBBoxCubeVAO;

    QMatrix4x4 modelMat;
    QMatrix4x4 viewMat;
    QMatrix4x4 projMat;

    GLfloat cubeVertices[24] = {
        // front
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        // back
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
    };

    GLuint cubeTriangleIndices[36] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // top
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // bottom
        4, 0, 3,
        3, 7, 4,
        // left
        4, 5, 1,
        1, 0, 4,
        // right
        3, 2, 6,
        6, 7, 3,
    };

    QColor backgroundColor;
    int numSamples = 200;
	int numSamplesInteractive = 0;
    float sampleRangeStart = 0.000f;
    float sampleRangeEnd = 1.000f;
	float shadingThreshold = 0.15f;
    CompositingMethod compositingMethod = CompositingMethod::MIP;
    bool enableShading = false;

    QPoint lastMousePos;
    float volumeRotAngleX;
    float volumeRotAngleY;
    float volumeRotAngleZ;
    QVector3D viewOffset;

};

#endif // GLWIDGET_H
