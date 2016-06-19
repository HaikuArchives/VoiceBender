/* BenderWindow.h
*/

#ifndef BENDER_WINDOW_H
#define BENDER_WINDOW_H

#ifndef _WINDOW_H
#include <Window.h>
#endif

class BenderWindow : public BWindow {

public:
				BenderWindow(BRect frame); 
virtual	bool	QuitRequested();
};

#endif
