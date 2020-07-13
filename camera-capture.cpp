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

#include "captureWindow.h"
#include "controlWindow.h"

#include "videoSource.h"

#include <signal.h>


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
	printf("usage: camera-capture [-h] input_URI\n\n");
	printf("GUI tool for collecting & labeling data from live camera feed\n\n");
	printf("optional arguments:\n");
	printf("  --help           show this help message and exit\n");
	printf("%s", videoSource::Usage());

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
	 * create capture window
	 */
	CaptureWindow* captureWindow = CaptureWindow::Create(cmdLine);

	if( !captureWindow )
	{
		printf("camera-capture:  failed to open CaptureWindow\n");
		return 0;
	}

	
	/*
	 * create control window
	 */
	ControlWindow* controlWindow = ControlWindow::Create(cmdLine, captureWindow);

	if( !controlWindow )
	{
		printf("camera-capture:  failed to open ControlWindow\n");
		return 0;
	}


	/*
	 * processing loop
	 */
	while( !signal_recieved )
	{
		// capture & render latest camera frame
		captureWindow->Render();

		// update the control window
		controlWindow->ProcessEvents();

		// check if the user quit
		if( captureWindow->IsClosed() || controlWindow->IsClosed() )
			signal_recieved = true;
	}
	

	/*
	 * destroy resources
	 */
	printf("camera-capture:  shutting down...\n");
	
	if( controlWindow != NULL )
		delete controlWindow;

	if( captureWindow != NULL )
		delete captureWindow;

	printf("camera-capture:  shutdown complete.\n");
	return 0;
}

