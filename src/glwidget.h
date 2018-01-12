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
		ALPHA      = 0, // alpha compositing ("dvr")
		MIDA       = 1, // maximum intensity difference accumulation (allows interpolation between dvr and mip)
		MIP        = 2, // maximum intensity projection
		AVERAGE    = 3, // average intensity projection
		MINIP      = 4  // minimum intensity projection
    };

public slots:

	void dataLoaded(Volume *volume);

	// set total number of samples along the ray
    void setNumSamples(int numSamples);

	// samples before this sample point along the ray are ignored
    void setSampleRangeStart(double sampleRangeStart);

	// samples after this sample point along the ray are ignored
    void setSampleRangeEnd(double sampleRangeEnd);

	// set the CompositingMethod
    void setCompositingMethod(CompositingMethod m);

	// load 1D transfer function texture from which colors are sampled based on intensity
    void loadTransferFunctionImage();

	// enable or disable gradient-based shading
    void setShading(bool enableShading);

	// set threshold when gradient-based shading should start
    void setShadingThreshold(double thresh);

	// dont draw voxels with intensity below this value
	void setIntensityClampMin(float value);

	// dont draw voxels with intensity higher than this value
	void setIntensityClampMax(float value);

	// multiply opacity used in accumulation
	void setOpacityFactor(float factor);

	// offset opacity used in accumulation
	void setOpacityOffset(float offset);

	// multiply transfer function texture lookup position
	void setTTFSampleFactor(float factor);

	// offset transfer function texture lookup position
	void setTTFSampleOffset(float offset);

	// set parameter for MIDA compositing method
	// value in range [-1,1]
	// -1: like DVR, colors are accumulated like in traditional DVR alpha compositing
	//  1: like MIP, previously accumulated color is ignored, only highest intensity is projected
	// the higher the value, the higher the contribution of values that represent new maxima
	// and the less previous accumulation is weighted, leaving more transparency for new to shine through.
	// usually param will be inbetween, thus giving the advantage of
	// important structures shining through as in MIP combined with depth cue from some accumulation.
	void setMIDAParam(float value);

	// switch between perspective and orthographic camera projection mode
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
	CompositingMethod compositingMethod = CompositingMethod::MIDA;
	bool enableShading = false;
	float intensityClampMin = 0.f;
	float intensityClampMax = 1.f;
	float opacityFactor = 1.f;
	float opacityOffset = 0.f;
	float ttfSampleFactor = 1.f;
	float ttfSampleOffset = 0.f;
	float midaParam = 0.f;

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
