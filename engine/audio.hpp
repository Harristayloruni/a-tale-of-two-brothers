// audio.hpp - sound effects and background music
// added, no audio lab - coursework needs some form of sound effect or music
// GDD media list: background music, "oof" when damaged, drinking sound
// sound effects use an object pool, same idea as the bullet pool in
// lab 3 - a fixed set of sounds that get reused,
// so playing a sound never has to create a new one
//
// this file is the list of audio tools, the code behind them is in audio.cpp

#pragma once // only include this header once per build
#include <string>

// groups all the audio functions under one name
// everything is static because the game only has one audio system,
// so you call Audio::play_sound() and never create an Audio object
class Audio
{
public:
    Audio() = delete; // static only, never create one

    // Setup
    static void initialise(); // Get ready to play audio from Settings
    static void shutdown();   // stops everything and cleans up when the game closes

    // Playing
    // path = which file to play, e.g. "sounds/hit.wav"

    // pitch = how high or low it sounds, 1 is normal
    static void play_sound(const std::string &path, float pitch = 1.f); // play this sound effect once

    // loop = true means start again when the track ends
    static void play_music(const std::string &path, bool loop = true); // play this music track
    static void stop_music();                                          // stop the music

    // Volume
    // volume goes from 0 (silent) to 100 (full)
    static void set_music_volume(float volume); // store and use this music volume
    static void set_sfx_volume(float volume);   // store and use this sound effect volume
    static float get_music_volume();            // what is the music volume?
    static float get_sfx_volume();              // what is the sound effect volume?
};
