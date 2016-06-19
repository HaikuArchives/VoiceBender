
#if ! defined __WORKER_H
#define __WORKER_H 

#define WORKER_BUFFER_COUNT 2

class worker
{
	sem_id	running;
	
	sem_id	freebuffers;			/* free buffers */
	sem_id	usedbuffers;			/* processing stops on this */
	long	firstused;
	long	firstfree;
	void	*data[WORKER_BUFFER_COUNT];	/* work data */
	long	sz;
	
	char		*wname;
	long		wpri;
	thread_id	wid;
	
	volatile bool	havetogo;
	worker	*tgt;

	static long	workerthreadstub(void *data) { return ((worker *)data)->workerthread(); }
	long	workerthread(void);

protected:
	void	*acquire_source(void);
	void	release_source(void);
	virtual void processdata(void *source, void *destination) = 0;

public:
			worker(char *name, long pri, worker *target, long datasize);
	virtual	~worker(void);

	void	*acquire_destination(long timeout = 0);
	void	release_destination(void);

	void	start(void);
	void	stop(void);
};

#endif