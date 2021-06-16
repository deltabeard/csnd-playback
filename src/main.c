#include <3ds.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define STACKSIZE (4 * 1024)

#define CHAN_1 8
#define CHAN_2 9

struct thread_args {
	atomic_bool run_threads;
};

void play_csnd_thread(void *arg)
{
	struct thread_args *ta = arg;

	CSND_SetPlayState(CHAN_1, 0);
	CSND_SetPlayState(CHAN_2, 0);
	CSND_UpdateInfo(0);

	while(ta->run_threads)
	{
		const s64 sleep_dur_ns = 1 * 1024;



		svcSleepThread(sleep_dur_ns);
	}
}

int main(int argc, char *argv[])
{
	Thread csnd_thread;
	struct thread_args ta;

	/* These parameters are unused. */
	(void) argc;
	(void) argv;

	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	csndInit();

	/* Create CSND thread. */
	{
		s32 prio = 0;

		atomic_init(&ta.run_threads, true);
		svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
		csnd_thread = threadCreate(play_csnd_thread, &ta, STACKSIZE,
				prio-1, -2, false);
		puts("CSND thread created.");
	}

	puts("This application will play audio using CSND.");
	puts("Press START to quit.");

	/* Main loop. */
	while (aptMainLoop())
	{
		u32 kDown;

		gspWaitForVBlank();
		hidScanInput();

		kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;

		/* Flush and swap framebuffers */
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	atomic_store(&ta.run_threads, false);
	threadJoin(csnd_thread, U64_MAX);
	threadFree(csnd_thread);

	csndExit();
	gfxExit();
	return 0;
}
