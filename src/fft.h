
class fft
{
	int	N;
	int P;
	short *pertable;
	double *ctable, *stablet, *stablei;

	int    permute(int n);
public:
			fft(int n);
			~fft(void);
	int		size(void)	{ return N; }
	void	forward(double *data, double *real, double *imag);
	void	reverse(double *data, double *real, double *imag);
};
