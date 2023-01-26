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

#include "controlDetection.h"

#include "detectNet.h"

#include "glEvents.h"
#include "glWidget.h"

#include "xml.h"

using namespace tinyxml2;


#define STATUS_MSG "Status - "
#define SELECT_LABEL_FILE_MSG STATUS_MSG "select output dataset path and label file"
#define DEFAULT_JPEG_QUALITY 95

#define BBOX_PROPERTY  "bboxIndex"
#define COORD_PROPERTY "coordIndex"


// constructor
ControlDetectionWidget::ControlDetectionWidget( commandLine* cmdLine, CaptureWindow* capture )
{
	captureWindow = capture;

	/*
	 * create layout
 	 */
	QVBoxLayout* layout = new QVBoxLayout();

	layout->setAlignment(Qt::AlignTop);


	// dataset location
	QHBoxLayout* datasetLayout = new QHBoxLayout();
	QPushButton* datasetButton = new QPushButton(tr("..."));

	datasetButton->setMaximumWidth(50);
	connect(datasetButton, SIGNAL(clicked()), this, SLOT(selectDatasetPath()));

	datasetWidget = new QLabel();

	datasetWidget->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	datasetWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	datasetLayout->addWidget(new QLabel(tr("Dataset Path    ")));
	datasetLayout->addWidget(datasetWidget);
	datasetLayout->addWidget(datasetButton);

	layout->addItem(datasetLayout);


	// class label file
	QHBoxLayout* labelLayout = new QHBoxLayout();
	QPushButton* labelButton = new QPushButton(tr("..."));

	labelButton->setMaximumWidth(50);
	connect(labelButton, SIGNAL(clicked()), this, SLOT(selectLabelFile()));

	labelWidget = new QLabel();

	labelWidget->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	labelWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	labelLayout->addWidget(new QLabel(tr("Class Labels      ")));
	labelLayout->addWidget(labelWidget);
	labelLayout->addWidget(labelButton);

	layout->addItem(labelLayout);


	// subset drop-down
	QHBoxLayout* subsetLayout = new QHBoxLayout();

	setDropdown = new QComboBox();

	setDropdown->addItem(tr("train"));
	setDropdown->addItem(tr("val"));
	setDropdown->addItem(tr("test"));

	setDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	subsetLayout->addWidget(new QLabel(tr("Current Set       ")));
	subsetLayout->addWidget(setDropdown);

	layout->addItem(subsetLayout);


	// jpeg quality
	QHBoxLayout* qualityLayout = new QHBoxLayout();

	qualitySlider = new QSlider(Qt::Horizontal);

	qualitySlider->setRange(1,100);
	qualitySlider->setValue(DEFAULT_JPEG_QUALITY);

	connect(qualitySlider, SIGNAL(valueChanged(int)), this, SLOT(onQualityChanged(int)));

	qualityLabel = new QLabel(QString::number(DEFAULT_JPEG_QUALITY));

	qualityLayout->addWidget(new QLabel(tr("JPEG Quality   ")));
	qualityLayout->addWidget(qualitySlider);
	qualityLayout->addWidget(qualityLabel);

	layout->addLayout(qualityLayout);

	
	// object list
	bboxTable = new QTableWidget();

	const int numColumns = 6;
	const int coordColumnWidth = 65;

	bboxTable->setMinimumHeight(125);
	bboxTable->setColumnCount(numColumns);
	bboxTable->setHorizontalHeaderLabels( { "Class", "x", "y", "Width", "Height", "Delete" } );

	for( int n=1; n < numColumns; n++ )
		bboxTable->setColumnWidth(n, coordColumnWidth);

	layout->addWidget(bboxTable);


	// option checkboxes
	saveOnUnfreeze = new QCheckBox("Save on Unfreeze");
	clearOnUnfreeze = new QCheckBox("Clear on Unfreeze");
	mergeDataSubsets = new QCheckBox("Merge Sets");

	saveOnUnfreeze->setCheckState(Qt::Checked);
	clearOnUnfreeze->setCheckState(Qt::Checked);

	QHBoxLayout* optionsLayout = new QHBoxLayout();

	optionsLayout->addWidget(saveOnUnfreeze);
	optionsLayout->addWidget(clearOnUnfreeze);
	optionsLayout->addWidget(mergeDataSubsets);

	layout->addLayout(optionsLayout);

	// freeze button
	freezeButton = new QPushButton("Freeze/Edit (space)");

	freezeButton->setEnabled(false);
	freezeButton->setCheckable(true);
	freezeButton->setShortcut(QKeySequence(Qt::Key_Space));

	connect(freezeButton, SIGNAL(toggled(bool)), this, SLOT(onFreeze(bool)));

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(freezeButton);


	// save button
	saveButton = new QPushButton("Save (S)");

	saveButton->setEnabled(false);
	saveButton->setShortcut(QKeySequence(Qt::Key_S));

	connect(saveButton, SIGNAL(clicked()), this, SLOT(onSave()));

	buttonLayout->addWidget(saveButton);
	layout->addLayout(buttonLayout);


	// status bar
	statusBar = new QStatusBar();

	statusBar->setStyleSheet("QStatusBar{border-top: 1px outset grey; color: rgb(150,150,150);}");
	statusBar->showMessage(tr(SELECT_LABEL_FILE_MSG));

	layout->addWidget(statusBar);

	
	// finish configuration
	setLayout(layout);

	glRegisterEvents(ControlDetectionWidget::onCaptureEvent, this);

}


