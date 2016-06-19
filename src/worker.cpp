#include "worker.h"

worker::worker(char *name, long pri, worker *target, long datasize)
 : tgt(target), sz(datasize), firstused(0), firstfree(0), havetogo(FALSE)
{
	int	i;
	
	for(i = 0; i < WORKER_BUFFER_COUNT; i++)
		data[i] = new char [sz];

	running = create_sem(0, "running");

	wname = new char [strlen(name) + 1];
	strcpy(wname, name);
	wpri = pri;
}

worker::~worker(void)
{
	int	i;

	delete_sem(running);
	
	for(i = 0; i < WORKER_BUFFER_COUNT; i++)
		delete [] data[i];
}

void worker::start(void)
{
	freebuffers = create_sem(2, "free buffers");	
	usedbuffers = create_sem(0, "used buffers");
	release_sem(running);
	wid = spawn_thread(workerthreadstub, wname, wpri, this);
	resume_thread(wid);
}

void worker::stop(void)
{
	/* Flag termination
	 */
	havetogo = TRUE;

	/* Delete semaphores to force termination... ugly
	 */
	delete_sem(usedbuffers);
	delete_sem(freebuffers);

	/* Wait for end
	 */
	acquire_sem(running);
}

void *worker::acquire_source(void)
{
	void	*ret = 0;
	
	if(acquire_sem(usedbuffers) == B_NO_ERROR)
		ret = data[firstused % WORKER_BUFFER_COUNT];
		
	return ret;
}

void worker::release_source(void)
{
	atomic_add(&firstused, 1);

	release_sem(freebuffers);
}

void *worker::acquire_destination(long timeout)
{
	void	*ret = 0;

	if(timeout == 0)
	{
		if(acquire_sem(freebuffers) == B_NO_ERROR)
			ret = data[firstfree % WORKER_BUFFER_COUNT];
	}
	else
	{
		if(acquire_sem_timeout(freebuffers, timeout) == B_NO_ERROR)
			ret = data[firstfree % WORKER_BUFFER_COUNT];
	}
		
	return ret;
}

void worker::release_destination(void)
{
	atomic_add(&firstfree, 1);
	
	release_sem(usedbuffers);
}	

long worker::workerthread(void)
{
	void	*source;
	void	*dest;

	while(! havetogo)
	{
		source = acquire_source();
		if(source)
		{
			dest = tgt ? tgt->acquire_destination() : 0;

			processdata(source, dest);

			release_source();
			if(tgt)
				tgt->release_destination();
		}
	}

	release_sem(running);

	return 0;
}
