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
custom deleter for smart pointers containing openal-soft audio buffers; deletes the buffer's contents before deleting the buffer itself
@param b: the audio buffer to delete
*/
void deleteAudioBuffer(ALuint* b) {
	alDeleteBuffers(1, b);
	delete b;
}

/*
load the sound file with the specified name
@param soundName: the name of the sound file to load (including the extension; must be .ogg for now)
@returns: the shared pointer in the sounds map to the sound
*/
std::shared_ptr<ALuint> loadSound(std::string soundName) {
	// check if the sound has already been loaded
	std::unordered_map<std::string, std::shared_ptr<ALuint>>::iterator search = sounds.find(soundName);
	if (search != sounds.end())
		return search->second;
	
	// load sound file via stb_vorbis
	int numChannels, sample_rate;
	ALshort* output;
	int slen = stb_vorbis_decode_filename(soundName.c_str(), &numChannels, &sample_rate, &output);

	// convert sound data into an ALuint buffer
	// TODO: handle remaining formats
	// TODO: fail explicitly on > 2 channels
	ALenum format = (numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, output, sizeof(ALshort) * slen * numChannels, sample_rate);
	
	// create source to play the buffered sound data
	std::shared_ptr<ALuint> source(new ALuint(0), deleteAudioBuffer);
	alGenSources(1, &*source);
	alSourcei(*source, AL_BUFFER, buffer);
	if (alGetError() != AL_NO_ERROR) ERROR(std::cout << "Failed to load and/or play sound '" << soundName << "'" << std::endl);
	
	// now cleanup, add to the sound map, and return
	alDeleteBuffers(1,&buffer);
	sounds.insert({ soundName, source });
	return source;
}

/*play the specified sound
@param soundName: the name of the sound to play
*/
void playSound(std::string soundName) {
	std::unordered_map<std::string, std::shared_ptr<ALuint>>::iterator search = sounds.find(soundName);
	std::shared_ptr<ALuint> buffer = (search == sounds.end() ? loadSound(soundName) : search->second);
	alSourcePlay(*buffer);
}