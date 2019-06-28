#pragma once
#include "stdafx.h"
#include "alhelpers.h"
#include "settings.hpp"
#include <stdio.h>
#include <assert.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

/*
initialize audio via OpenAL-Soft
*/
void initAudio() {
	InitAL(NULL, NULL);
}

/*
load the sound file with the specified name
@param soundName: the name of the sound file to load (including the extension; must be .ogg for now)
@returns: the shared pointer in the sounds map to the sound
*/
std::shared_ptr<ALuint> loadSound(std::string soundName) {
	std::unordered_map<std::string, std::shared_ptr<ALuint>>::iterator search = sounds.find(soundName);
	if (search != sounds.end())
		return search->second;
	// load file via stb_vorbis
	ALshort* output;
	int numChannels, sample_rate;
	int slen = stb_vorbis_decode_filename(soundName.c_str(), &numChannels, &sample_rate, &output);

	// convert data to an ALuint buffer
	ALuint buffer = 0;
	// TODO: handle remaining formats
	ALenum format = (numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
	// TODO: fail explicitly on > 2 channels

	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, output, sizeof(ALshort) * slen * numChannels, sample_rate);
	
	// create source with which to play the sound
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	if (alGetError() != AL_NO_ERROR) ERROR(std::cout << "Failed to load and/or play sound '" << soundName << "'" << std::endl);
	alDeleteBuffers(1,&buffer);
	
	//sounds.insert({ soundName, source });
	// TODO: should be a bit more efficient if we make source a shared_ptr from the start
	return std::make_shared<ALuint>(source);
}

/*play the specified sound
@param soundName: the name of the sound to play
*/
void playSound(std::string soundName) {
	std::unordered_map<std::string, std::shared_ptr<ALuint>>::iterator search = sounds.find(soundName);
	std::shared_ptr<ALuint> buffer = (search == sounds.end() ? loadSound(soundName) : search->second);
	alSourcePlay(*buffer);
}