/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "qcommon/q_shared.h"
#include "client/client.h"
#include "client/snd_local.h"

extern dma_t		dma;
SDL_AudioDeviceID	dev;
qboolean snd_inited = qfalse;

cvar_t *s_sdlBits;
cvar_t *s_sdlSpeed;
cvar_t *s_sdlChannels;
cvar_t *s_sdlDevSamps;
cvar_t *s_sdlMixSamps;
extern cvar_t* s_maxSounds;

/* The audio callback. All the magic happens here. */
static int dmapos = 0;
static int dmasize = 0;

/*
===============
SNDDMA_AudioCallback
===============
*/
static void SNDDMA_AudioCallback(void *userdata, Uint8 *stream, int len)
{
	int pos = (dmapos * (dma.samplebits/8));
	if (pos >= dmasize)
		dmapos = pos = 0;

	if (!snd_inited)  /* shouldn't happen, but just in case... */
	{
		memset(stream, '\0', len);
		return;
	}
	else
	{
		int tobufend = dmasize - pos;  /* bytes to buffer's end. */
		int len1 = len;
		int len2 = 0;

		if (len1 > tobufend)
		{
			len1 = tobufend;
			len2 = len - len1;
		}
		memcpy(stream, dma.buffer + pos, len1);
		if (len2 <= 0)
			dmapos += (len1 / (dma.samplebits/8));
		else  /* wraparound? */
		{
			memcpy(stream+len1, dma.buffer, len2);
			dmapos = (len2 / (dma.samplebits/8));
		}
	}

	if (dmapos >= dmasize)
		dmapos = 0;
}

static struct
{
	Uint16		enumFormat;
	const char	*stringFormat;
} formatToStringTable[ ] =
{
	{ AUDIO_U8,     "AUDIO_U8" },
	{ AUDIO_S8,     "AUDIO_S8" },
	{ AUDIO_U16LSB, "AUDIO_U16LSB" },
	{ AUDIO_S16LSB, "AUDIO_S16LSB" },
	{ AUDIO_U16MSB, "AUDIO_U16MSB" },
	{ AUDIO_S16MSB, "AUDIO_S16MSB" },
	{ AUDIO_S32LSB, "AUDIO_S32LSB" },
	{ AUDIO_S32MSB, "AUDIO_S32MSB" },
	{ AUDIO_F32LSB, "AUDIO_F32LSB" },
	{ AUDIO_F32MSB, "AUDIO_F32MSB" }
};

static const size_t formatToStringTableSize = ARRAY_LEN( formatToStringTable );

/*
===============
SNDDMA_PrintAudiospec
===============
*/
static void SNDDMA_PrintAudiospec(const char *str, const SDL_AudioSpec *spec)
{
	const char	*fmt = NULL;

	Com_Printf( "%s:\n", str );

	for( size_t i = 0; i < formatToStringTableSize; i++ ) {
		if( spec->format == formatToStringTable[ i ].enumFormat ) {
			fmt = formatToStringTable[ i ].stringFormat;
		}
	}

	if( fmt ) {
		Com_Printf( "  Format:   %s\n", fmt );
	} else {
		Com_Printf( "  Format:   " S_COLOR_RED "UNKNOWN (%d)\n", (int)spec->format);
	}

	Com_Printf( "  Freq:     %d\n", (int) spec->freq );
	Com_Printf( "  Samples:  %d\n", (int) spec->samples );
	Com_Printf( "  Channels: %d\n", (int) spec->channels );
}

static int SNDDMA_ExpandSampleFrequencyKHzToHz(int khz)
{
	switch (khz)
	{
		default:
		case 44: return 44100;
		case 22: return 22050;
		case 11: return 11025;
	}
}
/*
================
Initialize the soundBuffer 
================
*/
void SNDDMA_InitSoundBuffer(int maxSounds) 
{
	sb.soundCount = 0;
	sb.lastReset = 0;
	sb.maxSoundsPerSec = maxSounds;
}