// destructor
ControlDetectionWidget::~ControlDetectionWidget()
{


}


// hideEvent
void ControlDetectionWidget::hideEvent( QHideEvent* event )
{
	printf("camera-capture:  detection dataset control widget hide()\n");
}


// showEvent
void ControlDetectionWidget::showEvent( QShowEvent* event )
{
	printf("camera-capture:  detection dataset control widget show()\n");
}


// onWidgetEvent
bool ControlDetectionWidget::onWidgetEvent( glWidget* widget, uint16_t event, int a, int b, void* user )
{
	ControlDetectionWidget* control = (ControlDetectionWidget*)user;

	if( event == WIDGET_MOVED || event == WIDGET_RESIZED )
		control->updateBoxCoords(widget->GetIndex());

	return true;
}


// onCaptureEvent
bool ControlDetectionWidget::onCaptureEvent( uint16_t event, int a, int b, void* user )
{
	ControlDetectionWidget* control = (ControlDetectionWidget*)user;

	if( !control )
		return false;

	if( event == WIDGET_CREATED )
	{
		// object class drop-down
		QComboBox* itemClass = new QComboBox();

		for( size_t n=0; n < control->classLabels.size(); n++ )
			itemClass->addItem(QString::fromStdString(control->classLabels[n]));	

		connect(itemClass, SIGNAL(currentIndexChanged(int)), control, SLOT(onBoxClass(int)));

		control->bboxTable->setRowCount(a+1);
		control->bboxTable->setCellWidget(a, 0, itemClass);

		// coordinate spinners
		QDoubleSpinBox* spinBox[4];

		for( int n=0; n < 4; n++ )
		{
			spinBox[n] = new QDoubleSpinBox();		

			spinBox[n]->setDecimals(1);	
			spinBox[n]->setRange(n < 2 ? -9999.0 : 0.0, 9999.0);
			spinBox[n]->setProperty(COORD_PROPERTY, n);

			connect(spinBox[n], SIGNAL(valueChanged(double)), control, SLOT(onBoxCoord(double)));
			control->bboxTable->setCellWidget(a, n+1, spinBox[n]);
		}

		// remove button
		QPushButton* removeButton = new QPushButton("X");
		connect(removeButton, SIGNAL(pressed()), control, SLOT(onBoxRemove()));
		control->bboxTable->setCellWidget(a, 5, removeButton);

		// update properties
		control->updateBoxIndices();
		control->updateBoxCoords(a);
		control->updateBoxColor(a,0);

		// subscribe to widget events
		glWidget* widget = control->captureWindow->GetWidget(a);

		widget->AddEventHandler(ControlDetectionWidget::onWidgetEvent, control);
		widget->SetLineWidth(4.0f);
	}
		
	return true;
}


// clearBoxes
bool ControlDetectionWidget::clearBoxes()
{
	if( !bboxTable || !captureWindow )
		return false;

	bboxTable->setRowCount(0);
	captureWindow->RemoveAllWidgets();

	return true;
}


