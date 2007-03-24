#define	DEBUG
#include <ucommon/ucommon.h>

#include <stdio.h>

using namespace UCOMMON_NAMESPACE;

class mythread : public Thread
{
public:
	inline mythread() : Thread() {};

	void run(void);
};

void mythread::run(void)
{
	printf("starting...\n");
	suspend(10000);
	printf("finishing\n");
};

extern "C" int main()
{
	mythread *thr = new mythread();

	printf("before\n");
	thr->start();
	suspend(1000);
	thr->release();
	printf("joining\n");
	delete thr;
	printf("ending\n");
};
