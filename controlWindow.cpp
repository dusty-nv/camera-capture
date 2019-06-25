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

//#include <QVBoxLayout>


// constructor
ControlWindow::ControlWindow( commandLine& cmdLine, CaptureWindow* capture )
{
	captureWindow = capture;

	/*
	 * create layout
 	 */
	QVBoxLayout* layout = new QVBoxLayout();

	// label file
	QHBoxLayout* labelLayout = new QHBoxLayout();
	QPushButton* labelButton = new QPushButton("...");

	connect(labelButton, SIGNAL(clicked()), this, SLOT(selectLabelFile()));

	labelWidget = new QLabel("/home/nvidia/test.txt");
	labelWidget->setFrameStyle(QFrame::Panel|QFrame::Sunken);

	labelLayout->addWidget(new QLabel("Label File"));
	labelLayout->addWidget(labelWidget);
	labelLayout->addWidget(labelButton);

	layout->addItem(labelLayout);

	
	/*
	 * configure options
 	 */
	setLayout(layout);
	setMinimumWidth(200);
	setWindowTitle("Capture Control");

	// move the window away from the camera feed
	move(capture->GetCameraWidth() + 75, pos().y());
}


// destructor
ControlWindow::~ControlWindow()
{


}


// Create
ControlWindow* ControlWindow::Create( commandLine& cmdLine, CaptureWindow* capture )
{
	// create QApplication
	QApplication* app = new QApplication(cmdLine.argc, cmdLine.argv);

	// create ControlWindow
	ControlWindow* window = new ControlWindow(cmdLine, capture);

	if( !window )
		return NULL;

	window->show();
	return window;
}


// selectLabelFile
void ControlWindow::selectLabelFile()
{
	printf("CLICKED!\n");
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


