#include <math.h>
#include "fft.h"

fft::fft(int bits)
{
	int    i,y;
	double v;

	N = 1 << bits;
	P = bits;
	pertable = new short[N];
	ctable = new double[N];
	stablet = new double[N];
	stablei = new double[N];
	
	for(i=0;i<N;i++)
		pertable[i]=permute(i);
	v = 2.0*PI/(double)N;
	for(i=0;i<N/2;i++)
	{
		y=pertable[i*2];
		ctable[i]=cos(v*y);
		stablet[i]=-sin(v*y);
		stablei[i]=sin(v*y);
	}
}

fft::~fft(void)
{
	delete [] pertable;
	delete [] ctable;
	delete [] stablet;
	delete [] stablei;
}

int fft::permute(int n)
{
	int 	ac,
			i;

	ac = 0;
	for(i = 0; i < P; i++)
	{
		ac <<= 1;
		if((n & 1) == 1)
			ac |= 1;
		n >>= 1;
	}

	return ac;
}

void fft::forward(double *data, double *real, double *imag)
{
	unsigned i1,i2,i3,i4;
	int loop,loop1,loop2;
	double a1,a2,b1,b2,z1,z2,v,tr,ti;

	for(loop = 0; loop < N; loop++)
	{
		real[loop] = data[loop];
		imag[loop] = 0;
	}

    i1 = N >> 1;
	i2 = 1;
	v = 2.0*PI/(double)N;
	for(loop = 0; loop < P; loop++)
	{
		i3 = 0;
		i4 = i1;
		for(loop1 = 0; loop1 < i2; loop1++)
		{
			z1 =  ctable[loop1];
			z2 =  stablet[loop1];
			for(loop2 = i3; loop2 < i4; loop2++)
			{
				a1 = real[loop2];
				a2 = imag[loop2];
				b1 = z1*real[loop2+i1] - z2*imag[loop2+i1];
				b2 = z2*real[loop2+i1] + z1*imag[loop2+i1];
				real[loop2] 	 = a1 + b1;
				imag[loop2] 	 = a2 + b2;
				real[loop2 + i1] = a1 - b1;
				imag[loop2 + i1] = a2 - b2;
			}
			i3 += (i1<<1);
			i4 += (i1<<1);
		}
		i1 >>= 1;
		i2 <<= 1;
	}
	for(loop=0;loop<N;loop++)
	{
		loop2=pertable[loop];
		if (loop<loop2)
		{
			tr=real[loop];
			ti=imag[loop];
			real[loop]=real[loop2];
			imag[loop]=imag[loop2];
			real[loop2]=tr;
			imag[loop2]=ti;
		}
	}
}

void fft::reverse(double *data, double *real, double *imag)
{
	unsigned i1,i2,i3,i4;
	int loop,loop1,loop2;
	double a1,a2,b1,b2,z1,z2,v;

	i1 = N >> 1;
	i2 = 1;
	v  = 2*PI/(double)N;
	for(loop = 0; loop < P; loop++)
	{
		i3 = 0;
		i4 = i1;
		for(loop1 = 0; loop1 < i2; loop1++)
		{
			z1 = ctable[loop1];
			z2 = stablei[loop1];
			for(loop2 = i3; loop2 < i4; loop2++)
			{
				a1 = real[loop2];
				a2 = imag[loop2];
				b1 = z1*real[loop2+i1] - z2*imag[loop2+i1];
				b2 = z2*real[loop2+i1] + z1*imag[loop2+i1];
				real[loop2] 	 = a1 + b1;
				imag[loop2] 	 = a2 + b2;
				real[loop2 + i1] = a1 - b1;
				imag[loop2 + i1] = a2 - b2;
			}
			i3 += (i1<<1);
			i4 += (i1<<1);
		}
		i1 >>= 1;
		i2 <<= 1;
	}
  
	for(loop = 0; loop < N; loop++)
		data[loop] = real[pertable[loop]];
}              

