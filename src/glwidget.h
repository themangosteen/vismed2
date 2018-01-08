#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QFileDialog>

#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <Qt3DRender/QCamera>

#include "volume.h"

class MainWindow;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
	GLWidget(QWidget *parent);
    ~GLWidget();

    enum CompositingMethod {
        MIP        = 0,
        AVERAGE    = 1,
		MIDA       = 2,
		ALPHA      = 3
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
	void setOpacityOffset(int offset);
	void setPerspective(bool enabled);

protected:

	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

protected slots:

	void paintGL() Q_DECL_OVERRIDE;
	void initializeGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;

private:

	// SHADERS AND DATA

	void initShaders();

    void loadTransferFunction1DTex(const QString &fileName);
    void initRayVolumeExitPosMapFramebuffer();
    void loadVolume3DTex();
    void precomputeGradients3DTex();

    void initVolumeBBoxCubeVBO();
    void drawVolumeBBoxCube(GLenum glFaceCullingMode, QOpenGLShaderProgram *shader);

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


	// RENDERING PARAMETERS

    QColor backgroundColor;
	int numSamples = 500;
	const int NUM_SAMPLES_STATIC = 500;
	const int NUM_SAMPLES_INTERACTIVE = 20;
    float sampleRangeStart = 0.000f;
    float sampleRangeEnd = 1.000f;
	float shadingThreshold = 0.15f;
    CompositingMethod compositingMethod = CompositingMethod::MIP;
	bool enableShading = false;
	float opacityOffset = 0;

	// UI AND INTERACTION

	MainWindow *mainWindow;

	Qt3DRender::QCamera camera;
	QPoint lastMousePos; // last mouse position (to determine mouse movement delta)

	void initCamera();


	// DEBUG

	QOpenGLDebugLogger *logger;
	void printDebugMsg(const QOpenGLDebugMessage &msg) { qDebug() << qPrintable(msg.message()); }

};

#endif // GLWIDGET_H
