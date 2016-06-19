/* BenderWindow.cpp
 */

#ifndef _APPLICATION_H
#include <Application.h>
#endif

#ifndef BENDER_WINDOW_H
#include "BenderWindow.h"
#endif

BenderWindow::BenderWindow(BRect frame)
				: BWindow(frame, "Bender", B_TITLED_WINDOW, 
				B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
}

bool BenderWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(TRUE);
}
