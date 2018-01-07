#include "mainwindow.h"

#include <QFileDialog>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , volume(0)
{
	ui->setupUi(this);

	ui->progressBar->hide();

	glWidget = ui->glWidget;

	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeAction()));

	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeAction()));
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openFileAction()));
	connect(this, &MainWindow::dataLoaded, glWidget, &GLWidget::dataLoaded);

	connect(ui->numSamplesSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), glWidget, &GLWidget::setNumSamples);
	connect(ui->sampleStartSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), glWidget, &GLWidget::setSampleRangeStart);
	connect(ui->sampleEndSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), glWidget, &GLWidget::setSampleRangeEnd);
    connect(ui->shadingThresholdSpinbox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), glWidget, &GLWidget::setShadingThreshold);
	connect(ui->opacityOffsetSlider, &QSlider::valueChanged, glWidget, &GLWidget::setOpacityOffset);
	connect(ui->compositingModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setCompositing);
	connect(ui->loadDataPushButton, &QPushButton::clicked, this, &MainWindow::openFileAction);
	connect(ui->loadTffImageButton, &QPushButton::clicked, glWidget, &GLWidget::loadTransferFunctionImage);
	connect(ui->shadedCheckBox, &QCheckBox::clicked, glWidget, &GLWidget::setShading);
	connect(ui->perspectiveCheckBox, &QCheckBox::clicked, this, &MainWindow::setPerspective);

}

MainWindow::~MainWindow()
{
	delete volume;
}


//-------------------------------------------------------------------------------------------------
// Slots
//-------------------------------------------------------------------------------------------------

void MainWindow::openFileAction()
{
	QString filepath = QFileDialog::getOpenFileName(this, "Data File", 0, tr("Data Files (*.dat)"));

	openFile(filepath);
}

void MainWindow::openFile(QString filepath)
{
	if (!filepath.isEmpty())
	{
		// store filename
		fileType.filename = filepath;
		std::string fn = filepath.toStdString();
		bool success = false;

		// progress bar and top label
		ui->progressBar->show();
		ui->progressBar->setEnabled(true);
		ui->labelTop->setText("Loading data ...");

		// load data according to file extension
		if (fn.substr(fn.find_last_of(".") + 1) == "dat") // LOAD VOLUME
		{
			// create VOLUME
			fileType.type = VOLUME;
			volume = new Volume();

			// load file
			success = volume->loadFromFile(filepath, ui->progressBar);
		}

		ui->progressBar->setEnabled(false);
		ui->progressBar->hide();

		// status message
		if (success)
		{
			QString type;
			if (fileType.type == VOLUME) {
				type = "VOLUME";
				emit dataLoaded(volume);
			}
			ui->labelTop->setText(QString("Loaded VOLUME [%1 x %2 x %3] \n %4").arg(QString::number(volume->getWidth()), QString::number(volume->getHeight()), QString::number(volume->getDepth()), filepath));
		}
		else
		{
			ui->labelTop->setText("ERROR loading file " + filepath + "!");
			ui->progressBar->setValue(0);
		}
	}
}

void MainWindow::closeAction()
{
	close();
}

void MainWindow::setCompositing(int mode)
{
	glWidget->setCompositingMethod((GLWidget::CompositingMethod)mode);
}

void MainWindow::setShading()
{
	glWidget->setShading(ui->shadedCheckBox->isChecked());
}

void MainWindow::setPerspective(bool enabled)
{
	if (enabled) {
		ui->label3DInteractionHelp->setText("ROTATE: left mouse, PAN: shift + left mouse, ZOOM PERSPECTIVE: alt + left mouse");
	}
	else {
		ui->label3DInteractionHelp->setText("ROTATE: left mouse, PAN: shift + left mouse");
	}

	glWidget->setPerspective(enabled);
}

