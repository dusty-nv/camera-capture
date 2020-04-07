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

#ifndef __CAMERA_CONTROL_CLASSIFY_WIDGET__
#define __CAMERA_CONTROL_CLASSIFY_WIDGET__

#include <QtWidgets>

#include "commandLine.h"
#include "captureWindow.h"


/*
 * Classification control widget
 */
class ControlClassifyWidget : public QWidget
{
	Q_OBJECT

public:
	// create window
	ControlClassifyWidget( commandLine* cmdLine, CaptureWindow* captureWindow );

	// destructor
	~ControlClassifyWidget();

public slots:
	void onCapture();
	void onQualityChanged( int value );

	void selectDatasetPath();
	void selectLabelFile();

protected:
	void createDatasetDirectories();

	CaptureWindow* captureWindow;
	QStatusBar*    statusBar;

	std::string labelPath;
	QLabel*     labelWidget;
	QComboBox*  labelDropdown;
	QComboBox*  setDropdown;

	std::string datasetPath;
	QLabel*     datasetWidget;	

	QLabel*     qualityLabel;
	QSlider*    qualitySlider;

	QPushButton* captureButton;
};


#endif

