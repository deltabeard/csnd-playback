#include <3ds.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define DR_FLAC_IMPLEMENTATION
#include <dr_flac.h>

#define STACKSIZE (4 * 1024)

#define CHAN_1 8
#define CHAN_2 9

#define AUDIO_FILE "sdmc:/music/test.flac"

struct thread_args {
	atomic_bool run_threads;
	drflac *flac;
};

void play_csnd_thread(void *arg)
{
	struct thread_args *ta = arg;
	drflac_int16 *samples;
	drflac_int16 *buffer1, *buffer2;
	size_t samples_sz;
	size_t pcm_frames_in_each_buf;
       
	/* Allocate enough memory for 1 second of audio. */
	samples_sz = ta->flac->sampleRate * sizeof(drflac_int16);
	samples = malloc(samples_sz);
	if(samples == NULL)
		return;

	/* Set the first buffer to the start of the samples. */
	buffer1 = samples;
	/* Set the second buffer to the second half of the samples. */
	buffer2 = samples + (samples_sz / 2);
	pcm_frames_in_each_buf = (samples_sz / sizeof(drflac_int16)) / 2;

	/* Fill entire buffer with samples. */
	drflac_read_pcm_frames_s16(ta->flac, samples_sz/sizeof(drflac_int16),
			samples);
	/* Begin playing entire buffer. */
	csndPlaySound(CHAN_1, SOUND_REPEAT | SOUND_FORMAT_16BIT | SOUND_ENABLE,
			ta->flac->sampleRate, 1.0f, 0.0f, samples, samples,
			samples_sz/2);

	while(ta->run_threads)
	{
		s64 sleep_dur_ns = 1000 * 500;
		drflac_uint64 ret;

		/* Wait for the first half of the buffer to finish playing. */
		svcSleepThread(sleep_dur_ns);

		/* Replace the first half of the buffer with new audio. */
		ret = drflac_read_pcm_frames_s16(ta->flac,
				pcm_frames_in_each_buf, buffer1);
		/* Return if the end of stream is reached. */
		if(ret <= 0)
			break;

		/* Wait for the second half of the buffer to finish playing. */
		svcSleepThread(sleep_dur_ns);
		ret = drflac_read_pcm_frames_s16(ta->flac,
				pcm_frames_in_each_buf, buffer2);
		if(ret <= 0)
			break;

		/* TODO: Replace svcSleepThread with a more accurate timer. */
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

	ta.flac = drflac_open_file(AUDIO_FILE, NULL);

	if(ta.flac != NULL)
	{
		s32 prio = 0;

		/* Create CSND thread. */
		atomic_init(&ta.run_threads, true);
		svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
		csnd_thread = threadCreate(play_csnd_thread, &ta, STACKSIZE,
				prio-1, -2, false);
		puts("This application will play audio using CSND.");
	}
	else
	{
		puts("Error opening file " AUDIO_FILE);
	}

	if(ta.flac->channels != 1)
		puts("Only single channel flac is supported properly.");

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

	if(ta.flac != NULL)
	{
		atomic_store(&ta.run_threads, false);
		threadJoin(csnd_thread, U64_MAX);
		threadFree(csnd_thread);
		drflac_close(ta.flac);
	}

	csndExit();
	gfxExit();
	return 0;
}
