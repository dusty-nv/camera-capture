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

#ifndef __CAMERA_CAPTURE_WINDOW__
#define __CAMERA_CAPTURE_WINDOW__

#include "commandLine.h"
#include "cudaUtility.h"

// forward declarations
class videoSource;
class glDisplay;
class glWidget;


/*
 * Camera feed window
 */
class CaptureWindow
{
public:
	// CaptureMode enum
	enum CaptureMode
	{
		Live,
		Edit
	};

	// create the window and camera object
	static CaptureWindow* Create( commandLine& cmdLine );

	// close the window and camera object
	~CaptureWindow();

	// capture & render next camera frame
	void Render();

	// save the latest frame to disk
	bool Save( const char* filename, int quality=95 );

	// set the current capture mode
	void SetMode( CaptureMode mode );

	// window open/closed status
	bool IsOpen() const;
	bool IsClosed() const;

	// camera streaming status
	bool IsStreaming() const;

	// camera dimensions
	int GetCameraWidth() const;
	int GetCameraHeight() const;

	// window dimensions
	int GetWindowWidth() const;
	int GetWindowHeight() const;

	// widget operations
	glWidget* GetWidget( int index ) const;

	void RemoveWidget( int index ) const;
	void RemoveAllWidgets() const;

protected:
	CaptureWindow();
	bool init( commandLine& cmdLine );

	static const int cameraOffsetX = 5;
	static const int cameraOffsetY = 5;

	CaptureMode mode;

	videoSource* camera;
	glDisplay* display;

	uchar3* imgRGB;
};

#endif

