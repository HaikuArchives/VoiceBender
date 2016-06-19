/* BenderView.cpp
*/

#ifndef BENDER_VIEW_H
#include "BenderView.h"
#endif

BenderView::BenderView(BRect rect, char *name)
	   	   : BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW)
{
}

void BenderView::AttachedToWindow()
{
	SetFontName("Times New Roman");
	SetFontSize(24);
}

void BenderView::Draw(BRect /*updateRect*/)
{
	MovePenTo(BPoint(2, 30));
	DrawString("Welcome to ");
	SetHighColor(0, 0, 255);
	DrawString("B");
	SetHighColor(255, 0, 0);
	DrawString("e");
	SetHighColor(0, 0, 0);
	DrawString("nder!");
}
