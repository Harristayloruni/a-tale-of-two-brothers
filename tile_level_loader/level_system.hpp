// level_system.hpp - loads a room from a text file and draws it
// built as its own library, it only needs SFML, not the engine, lab 4
// lab 4 - LevelSystem class, text file format, get_tile, get_tile_position, get_tile_at
// lab 5 - find_tiles
// lab 7 - get_groups and in_group, joining tiles into rectangles
// so the physics world has fewer bodies
//
// each character in the text file is one tile:
//   w = WALL   space = EMPTY (floor)   s = START   e = END (the exit door)
//   n = ENEMY  + = WAYPOINT
//   c = CHEST  b = BOSS   k = NPC (wounded knight)   <- added for our GDD
//
// this file is the list of level tools, the code behind them is in level_system.cpp

// only include this header once per build
#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

// groups all the level functions under one name
// everything is static because the game only has one room loaded at a time,
// so you call LevelSystem::get_tile() and never create a LevelSystem object
class LevelSystem
{
public:
    // the kinds of tile, so the code can say WALL and not the letter w
    enum Tile
    {
        EMPTY, START, END, WALL, ENEMY, WAYPOINT, CHEST, BOSS, NPC
    };

    // a rectangle of tiles, measured in tiles not pixels (what get_groups gives back)
    struct TileRect
    {
        sf::Vector2i top_left;
        sf::Vector2i size;
    };

    // Loading and drawing

    // read a room from a text file, tile_size = how many pixels wide each tile is
    static void load_level(const std::string &path, float tile_size = 100.f);

    // throw the room away
    static void unload();

    // draw the whole room
    static void render(sf::RenderTarget &target);

    // Colours, lab 4 - used for a tile type that has no picture

    // what colour is this tile type?
    static sf::Color get_color(Tile t);

    // store a colour for this tile type
    static void set_color(Tile t, sf::Color c);

    // Pictures (added) - draw tiles from a tileset image and not flat colours

    // which image the tile pictures are cut from
    static void set_texture(const sf::Texture *texture);

    // which parts of the image this tile type can use
    // if there are several, each square of the room picks one, so floors look less repetitive
    static void set_tile_rects(Tile t, const std::vector<sf::IntRect> &rects);

    // Asking about tiles
    // grid position = which tile, counted in tiles (column, row)
    // screen position = where on the screen, counted in pixels

    // what tile is at this grid position?
    static Tile get_tile(sf::Vector2i grid_pos);

    // where on the screen is the top left corner of this tile?
    static sf::Vector2f get_tile_position(sf::Vector2i p);

    // where on the screen is the middle of this tile? (added)
    static sf::Vector2f get_tile_center(sf::Vector2i p);

    // what tile is at this screen position?
    static Tile get_tile_at(sf::Vector2f screen_pos);

    // which grid position is this screen position inside? (added)
    static sf::Vector2i world_to_grid(sf::Vector2f screen_pos);

    // is this grid position inside the room? (added)
    static bool is_in_bounds(sf::Vector2i grid_pos);

    // can something walk on this tile? (added, used by pathfinding later)
    static bool is_walkable(sf::Vector2i grid_pos);

    // Asking about the room

    // how many tiles tall and wide is the room?
    static int get_height();
    static int get_width();

    // how many pixels wide is one tile?
    static float get_tile_size();

    // where does the wizard start? (the middle of the s tile)
    static sf::Vector2f get_start_position();

    // move the whole room on the screen, e.g. down to leave space for the HUD
    static void set_offset(const sf::Vector2f &offset);
    static const sf::Vector2f &get_offset();

    // is a room loaded right now?
    static bool is_loaded();

    // Finding tiles

    // give back the grid position of every tile of this type, lab 5
    static std::vector<sf::Vector2i> find_tiles(Tile type);

    // is this grid position in this list? lab 7
    static bool in_group(const sf::Vector2i &pos, const std::vector<sf::Vector2i> &group);

    // join touching tiles of this type into rectangles, lab 7
    static std::vector<TileRect> get_groups(Tile type);

protected:
    // the tiles of the room, all in one long list, row after row, lab 4
    static std::unique_ptr<Tile[]> _tiles;

    // how many tiles wide and tall the room is
    static int _width;
    static int _height;

    // how far the room is moved on the screen
    static sf::Vector2f _offset;

    // how many pixels wide one tile is
    static float _tile_size;

    // the colour for each tile type
    static std::map<Tile, sf::Color> _colors;

    // where the wizard starts
    static sf::Vector2f _start_position;

    // every corner of every tile, in one list, so the whole room is one draw call
    static sf::VertexArray _vertices;

    // the tileset image
    static const sf::Texture *_texture;

    // which parts of the image each tile type can use
    static std::map<Tile, std::vector<sf::IntRect>> _rects;

    // rebuild the list of corners, called whenever the room or its look changes
    static void build_sprites();

private:
    // static only, never create or destroy one
    LevelSystem() = delete;
    ~LevelSystem() = delete;
};
