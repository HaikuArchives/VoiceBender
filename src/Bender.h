/* Bender.h
 */

#ifndef BENDER_H
#define BENDER_H

#ifndef _APPLICATION_H
#include <Application.h>
#endif

#include "fft.h"
#include "worker.h"

class BenderApp : public BApplication
{
	BAudioSubscriber	*reader;
	BAudioSubscriber	*writer;

	/* Workers
	 */
	worker	*step1;
	worker	*step2;
	worker	*step3;
	worker	*last;

	/* Save previous Audio state
	 */
	long	prevADC;
	bool	prevLB;
	bool	prevMIC;
	
	short	*fromadc;
	short	*todac;
	
	long	dataavailable;

public:
			BenderApp();
			~BenderApp(void);

	void	ReadyToRun(void);

	void 	bentdata(void *data);
	bool	input(char *buffer, long count);
	bool	output(char *buffer, long count);
	static bool	inputstub(void *user, char *buffer, long count) { return ((BenderApp *)user)->input(buffer, count); }
	static bool	outputstub(void *user, char *buffer, long count) { return ((BenderApp *)user)->output(buffer, count); }
};

#endif
