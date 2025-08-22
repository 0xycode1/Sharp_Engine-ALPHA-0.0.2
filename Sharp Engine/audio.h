#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <fstream>

#include <al.h>
#include <alc.h>
#include <sndfile.h>

#include "objectRender.h" // POSSIBLE if-else condition to fix stopping program when the sound is playing


void soundPlayer(const char *filename) {
	ALCdevice* device = alcOpenDevice(nullptr);

	if (!device) {
		std::cerr << "Can`t open audio device." << std::endl;

		return;
	}

	ALCcontext* context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);

	SF_INFO sfInfo;
	SNDFILE* sndFile = sf_open(filename, SFM_READ, &sfInfo);

	if (!sndFile) {
		std::cerr << "Can`t open audio file." << std::endl;

		return;
	}

	ALuint buffer;
	alGenBuffers(1, &buffer);

	std::vector<short> samples(sfInfo.frames * sfInfo.channels);

	sf_readf_short(sndFile, samples.data(), sfInfo.frames);
	sf_close(sndFile);

	ALenum format;

	if (sfInfo.channels == 1)
		format = AL_FORMAT_MONO16;
	else if (sfInfo.channels == 2)
		format = AL_FORMAT_STEREO16;
	else
		return;


	alBufferData(buffer, format, samples.data(), samples.size() * sizeof(short), sfInfo.samplerate);

	ALuint source;
	alGenSources(1, &source);

	alSourcei(source, AL_BUFFER, buffer);

	//alSourcei(source, AL_LOOPING, AL_TRUE); // loop options


	alSourcePlay(source);


	ALint state;

	do {
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	} while (state == AL_PLAYING);

	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}


#endif // Audio.h