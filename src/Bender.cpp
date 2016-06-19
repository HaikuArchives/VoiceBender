/* Bender.cpp
 */

#ifndef BENDER_WINDOW_H
#include "BenderWindow.h"
#endif
#ifndef BENDER_VIEW_H
#include "BenderView.h"
#endif
#ifndef BENDER_H
#include "Bender.h"
#endif

#include "worker.h"
#include "math.h"

#define FFT_BITS 8
#define FFT_SAMPLES (1 << FFT_BITS)
#define RAW_DATA_SIZE (FFT_SAMPLES * sizeof(short))

/* ********************************************************************************** */
main()
{	
	BenderApp *myApplication;

	myApplication = new BenderApp();
	if(myApplication)
	{
		myApplication->Run();
		delete myApplication;
	}
	
	return 0;
}

/* ********************************************************************************** */
class timetofreq : public worker
{
	double	*fdata;
	fft		transform;
	void	processdata(void *source, void *destination);

public:
			timetofreq(worker *target);
			~timetofreq(void);
};

timetofreq::timetofreq(worker *target)
 : worker("Time to freq", B_REAL_TIME_PRIORITY, target, sizeof(short) * FFT_SAMPLES), transform(FFT_BITS)
{
	fdata = new double [FFT_SAMPLES];
}

timetofreq::~timetofreq(void)
{
	delete [] fdata;
}

void timetofreq::processdata(void *source, void *destination)
{
	double	*real = (double *)destination;
	double	*imag = ((double *)destination) + FFT_SAMPLES;
	short	*data = (short *)source;
	int		i;
	
	for(i = 0; i < FFT_SAMPLES; i++)
		fdata[i] = (double)data[i] / (2. * (double)FFT_SAMPLES);

	transform.forward(fdata, real, imag);
}

/* ********************************************************************************** */
class bender : public worker
{
	int		*index;
	void	processdata(void *source, void *destination);

public:
			bender(worker *target);
			~bender(void);

	void	change(double a);
};

bender::bender(worker *target)
 : worker("Bend it!", B_REAL_TIME_PRIORITY, target, sizeof(double) * 2 * FFT_SAMPLES)
{
	index = new int [FFT_SAMPLES / 2];
	
	change(1);
}

bender::~bender(void)
{
	delete [] index;
}

void bender::change(double a)
{
	int		i;

	/* This is a very quick hack, insert your own here
	 */	
	for(i = 1; i < FFT_SAMPLES / 2; i++)
		index[i] = (int)(exp(log(i) * a) + .5);
}

void bender::processdata(void *source, void *destination)
{
	double	*srcreal = (double *)source;
	double	*srcimag = ((double *)source) + FFT_SAMPLES;
	double	*destreal = (double *)destination;
	double	*destimag = ((double *)destination) + FFT_SAMPLES;
	int		i;
	int		j;

	memcpy(destination, source, FFT_SAMPLES * 2 * sizeof(double));
	for(i = 1; i <= FFT_SAMPLES / 2; i++)
	{
		j = index[i];
		destreal[i] = (j <= FFT_SAMPLES / 2) ? srcreal[j] : 0;
		destimag[i] = (j <= FFT_SAMPLES / 2) ? srcimag[j] : 0;
	}

	for(i = 1; i <= FFT_SAMPLES / 2; i++)
	{
		destreal[FFT_SAMPLES - i] = destreal[i];
		destimag[FFT_SAMPLES - i] = -destimag[i];
	}
}

/* ********************************************************************************** */
class freqtotime : public worker
{
	double	*fdata;
	fft		transform;
	void	processdata(void *source, void *destination);

public:
			freqtotime(worker *target);
			~freqtotime(void);
};

freqtotime::freqtotime(worker *target)
 : worker("Freq to time", B_REAL_TIME_PRIORITY, target, sizeof(double) * 2 * FFT_SAMPLES), transform(FFT_BITS)
{
	fdata = new double [FFT_SAMPLES];
}

freqtotime::~freqtotime(void)
{
	delete [] fdata;
}

void freqtotime::processdata(void *source, void *destination)
{
	double	*real = (double *)source;
	double	*imag = ((double *)source) + FFT_SAMPLES;
	short	*data = (short *)destination;
	int		i;

	transform.reverse(fdata, real, imag);

	for(i = 0; i < FFT_SAMPLES; i++)
		data[i] = fdata[i] + 0.5;
}

/* ********************************************************************************** */
class lastworker : public worker
{
	BenderApp	*app;
	void	processdata(void *source, void *destination);

public:
			lastworker(BenderApp *bender);
};

lastworker::lastworker(BenderApp *bender)
 : worker("last worker", B_REAL_TIME_PRIORITY, 0, sizeof(short) * FFT_SAMPLES), app(bender)
{
}

void lastworker::processdata(void *source, void * /* destination */)
{
	app->bentdata(source);
}

/* ********************************************************************************** */
/* This is a customized scrollbar used to change the bend factor
 */
class BendBar : public BScrollBar
{
	bender *b;
public:
			BendBar(bender *thebender, BRect frame);

