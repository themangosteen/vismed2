#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{

	QApplication app(argc, argv);

	// set default opengl surface format
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow mainWindow;
	mainWindow.show();

	return app.exec();

}
