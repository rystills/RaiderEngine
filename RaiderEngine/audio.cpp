#include "stdafx.h"
#include "alhelpers.hpp"
#include <stdio.h>
#include <assert.h>
#include "settings.hpp"
#include "audio.hpp"
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#include "terminalColors.hpp"

void initAudio() {
	InitAL(NULL, NULL);
}

void closeAudio() {
	CloseAL();
}

void deleteAudioBuffer(ALuint* b) {
	alDeleteBuffers(1, b);
	delete b;
}

ALuint loadSound(std::string soundName) {
	std::cout << "Loading sound '" << soundName << "'" << std::endl;
	// check if the sound has already been loaded
	if (sounds.contains(soundName))
		return *sounds[soundName];

	// load sound file via stb_vorbis
	double sTime = glfwGetTime();
	int numChannels, sample_rate;
	ALshort* output;
	std::ifstream input(soundDir + soundName, std::ios::binary | std::ios::ate);
	std::vector<unsigned char> vec(input.tellg());
	input.seekg(0, std::ios::beg);
	input.read((char*)vec.data(), vec.size());
	int slen = stb_vorbis_decode_memory(&vec[0], vec.size(), &numChannels, &sample_rate, &output);

	// convert sound data into an ALuint buffer
	// TODO: handle remaining formats
	// TODO: fail explicitly on > 2 channels
	ALenum format = (numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, output, sizeof(ALshort) * slen * numChannels, sample_rate);

	// create source to play the buffered sound data
	ALuint* source = new ALuint(0);
	alGenSources(1, source);
	alSourcei(*source, AL_BUFFER, buffer);
	if (alGetError() != AL_NO_ERROR) ERRORCOLOR(std::cout << "Failed to load and/or play sound '" << soundName << "'" << std::endl);

	// now cleanup, add to the sound map, and return
	alDeleteBuffers(1, &buffer);
	sounds.insert({ soundName, std::unique_ptr<ALuint, std::function<void(ALuint*)>>(source, std::bind(&deleteAudioBuffer,std::placeholders::_1)) });
	SUCCESSCOLOR(std::cout << "Finished loading sound '" << soundName << "' in " << glfwGetTime() - sTime << " seconds" << std::endl);
	return *source;
}

void playSound(std::string soundName) {
	alSourcePlay(sounds.contains(soundName) ? *sounds[soundName] : loadSound(soundName));
}