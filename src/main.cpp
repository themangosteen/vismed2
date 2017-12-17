#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// enable multisampling for anti-aliasing
	QSurfaceFormat format = QSurfaceFormat();
	format.setSamples(16);
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