	void	ValueChanged(long newValue);
};

BendBar::BendBar(bender *thebender, BRect frame)
 : BScrollBar(frame, "BendBar", 0, 0, 79, B_HORIZONTAL), b(thebender)
{
}

void BendBar::ValueChanged(long newValue)
{
	b->change(.80 + newValue * .005);
}

/* ********************************************************************************** */
BenderApp::BenderApp()
: BApplication('BNDR'), 
	reader(0), writer(0), dataavailable(0)
{
	BenderWindow	*aWindow;
	BenderView		*aView;
	BRect			aRect;
	BendBar			*bView;
	bender			*b;
	
	fromadc = new short [FFT_SAMPLES];
	todac = new short [FFT_SAMPLES];
	
	last = new lastworker(this);
	step3 = new freqtotime(last);
	step2 = b = new bender(step3);
	step1 = new timetofreq(step2);

	last->start();
	step3->start();
	step2->start();
	step1->start();

	// set up a rectangle and instantiate a new window
	aRect.Set(100, 100, 100 + 200, 100 + 60);
	aWindow = new BenderWindow(aRect);
	
	aView = new BenderView(BRect(0, 0, 200, 60 - B_H_SCROLL_BAR_HEIGHT), "BenderView");
	bView = new BendBar(b, BRect(0, 60 - B_H_SCROLL_BAR_HEIGHT, 200, 60));
		
	// add view to window
	aWindow->AddChild(aView);
	aWindow->AddChild(bView);
	
	// make window visible
	aWindow->Show();
	bView->SetValue(40);
}

BenderApp::~BenderApp(void)
{
	if(reader) 
	{
		reader->ExitStream(TRUE);
		reader->Unsubscribe();
		reader->SetADCInput(prevADC);
		reader->EnableDevice(B_MIC_IN, prevMIC);
		delete reader;
		reader = 0;
	}		
	if(writer) 
	{
		writer->ExitStream(TRUE);
		writer->Unsubscribe();
		writer->EnableDevice(B_LOOPBACK, prevLB);
		delete writer;
		writer = 0;
	}		

	step1->stop();
	step2->stop();
	step3->stop();
	last->stop();

	delete step1;
	delete step2;
	delete step3;
	delete last;

	delete [] fromadc;
	delete [] todac;
}

void BenderApp::ReadyToRun(void)
{
	/* create the audio subscribers
	 */
	reader = new BAudioSubscriber("BenderIn");
	writer = new BAudioSubscriber("BenderOut");
	if(reader && writer)
	{
		long error;
	
		/* Mute MIC
		 */
		prevMIC = reader->IsDeviceEnabled(B_MIC_IN);
		reader->EnableDevice(B_MIC_IN, FALSE);

		/* Turn off loopback, as we will be feeding data via software
		 */		
		prevLB = reader->IsDeviceEnabled(B_LOOPBACK);
		writer->EnableDevice(B_LOOPBACK, FALSE);

		/* Input from MIC
		 */
		prevADC = reader->ADCInput();
		reader->SetADCInput(B_MIC_IN);

		error = reader->Subscribe(B_ADC_STREAM, B_SHARED_SUBSCRIBER_ID, TRUE);
		error = writer->Subscribe(B_DAC_STREAM, writer->ID(), TRUE);
		
		/* Mono, 22050 Hz, more than enough for voice
		 */
		error = reader->SetADCSampleInfo(2, 1, B_BIG_ENDIAN, B_LINEAR_SAMPLES);
		error = writer->SetDACSampleInfo(2, 1, B_BIG_ENDIAN, B_LINEAR_SAMPLES);
		error = reader->SetSamplingRate(22050);

		/* Fewer buffers than default to reduce latency
		 */
		reader->SetStreamBuffers(FFT_SAMPLES * sizeof(short), 2);
		writer->SetStreamBuffers(FFT_SAMPLES * sizeof(short), 2);

		error = reader->EnterStream(NULL, FALSE, be_app, inputstub, NULL, TRUE);
		error = writer->EnterStream(NULL, FALSE, be_app, outputstub, NULL, TRUE);
	}
}

void BenderApp::bentdata(void *data)
{
	memcpy(todac, data, sizeof(short) * FFT_SAMPLES);
	atomic_add(&dataavailable, 1);
}

/* These are the audio stream input and output functions,
 * they shuffle audio data to the fft routines and back
 */
bool BenderApp::input(char *buffer, long /* count */)
{
	void	*buf;
	
	if((buf = step1->acquire_destination(20)) != 0)
	{
		memcpy(buf, buffer, sizeof(short) * FFT_SAMPLES);
		step1->release_destination();
	}
		
	return TRUE;
}

bool BenderApp::output(char *buffer, long count)
{
	short	*data = (short *)buffer;
	long	i;
	
	if(dataavailable)
	{
		for(i = 0; i < count / sizeof(short); i++)
			data[i] += todac[i];
		atomic_add(&dataavailable, -1);
	}
	
	return TRUE;
}