// onBoxRemove
void ControlDetectionWidget::onBoxRemove()
{
	QPushButton* senderWidget = qobject_cast<QPushButton*>(sender());

	if( !senderWidget )
		return;

	const int bboxIndex = senderWidget->property(BBOX_PROPERTY).toInt();

	//printf("camera-capture:  on box removed => box %i\n", bboxIndex);

	captureWindow->RemoveWidget(bboxIndex);
	bboxTable->removeRow(bboxIndex);
	updateBoxIndices();
}


// onBoxClass
void ControlDetectionWidget::onBoxClass( int index )
{
	QComboBox* senderWidget = qobject_cast<QComboBox*>(sender());

	if( !senderWidget )
		return;

	const int bboxIndex = senderWidget->property(BBOX_PROPERTY).toInt();
	//printf("camera-capture:  on class changed => box %i, class %i\n", bboxIndex, index);
	updateBoxColor(bboxIndex, index);
}


// onBoxCoord
void ControlDetectionWidget::onBoxCoord( double value )
{
	QDoubleSpinBox* senderWidget = qobject_cast<QDoubleSpinBox*>(sender());

	if( !senderWidget )
		return;

	const int bboxIndex = senderWidget->property(BBOX_PROPERTY).toInt();
	const int coordIndex = senderWidget->property(COORD_PROPERTY).toInt();

	//printf("camera-capture:  on coord changed => box %i, coord %i, %f\n", bboxIndex, coordIndex, (float)value);

	glWidget* widget = captureWindow->GetWidget(bboxIndex);

	if( coordIndex == 0 )
		widget->SetX(value);
	else if( coordIndex == 1 )
		widget->SetY(value);
	else if( coordIndex == 2 )
		widget->SetWidth(value);
	else if( coordIndex == 3 )
		widget->SetHeight(value);
}


// updateBoxColor
void ControlDetectionWidget::updateBoxColor( uint32_t index, uint32_t classID )
{
	glWidget* widget = captureWindow->GetWidget(index);
	const float4 color = detectNet::GenerateColor(classID);
	widget->SetLineColor(color.x/255.0f, color.y/255.0f, color.z/255.0f);
}


// updateBoxCoords
void ControlDetectionWidget::updateBoxCoords( uint32_t index )
{
	glWidget* box = captureWindow->GetWidget(index);

	if( !box )
		return;

	QDoubleSpinBox* x = qobject_cast<QDoubleSpinBox*>(bboxTable->cellWidget(index,1));
	QDoubleSpinBox* y = qobject_cast<QDoubleSpinBox*>(bboxTable->cellWidget(index,2));
	QDoubleSpinBox* w = qobject_cast<QDoubleSpinBox*>(bboxTable->cellWidget(index,3));
	QDoubleSpinBox* h = qobject_cast<QDoubleSpinBox*>(bboxTable->cellWidget(index,4));

	if( !x || !y || !w || !h )
		return;

	x->setValue( box->X() );
	y->setValue( box->Y() );
	w->setValue( box->Width() );
	h->setValue( box->Height() );
}


// updateBoxIndices
void ControlDetectionWidget::updateBoxIndices()
{
	const int numRows = bboxTable->rowCount();
	const int numColumns = bboxTable->columnCount();

	for( int y=0; y < numRows; y++ )
	{
		for( int x=0; x < numColumns; x++ )
		{
			QWidget* widget = bboxTable->cellWidget(y,x);

			if( !widget )
				continue;

			widget->setProperty(BBOX_PROPERTY, y);
		}
	}
}


// makeDir
bool ControlDetectionWidget::makeDir( QDir& root, const QString& subdir )
{
	if( !root.exists(subdir) )
	{
		if( !root.mkpath(subdir) ) //mkdir(subdir) )
		{
			const QString msg = QString("Failed to create dataset subdirectory '%1/'").arg(subdir);
			QMessageBox::critical(this, tr("Error Creating Dataset Directories"), msg);
			statusBar->showMessage(QString(STATUS_MSG) + msg);
			return false;
		}
	}
	
	return true;
}


// createDatasetDirectories
bool ControlDetectionWidget::createDatasetDirectories()
{
	// check that we have a valid path to the dataset
	if( datasetPath.size() == 0 )
		return false;

	// create root structure
	QDir root(QString::fromStdString(datasetPath));

	if( !makeDir(root, "Annotations") ||
	    !makeDir(root, "ImageSets/Main") ||
	    !makeDir(root, "JPEGImages") )
	{
		return false;
	}

	return true;
}


