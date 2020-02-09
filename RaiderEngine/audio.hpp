#pragma once
#include "stdafx.h"

/*
initialize audio via OpenAL-Soft
*/
void initAudio();

void closeAudio();

/*
custom deleter for smart pointers containing openal-soft audio buffers; deletes the buffer's contents before deleting the buffer itself
@param b: the audio buffer to delete
*/
void deleteAudioBuffer(ALuint* b);

/*
load the sound file with the specified name
@param soundName: the name of the sound file to load (including the extension; must be .ogg for now)
@returns: the shared pointer in the sounds map to the sound
*/
ALuint loadSound(std::string soundName);

/*play the specified sound
@param soundName: the name of the sound to play
*/
void playSound(std::string soundName);