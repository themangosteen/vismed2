
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include "GLWidget.h"
#include "Volume.h"

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QVariant>
#include <QMouseEvent>


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	MainWindow(QWidget *parent = 0);
	~MainWindow();

signals:

	void dataLoaded(Volume *volumeData);

protected slots :

	void openFileAction();
	void closeAction();
    void setCompositing();
    void setShading();

private:

	// USER INTERFACE ELEMENTS

	Ui_MainWindow *ui;
	GLWidget *glWidget;


	// DATA

	enum DataType
	{
		VOLUME = 0,
	};

	struct FileType
	{
		QString filename;
		DataType type;
	} fileType;

	Volume *volume; // for Volume-Rendering

};

#endif