// selectDatasetPath
void ControlDetectionWidget::selectDatasetPath()
{
	// prompt user to select the file
	QString qPath = QFileDialog::getExistingDirectory(this, tr("Select Dataset Directory"));

	if( qPath.size() == 0 )
		return;

	datasetPath = qPath.toUtf8().constData();

	// make sure the directories exist
	createDatasetDirectories();

	// enable capture button
	//if( datasetPath.size() > 0 && labelPath.size() > 0 )
	//	freezeButton->setEnabled(true);

	// update label with new path
	QFontMetrics metrics(datasetWidget->font());
	int width = datasetWidget->width() - 2;
	QString clippedText = metrics.elidedText(qPath, Qt::ElideLeft, width);
	datasetWidget->setText(clippedText);

	// enable widgets if ready
	if( datasetPath.size() > 0 && labelPath.size() > 0 )
	{
		freezeButton->setEnabled(true);
		saveButton->setEnabled(true);
	}
}


// selectLabelFile
void ControlDetectionWidget::selectLabelFile()
{
	// prompt user to select the file
	QString qFilename = QFileDialog::getOpenFileName(this, tr("Select Label File"), QString(), tr("Text Files (*.txt)"));

	if( qFilename.size() == 0 )
		return;

	labelPath = qFilename.toUtf8().constData();

	// load the class descriptions	
	if( !detectNet::LoadClassLabels(labelPath.c_str(), classLabels) )
	{
		QMessageBox::critical(this, tr("Failed to Load Class Labels"), tr("There was an error loading the label files from:  ") + qFilename);
		statusBar->showMessage(tr(SELECT_LABEL_FILE_MSG));		
		return;
	}

	// TODO update existing label drop-downs in grid	
	statusBar->showMessage(QString(STATUS_MSG "loaded %1 class labels").arg(classLabels.size()));

	// enable capture button
	//if( datasetPath.size() > 0 && labelPath.size() > 0 )
	//	freezeButton->setEnabled(true);

	// update label with new filename
	QFontMetrics metrics(labelWidget->font());
	int width = labelWidget->width() - 2;
	QString clippedText = metrics.elidedText(qFilename, Qt::ElideLeft, width);
	labelWidget->setText(clippedText);

	// enable widgets if ready
	if( datasetPath.size() > 0 && labelPath.size() > 0 )
	{
		freezeButton->setEnabled(true);
		saveButton->setEnabled(true);
	}
}


// xmlAddElement
inline XMLElement* xmlAddElement( XMLDocument& doc, XMLNode* parent, const char* elementName )
{
	XMLElement* element = doc.NewElement(elementName);
	parent->InsertEndChild(element);
	return element;
}


// xmlAddElement
template<typename T> XMLElement* xmlAddElement( XMLDocument& doc, XMLNode* parent, const char* elementName, T elementValue )
{
	XMLElement* element = xmlAddElement(doc, parent, elementName);
	element->SetText(elementValue);
	return element;
}


