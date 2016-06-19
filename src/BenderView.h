/* BenderView.h
*/

#ifndef BENDER_VIEW_H
#define BENDER_VIEW_H

#ifndef _VIEW_H
#include <View.h>
#endif

class BenderView : public BView {

public:
				BenderView(BRect frame, char *name); 
virtual	void	AttachedToWindow();
virtual	void	Draw(BRect updateRect);
};

#endif
