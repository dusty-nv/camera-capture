/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "controlWindow.h"

#include "controlClassify.h"
#include "controlDetection.h"


// constructor
ControlWindow::ControlWindow( commandLine& commandLine, CaptureWindow* capture )
{
	captureWindow = capture;
	cmdLine 	    = &commandLine;

	datasetTypes[0] = "Classification";
	datasetTypes[1] = "Detection";

	memset(datasetWidgets, 0, sizeof(datasetWidgets));


	/*
	 * create layout
 	 */
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setAlignment(Qt::AlignTop);


	/*
	 * dataset type drop-down
	 */
	QHBoxLayout* datasetLayout = new QHBoxLayout();
	QComboBox* datasetDropdown = new QComboBox();

	for( int n=0; n < numDatasetTypes; n++ )
		datasetDropdown->addItem(tr(datasetTypes[n]));

	datasetDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	datasetLayout->addWidget(new QLabel(tr("  Dataset Type    ")));
	datasetLayout->addWidget(datasetDropdown);

	connect(datasetDropdown, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onDatasetType(const QString&)));

	layout->addItem(datasetLayout);


	/*
	 * dataset control widgets
	 */
	datasetWidgets[0] = new ControlClassifyWidget(cmdLine, captureWindow);
	datasetWidgets[1] = new ControlDetectionWidget(cmdLine, captureWindow);

	layout->addWidget(datasetWidgets[0]);
	layout->addWidget(datasetWidgets[1]);

	datasetWidgets[1]->hide();


	/*
	 * configure options
 	 */
	setMinimumWidth(200);
	setMaximumWidth(750);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
	setLayout(layout);
	setWindowTitle(tr("Data Capture Control"));
	

	// move the window away from the camera feed
	move(capture->GetWindowWidth() + 75, 25 /*pos().y()*/);
}


void ControlWindow::onDatasetType(const QString& text)
{
	printf("camera-capture:  switching dataset type to '%s'\n", text.toUtf8().constData());

	for( int n=0; n < numDatasetTypes; n++ )
	{
		if( text != datasetTypes[n] )
			datasetWidgets[n]->hide();
	}

	for( int n=0; n < numDatasetTypes; n++ )
	{
		if( text == datasetTypes[n] )
			datasetWidgets[n]->show();
	}
}
 

// sizeHint
QSize ControlWindow::sizeHint() const
{
	return QSize(475,15);
}


// destructor
ControlWindow::~ControlWindow()
{


}


// ProcessEvents
void ControlWindow::ProcessEvents()
{
	QCoreApplication::processEvents();
}


// IsOpen
bool ControlWindow::IsOpen() const
{
	return isVisible();
}


// IsClosed
bool ControlWindow::IsClosed() const
{
	return !isVisible();
}


// https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
class DarkStyle : public QProxyStyle {
 public:
  DarkStyle();
  explicit DarkStyle(QStyle *style);

  QStyle *baseStyle() const;

  void polish(QPalette &palette) override;
  void polish(QApplication *app) override;

 private:
  QStyle *styleBase(QStyle *style = Q_NULLPTR) const;
};


// Create
ControlWindow* ControlWindow::Create( commandLine& cmdLine, CaptureWindow* capture )
{
	// create QApplication
	QApplication* app = new QApplication(cmdLine.argc, cmdLine.argv);

	app->setStyle(new DarkStyle());

	// create ControlWindow
	ControlWindow* window = new ControlWindow(cmdLine, capture);

	if( !window )
		return NULL;

	window->show();
	return window;
}


//-----------------------------------------------------------------------------------------
DarkStyle::DarkStyle() : DarkStyle(styleBase()) {}

DarkStyle::DarkStyle(QStyle *style) : QProxyStyle(style) {}

QStyle *DarkStyle::styleBase(QStyle *style) const {
  static QStyle *base =
      !style ? QStyleFactory::create(QStringLiteral("Fusion")) : style;
  return base;
}

QStyle *DarkStyle::baseStyle() const { return styleBase(); }

void DarkStyle::polish(QPalette &palette) {
  // modify palette to dark
  palette.setColor(QPalette::Window, QColor(53, 53, 53));
  palette.setColor(QPalette::WindowText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::WindowText,
                   QColor(127, 127, 127));
  palette.setColor(QPalette::Base, QColor(42, 42, 42));
  palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
  palette.setColor(QPalette::ToolTipBase, Qt::white);
  palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
  palette.setColor(QPalette::Text, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
  palette.setColor(QPalette::Dark, QColor(35, 35, 35));
  palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  palette.setColor(QPalette::Button, QColor(53, 53, 53));
  palette.setColor(QPalette::ButtonText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                   QColor(127, 127, 127));
  palette.setColor(QPalette::BrightText, Qt::red);
  palette.setColor(QPalette::Link, QColor(42, 130, 218));
  palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
  palette.setColor(QPalette::HighlightedText, Qt::white);
  palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                   QColor(127, 127, 127));
}

void DarkStyle::polish(QApplication *app) {
  if (!app) return;

  // increase font size for better reading,
  // setPointSize was reduced from +2 because when applied this way in Qt5, the
  // font is larger than intended for some reason
  QFont defaultFont = QApplication::font();
  defaultFont.setPointSize(defaultFont.pointSize() + 1);
  app->setFont(defaultFont);

  // loadstylesheet
  QFile qfDarkstyle(QStringLiteral(":/darkstyle/darkstyle.qss"));
  if (qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // set stylesheet
    QString qsStylesheet = QString::fromLatin1(qfDarkstyle.readAll());
    app->setStyleSheet(qsStylesheet);
    qfDarkstyle.close();
  }
}