// saveFrame
bool ControlDetectionWidget::saveFrame()
{
	printf("camera-capture:  saving frame...\n");

	const std::string datasetName = QDir(QString::fromStdString(datasetPath)).dirName().toStdString();
	const std::string timestamp   = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss").toUtf8().constData();
	const std::string imgFilename = timestamp + ".jpg";
	const std::string imgPath     = datasetPath + "/JPEGImages/" + imgFilename;
	const std::string xmlPath     = datasetPath + "/Annotations/" + timestamp + ".xml";
	
	// save the image
	if( !captureWindow->Save(imgPath.c_str()) )
	{
		statusBar->showMessage(QString(STATUS_MSG "failed to save ") + QString::fromStdString(imgFilename));
		return false;
	}

	// create annotation XML
	XMLDocument doc;

	XMLNode* root = doc.NewElement("annotation");
	doc.InsertFirstChild(root);

	xmlAddElement(doc, root, "filename", imgFilename.c_str());
	xmlAddElement(doc, root, "folder", datasetName.c_str());
	
	XMLElement* source = xmlAddElement(doc, root, "source");
	
	xmlAddElement(doc, source, "database", datasetName.c_str());
	xmlAddElement(doc, source, "annotation", "custom");
	xmlAddElement(doc, source, "image", "custom");

	XMLElement* size = xmlAddElement(doc, root, "size");

	xmlAddElement(doc, size, "width", captureWindow->GetCameraWidth());
	xmlAddElement(doc, size, "height", captureWindow->GetCameraHeight());
	xmlAddElement(doc, size, "depth", 3);
	xmlAddElement(doc, root, "segmented", 0);

	// add bounding boxes to XML
	const int numBoxes = bboxTable->rowCount();

	for( int n=0; n < numBoxes; n++ )
	{
		XMLElement* object = xmlAddElement(doc, root, "object");

		xmlAddElement(doc, object, "name", qPrintable(qobject_cast<QComboBox*>(bboxTable->cellWidget(n,0))->currentText()));
		xmlAddElement(doc, object, "pose", "unspecified");
		xmlAddElement(doc, object, "truncated", "0");
		xmlAddElement(doc, object, "difficult", "0");

		const int bboxIndex = bboxTable->cellWidget(n,0)->property(BBOX_PROPERTY).toInt();
		glWidget* boxWidget = captureWindow->GetWidget(bboxIndex);

		float x1, y1, x2, y2;
		boxWidget->GetCoords(&x1, &y1, &x2, &y2);

		XMLElement* bbox = xmlAddElement(doc, object, "bndbox");
		
		xmlAddElement(doc, bbox, "xmin", (int)x1);
		xmlAddElement(doc, bbox, "ymin", (int)y1);
		xmlAddElement(doc, bbox, "xmax", (int)x2);
		xmlAddElement(doc, bbox, "ymax", (int)y2);
	}

	// save XML
	if( doc.SaveFile(xmlPath.c_str()) != XML_SUCCESS )
	{
		printf("camera-capture:  failed to save %s\n", xmlPath.c_str());
		statusBar->showMessage(QString(STATUS_MSG "failed to save ") + QString::fromStdString(xmlPath));
		return false;
	}

	// append to image list(s)
	const std::string currentSet = setDropdown->currentText().toLower().toStdString();

	if( mergeDataSubsets->checkState() == Qt::Checked )
	{
		addToImageSet("train", timestamp);
		addToImageSet("trainval", timestamp);
		addToImageSet("test", timestamp);
		addToImageSet("val", timestamp);
	}
	else
	{
		addToImageSet(currentSet, timestamp);
	
		if( currentSet == "train" || currentSet == "val" )
			addToImageSet("trainval", timestamp);
	}

	//const int numFiles = QDir(directory.c_str()).count() - 2;
	//statusBar->showMessage(QString(STATUS_MSG "%1 images in %2").arg(QString::number(numFiles), QString::fromStdString(subdirPath)));

	return true;
}


// onSave
void ControlDetectionWidget::onSave()
{
	saveFrame();
}


// addToImageSet
bool ControlDetectionWidget::addToImageSet( const std::string& imgSet, const std::string& imgName )
{
	const std::string filename = datasetPath + "/ImageSets/Main/" + imgSet + ".txt";
	const std::string imgNameNL = imgName + "\n";

	FILE* file = fopen(filename.c_str(), "a");

	if( !file )
	{
		printf("camera-capture:  failed to open %s\n", filename.c_str());
		statusBar->showMessage(QString(STATUS_MSG "failed to open ") + QString::fromStdString(filename));
		return false;
	}

	const int result = fputs(imgNameNL.c_str(), file);

	if( result < 0 )
	{
		printf("camera-capture:  failed to save %s\n", filename.c_str());
		statusBar->showMessage(QString(STATUS_MSG "failed to save ") + QString::fromStdString(filename));
		return false;
	}
	
	fclose(file);
	return true;
}


// onFreeze
void ControlDetectionWidget::onFreeze( bool toggled )
{
	printf("camera-capture:  on freeze (%i)\n", (int)toggled);

	if( !toggled )
	{
		if( saveOnUnfreeze->checkState() == Qt::Checked )
			saveFrame();

		if( clearOnUnfreeze->checkState() == Qt::Checked )
			clearBoxes();
	}

	captureWindow->SetMode( toggled ? CaptureWindow::Edit : CaptureWindow::Live );
}


// onQualityChanged
void ControlDetectionWidget::onQualityChanged( int value )
{
	qualityLabel->setText(QString::number(value));
}


