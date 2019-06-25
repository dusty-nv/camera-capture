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

#include "cameraWindow.h"

#include "gstCamera.h"
#include "glDisplay.h"



// constructor
CameraWindow::CameraWindow()
{
	camera  = NULL;
	display = NULL;
	imgRGBA = NULL;
}


// destructor
CameraWindow::~CameraWindow()
{
	SAFE_DELETE(camera);
	SAFE_DELETE(display);
}


// Create
CameraWindow* CameraWindow::Create( const commandLine& cmdLine )
{
	CameraWindow* window = new CameraWindow();

	if( !window || !window->init(cmdLine) )
	{
		printf("camera-collection:  CameraWindow::Create() failed\n");
		return NULL;
	}

	return window;
}


// init
bool CameraWindow::init( const commandLine& cmdLine )
{
	/*
	 * create the camera device
	 */
	camera = gstCamera::Create(cmdLine.GetInt("width", gstCamera::DefaultWidth),
						  cmdLine.GetInt("height", gstCamera::DefaultHeight),
						  cmdLine.GetString("camera"));

	if( !camera )
	{
		printf("\ncamera-collection:  failed to initialize camera device\n");
		return false;
	}
	
	printf("\ncamera-collection:  successfully initialized camera device\n");
	printf("    width:  %u\n", camera->GetWidth());
	printf("   height:  %u\n", camera->GetHeight());
	printf("    depth:  %u (bpp)\n\n", camera->GetPixelDepth());
	

	/*
	 * create openGL window
	 */
	display = glDisplay::Create();

	if( !display ) 
	{
		printf("camera-collection:  failed to create openGL display\n");
		return false;
	}

	return true;
}
	

// Render
void CameraWindow::Render()
{
	// capture RGBA image
	if( !camera->CaptureRGBA(&imgRGBA, 1000, true) )
		printf("camera-collection:  failed to capture RGBA image from camera\n");

	// update display
	if( display != NULL )
	{
		// render the image
		display->RenderOnce(imgRGBA, camera->GetWidth(), camera->GetHeight());

		// update the status bar
		char str[256];
		sprintf(str, "Data Collection Tool | %.0f FPS", display->GetFPS());
		display->SetTitle(str);
	}
}


// IsOpen
bool CameraWindow::IsOpen() const
{
	if( !display )
		return false;

	return display->IsOpen();
}


// IsClosed
bool CameraWindow::IsClosed() const
{
	if( !display )
		return true;

	return display->IsClosed();
}


// IsStreaming
bool CameraWindow::IsStreaming() const
{
	if( !camera )
		return false;

	return camera->IsStreaming();
}


