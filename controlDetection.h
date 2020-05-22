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

#ifndef __CAMERA_CONTROL_DETECTION_WIDGET__
#define __CAMERA_CONTROL_DETECTION_WIDGET__

#include <QtWidgets>

#include "commandLine.h"
#include "captureWindow.h"


/*
 * Detection control widget
 */
class ControlDetectionWidget : public QWidget
{
	Q_OBJECT

public:
	// create window
	ControlDetectionWidget( commandLine* cmdLine, CaptureWindow* captureWindow );

	// destructor
	~ControlDetectionWidget();

public slots:
	void onSave();
	void onFreeze( bool toggled );

	void onBoxRemove();
	void onBoxClass( int value );
	void onBoxCoord( double value );
	void onQualityChanged( int value );
	
	void selectDatasetPath();
	void selectLabelFile();

protected:	
	bool saveFrame();
	bool clearBoxes();
	bool createDatasetDirectories();
	bool makeDir( QDir& root, const QString& subdir );
	bool addToImageSet( const std::string& imgSet, const std::string& imgName );

	void hideEvent( QHideEvent* event );
	void showEvent( QShowEvent* event );

	void updateBoxColor( uint32_t index, uint32_t classID );
	void updateBoxCoords( uint32_t index );
	void updateBoxIndices();

	static bool onCaptureEvent( uint16_t event, int a, int b, void* user );
	static bool onWidgetEvent( glWidget* widget, uint16_t event, int a, int b, void* user );

	CaptureWindow* captureWindow;
	QStatusBar*    statusBar;

	std::vector<std::string> classLabels;

	std::string labelPath;
	QLabel*     labelWidget;
	QComboBox*  setDropdown;

	std::string datasetPath;
	QLabel*     datasetWidget;	

	QLabel*     qualityLabel;
	QSlider*    qualitySlider;

	QCheckBox*  saveOnUnfreeze;
	QCheckBox*  clearOnUnfreeze;
	QCheckBox*  mergeDataSubsets;

	QPushButton* freezeButton;
	QPushButton* saveButton;

	QTableWidget* bboxTable;
};


#endif