/*
===============
SNDDMA_Init
===============
*/
qboolean SNDDMA_Init(int sampleFrequencyInKHz)
{
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
	int tmp;
	s_maxSounds = Cvar_Get("s_maxSounds", "50", CVAR_ARCHIVE);
	SNDDMA_InitSoundBuffer(s_maxSounds->integer);

	if (snd_inited)
		return qtrue;

	if (!s_sdlBits) {
		s_sdlBits = Cvar_Get("s_sdlBits", "16", CVAR_ARCHIVE_ND);
		s_sdlChannels = Cvar_Get("s_sdlChannels", "2", CVAR_ARCHIVE_ND);
		s_sdlDevSamps = Cvar_Get("s_sdlDevSamps", "0", CVAR_ARCHIVE_ND);
		s_sdlMixSamps = Cvar_Get("s_sdlMixSamps", "0", CVAR_ARCHIVE_ND);
	}

	Com_Printf( "SDL_Init( SDL_INIT_AUDIO )... " );

	if (!SDL_WasInit(SDL_INIT_AUDIO))
	{
		if (SDL_Init(SDL_INIT_AUDIO) == -1)
		{
			Com_Printf( "FAILED (%s)\n", SDL_GetError( ) );
			return qfalse;
		}
	}

	Com_Printf( "OK\n" );

	Com_Printf( "SDL audio driver is \"%s\".\n", SDL_GetCurrentAudioDriver( ) );

	memset(&desired, '\0', sizeof (desired));
	memset(&obtained, '\0', sizeof (obtained));

	tmp = ((int) s_sdlBits->value);
	if ((tmp != 16) && (tmp != 8))
		tmp = 16;

	desired.freq = SNDDMA_ExpandSampleFrequencyKHzToHz(sampleFrequencyInKHz);
	desired.format = ((tmp == 16) ? AUDIO_S16SYS : AUDIO_U8);

	// I dunno if this is the best idea, but I'll give it a try...
	//  should probably check a cvar for this...
	if (s_sdlDevSamps->value)
		desired.samples = s_sdlDevSamps->value;
	else
	{
		// just pick a sane default.
		if (desired.freq <= 11025)
			desired.samples = 256;
		else if (desired.freq <= 22050)
			desired.samples = 512;
		else if (desired.freq <= 44100)
			desired.samples = 1024;
		else
			desired.samples = 2048;  // (*shrug*)
	}

	desired.channels = (int) s_sdlChannels->value;
	desired.callback = SNDDMA_AudioCallback;

	dev = SDL_OpenAudioDevice( NULL, 0, &desired, &obtained, 0 );
	if ( !dev )
	{
		Com_Printf("SDL_OpenAudioDevice() failed: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return qfalse;
	}

	SNDDMA_PrintAudiospec("SDL_AudioSpec", &obtained);

	// dma.samples needs to be big, or id's mixer will just refuse to
	//  work at all; we need to keep it significantly bigger than the
	//  amount of SDL callback samples, and just copy a little each time
	//  the callback runs.
	// 32768 is what the OSS driver filled in here on my system. I don't
	//  know if it's a good value overall, but at least we know it's
	//  reasonable...this is why I let the user override.
	tmp = s_sdlMixSamps->value;
	if (!tmp)
		tmp = (obtained.samples * obtained.channels) * 10;

	if (tmp & (tmp - 1))  // not a power of two? Seems to confuse something.
	{
		int val = 1;
		while (val < tmp)
			val <<= 1;

		tmp = val;
	}

	dmapos = 0;
	dma.samplebits = obtained.format & 0xFF;  // first byte of format is bits.
	dma.channels = obtained.channels;
	dma.samples = tmp;
	dma.submission_chunk = 1;
	dma.speed = obtained.freq;
	dmasize = (dma.samples * (dma.samplebits/8));
	dma.buffer = (byte *)calloc(1, dmasize);

	Com_Printf("Starting SDL audio callback...\n");
	SDL_PauseAudioDevice(dev, 0);  // start callback.

	Com_Printf("SDL audio initialized.\n");
	snd_inited = qtrue;
	return qtrue;
}

/*
===============
SNDDMA_GetDMAPos
===============
*/
int SNDDMA_GetDMAPos(void)
{
	return dmapos;
}

/*
===============
SNDDMA_Shutdown
===============
*/
void SNDDMA_Shutdown(void)
{
	Com_Printf("Closing SDL audio device...\n");
	SDL_PauseAudioDevice(dev, 1);
	SDL_CloseAudioDevice(dev);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	free(dma.buffer);
	dma.buffer = NULL;
	dmapos = dmasize = 0;
	snd_inited = qfalse;
	Com_Printf("SDL audio device shut down.\n");
}

/*
===============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
	SDL_UnlockAudioDevice(dev);
}

/*
===============
SNDDMA_BeginPainting
===============
*/
void SNDDMA_BeginPainting (void)
{
	SDL_LockAudioDevice(dev);
}

#ifdef USE_OPENAL
extern int s_UseOpenAL;
#endif

// (De)activates sound playback
void SNDDMA_Activate( qboolean activate )
{
#ifdef USE_OPENAL
	if ( s_UseOpenAL )
	{
		S_AL_MuteAllSounds( (qboolean)!activate );
	}
#endif

	if ( activate )
	{
		S_ClearSoundBuffer();
	}

	SDL_PauseAudioDevice( dev, !activate );
}
