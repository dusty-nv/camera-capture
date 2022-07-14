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

#include "controlClassify.h"
#include "imageNet.h"


#define STATUS_MSG "Status - "
#define SELECT_LABEL_FILE_MSG STATUS_MSG "select output dataset path and label file"

#define DEFAULT_JPEG_QUALITY 95


// constructor
ControlClassifyWidget::ControlClassifyWidget( commandLine* cmdLine, CaptureWindow* capture )
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


	// class label drop-down
	QHBoxLayout* classLayout = new QHBoxLayout();

	labelDropdown = new QComboBox();

	labelDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	classLayout->addWidget(new QLabel(tr("Current Class   ")));
	classLayout->addWidget(labelDropdown);

	layout->addItem(classLayout);


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


	// capture button
	captureButton = new QPushButton("Capture (space)");

	captureButton->setEnabled(false);
	captureButton->setShortcut(QKeySequence(Qt::Key_Space));

	connect(captureButton, SIGNAL(clicked()), this, SLOT(onCapture()));

	layout->addWidget(captureButton);


	// status bar
	statusBar = new QStatusBar();

	statusBar->setStyleSheet("QStatusBar{border-top: 1px outset grey; color: rgb(150,150,150);}");
	statusBar->showMessage(tr(SELECT_LABEL_FILE_MSG));

	layout->addWidget(statusBar);

	
	/*
	 * configure options
 	 */
	setLayout(layout);
}


// destructor
ControlClassifyWidget::~ControlClassifyWidget()
{


}


// createDatasetDirectories
void ControlClassifyWidget::createDatasetDirectories()
{
	// check that we have a valid path to the dataset
	if( datasetPath.size() == 0 )
		return;

	// check that we have loaded class labels
	const int numClasses = labelDropdown->count();

	if( numClasses == 0 )
		return;

	// check that each subdirectory exists
	QDir dir(QString::fromStdString(datasetPath));

	// create directories for each training set and class
	const int numSets = setDropdown->count();

	for( int s=0; s < numSets; s++ )
	{
		const QString setName = setDropdown->itemText(s);

		if( !dir.exists(setName) )
		{
			if( !dir.mkdir(setName) )
			{
				const QString msg = QString("Failed to create dataset subdirectory '%1/'").arg(setName);
				QMessageBox::critical(this, tr("Error Creating Dataset Directories"), msg);
				statusBar->showMessage(QString(STATUS_MSG) + msg);
				continue;
			}
		}
	
		QDir setDir = dir;
		
		if( !setDir.cd(setName) )
		{
			const QString msg = QString("Failed to cd to dataset subdirectory '%1/'").arg(setName);
			QMessageBox::critical(this, tr("Error Creating Dataset Directories"), msg);
			statusBar->showMessage(QString(STATUS_MSG) + msg);
			continue;
		}

		for( int n=0; n < numClasses; n++ )
		{
			const QString subdir = labelDropdown->itemText(n);

			if( !setDir.exists(subdir) )
			{
				if( !setDir.mkdir(subdir) )
				{
					const QString msg = QString("Failed to create dataset subdirectory '%1/%2/'").arg(setName, subdir);
					QMessageBox::critical(this, tr("Error Creating Dataset Directories"), msg);
					statusBar->showMessage(QString(STATUS_MSG) + msg);
					continue;
				}
			}
		}
	}		
}

// selectDatasetPath
void ControlClassifyWidget::selectDatasetPath()
{
	// prompt user to select the file
	QString qPath = QFileDialog::getExistingDirectory(this, tr("Select Dataset Directory"));

	if( qPath.size() == 0 )
		return;

	datasetPath = qPath.toUtf8().constData();

	// make sure the directories exist
	createDatasetDirectories();

	// enable capture button
	if( datasetPath.size() > 0 && labelPath.size() > 0 )
		captureButton->setEnabled(true);

	// update label with new path
	QFontMetrics metrics(datasetWidget->font());
	int width = datasetWidget->width() - 2;
	QString clippedText = metrics.elidedText(qPath, Qt::ElideLeft, width);
	datasetWidget->setText(clippedText);
}


// selectLabelFile
void ControlClassifyWidget::selectLabelFile()
{
	// prompt user to select the file
	QString qFilename = QFileDialog::getOpenFileName(this, tr("Select Label File"), QString(), tr("Text Files (*.txt)"));

	if( qFilename.size() == 0 )
		return;

	labelPath = qFilename.toUtf8().constData();

	// load the class descriptions	
	std::vector<std::string> classDesc;

	if( !imageNet::LoadClassLabels(labelPath.c_str(), classDesc) )
	{
		QMessageBox::critical(this, tr("Failed to Load Class Labels"), tr("There was an error loading the label files from:  ") + qFilename);
		statusBar->showMessage(tr(SELECT_LABEL_FILE_MSG));		
		return;
	}

	// update drop-down with new labels
	const size_t numClasses = classDesc.size();

	labelDropdown->clear();

	for( size_t n=0; n < numClasses; n++ )
		labelDropdown->addItem(QString::fromStdString(classDesc[n]));		

	statusBar->showMessage(QString(STATUS_MSG "loaded %1 class labels").arg(numClasses));


	// make sure the directories exist
	createDatasetDirectories();

	// enable capture button
	if( datasetPath.size() > 0 && labelPath.size() > 0 )
		captureButton->setEnabled(true);

	// update label with new filename
	QFontMetrics metrics(labelWidget->font());
	int width = labelWidget->width() - 2;
	QString clippedText = metrics.elidedText(qFilename, Qt::ElideLeft, width);
	labelWidget->setText(clippedText);
}


// onCapture
void ControlClassifyWidget::onCapture()
{
	const std::string subsetLabel = setDropdown->currentText().toUtf8().constData();
	const std::string classLabel  = labelDropdown->currentText().toUtf8().constData();
	const std::string timestamp   = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss").toUtf8().constData();
	const std::string subdirPath  = subsetLabel + "/" + classLabel;
	const std::string directory   = datasetPath + "/" + subdirPath;
	const std::string filename    = directory + "/" + timestamp + ".jpg";

	if( !captureWindow->Save(filename.c_str()) )
	{
		statusBar->showMessage(QString(STATUS_MSG "failed to save ") + QString::fromStdString(timestamp) + QString(".jpg"));
		return;
	}

	const int numFiles = QDir(directory.c_str()).count() - 2;
	statusBar->showMessage(QString(STATUS_MSG "%1 images in %2").arg(QString::number(numFiles), QString::fromStdString(subdirPath)));
}


// onQualityChanged
void ControlClassifyWidget::onQualityChanged( int value )
{
	qualityLabel->setText(QString::number(value));
}


