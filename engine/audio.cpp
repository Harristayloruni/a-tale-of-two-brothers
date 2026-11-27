// audio.cpp - see audio.hpp
// this is the code behind each tool listed in the header

#include "audio.hpp"
#include "resources.hpp"
#include "settings.hpp"
#include <SFML/Audio.hpp>
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>

// anything in here can only be used inside this file
namespace
{
    // how many sound effects can play at the same time
    constexpr size_t pool_size = 16;
    // pool = the 16 reusable sounds, empty until initialise() makes them
    // (made in initialise(), not here, same shutdown-order reason as the window)
    std::unique_ptr<std::array<sf::Sound, pool_size>> pool;
    // next_sound = which of the 16 sounds to use next (lab 3 trick)
    unsigned char next_sound = 0;
    // music = the one music player, empty until initialise() makes it
    std::unique_ptr<sf::Music> music;
    // current_music = the name of the track playing right now
    std::string current_music;
    // the volumes, from 0 to 100
    float music_volume = 60.f;
    float sfx_volume = 80.f;
} // namespace

// Setup
// gets audio ready, called once when the game starts
void Audio::initialise()
{
    // make the 16 reusable sounds
    pool = std::make_unique<std::array<sf::Sound, pool_size>>();
    // make the music player
    music = std::make_unique<sf::Music>();
    // read the saved music volume, or 60 if none is saved, kept between 0 and 100
    music_volume = std::clamp(Settings::get_float("audio.music", 60.f), 0.f, 100.f);
    // read the saved sound effect volume, or 80 if none is saved, kept between 0 and 100
    sfx_volume = std::clamp(Settings::get_float("audio.sfx", 80.f), 0.f, 100.f);
}

// stops everything and cleans up, called once when the game closes
void Audio::shutdown()
{
    // if the music player exists, stop it
    if (music)
        music->stop();
    // if the sounds exist, stop every one of them
    if (pool)
    {
        for (auto &s : *pool)
        {
            s.stop();
        }
    }
    // delete the sounds and the music player
    pool.reset();
    music.reset();
}

// Playing
// plays a sound effect once
void Audio::play_sound(const std::string &path, float pitch)
{
    // if the sounds have not been made yet, do nothing
    if (!pool)
        return;
    // try to play it
    try
    {
        // buffer = the sound file loaded into memory (loaded once, then shared)
        auto buffer = Resources::get<sf::SoundBuffer>(path);
        // s = the next sound in the pool
        sf::Sound &s = (*pool)[next_sound % pool_size];
        // move on to the next one, going back to 0 after 15
        next_sound = static_cast<unsigned char>((next_sound + 1) % pool_size);
        // stop whatever that sound was playing before
        s.stop();
        // give it the new file, pitch and volume, then play it
        s.setBuffer(*buffer);
        s.setPitch(pitch);
        s.setVolume(sfx_volume);
        s.play();
    }
    // if the file was missing, print the error and carry on
    // a missing sound should never crash the game
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }
}

// plays a music track
void Audio::play_music(const std::string &path, bool loop)
{
    // if the music player has not been made yet, do nothing
    if (!music)
        return;
    // if this track is already playing, do nothing
    if (current_music == path && music->getStatus() == sf::Music::Playing)
        return;
    // stop the old track
    music->stop();
    // open the new file, if it will not open print an error and stop here
    // music is read from disk while it plays, so it is not kept in Resources
    if (!music->openFromFile(std::string(Resources::root) + path))
    {
        std::cerr << "Failed to open music: " << path << "\n";
        current_music.clear();
        return;
    }
    // remember which track this is
    current_music = path;
    // set whether it repeats and how loud it is, then play it
    music->setLoop(loop);
    music->setVolume(music_volume);
    music->play();
}

// stops the music
void Audio::stop_music()
{
    // if the music player exists, stop it
    if (music)
        music->stop();
    // forget which track was playing
    current_music.clear();
}

// Volume
// changes the music volume
void Audio::set_music_volume(float volume)
{
    // keep the number between 0 and 100
    music_volume = std::clamp(volume, 0.f, 100.f);
    // if the music player exists, change its volume straight away
    if (music)
        music->setVolume(music_volume);
    // store it in Settings so it gets saved
    Settings::set_float("audio.music", music_volume);
}

// changes the sound effect volume
void Audio::set_sfx_volume(float volume)
{
    // keep the number between 0 and 100
    sfx_volume = std::clamp(volume, 0.f, 100.f);
    // store it in Settings so it gets saved
    Settings::set_float("audio.sfx", sfx_volume);
}

// gives back the music volume
float Audio::get_music_volume()
{
    return music_volume;
}
// gives back the sound effect volume
float Audio::get_sfx_volume()
{
    return sfx_volume;
}