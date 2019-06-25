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

#include <QtWidgets>
#include <signal.h>

#include "cameraWindow.h"
#include "captureWindow.h"



bool signal_recieved = false;

void sig_handler(int signo)
{
	if( signo == SIGINT )
	{
		printf("received SIGINT\n");
		signal_recieved = true;
	}
}

int usage()
{
	printf("usage: camera-capture [-h] [--camera CAMERA]\n");
	printf("                      [--width WIDTH] [--height HEIGHT]\n\n");
	printf("GUI tool for collecting & labeling data from live camera feed\n\n");
	printf("optional arguments:\n");
	printf("  --help           show this help message and exit\n");
	printf("  --camera CAMERA  index of the MIPI CSI camera to use (NULL for CSI camera 0),\n");
	printf("                   or for VL42 cameras the /dev/video node to use (/dev/video0).\n");
     printf("                   by default, MIPI CSI camera 0 will be used.\n");
	printf("  --width WIDTH    desired width of camera stream (default is 1280 pixels)\n");
	printf("  --height HEIGHT  desired height of camera stream (default is 720 pixels)\n\n");

	return 0;
}

int main( int argc, char** argv )
{
	/*
	 * parse command line
	 */
	commandLine cmdLine(argc, argv);

	if( cmdLine.GetFlag("help") )
		return usage();


	/*
	 * attach signal handler
	 */
	if( signal(SIGINT, sig_handler) == SIG_ERR )
		printf("\ncan't catch SIGINT\n");


	/*
	 * create camera window
	 */
	CameraWindow* cameraWindow = CameraWindow::Create(cmdLine);

	if( !cameraWindow )
	{
		printf("camera-capture:  failed to open CameraWindow\n");
		return 0;
	}


	/*
	 * processing loop
	 */
	while( !signal_recieved )
	{
		cameraWindow->Render();

		if( cameraWindow->IsClosed() )
			signal_recieved = true;
	}
	

	/*
	 * destroy resources
	 */
	printf("camera-capture:  shutting down...\n");
	
	if( cameraWindow != NULL )
		delete cameraWindow;

	printf("camera-capture:  shutdown complete.\n");
	return 0;
}

