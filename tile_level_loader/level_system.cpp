// level_system.cpp - see level_system.hpp, lab 4, lab 5, lab 7
// this is the code behind each tool listed in the header

#include "level_system.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// the header said these exist, these lines actually create them
std::unique_ptr<LevelSystem::Tile[]> LevelSystem::_tiles;
int LevelSystem::_width = 0;
int LevelSystem::_height = 0;
sf::Vector2f LevelSystem::_offset(0.0f, 0.0f);
float LevelSystem::_tile_size(100.f);
sf::Vector2f LevelSystem::_start_position(0.f, 0.f);
sf::VertexArray LevelSystem::_vertices(sf::Quads);
const sf::Texture *LevelSystem::_texture = nullptr;
std::map<LevelSystem::Tile, std::vector<sf::IntRect>> LevelSystem::_rects;

// the default colours, walls are dark purple and the exit is brown
std::map<LevelSystem::Tile, sf::Color> LevelSystem::_colors{
    {WALL, sf::Color(70, 62, 80)}, {END, sf::Color(170, 120, 40)}};

// Asking about the room

// gives back how many tiles tall the room is
int LevelSystem::get_height()
{
    return _height;
}

// gives back how many tiles wide the room is
int LevelSystem::get_width()
{
    return _width;
}

// gives back how many pixels wide one tile is
float LevelSystem::get_tile_size()
{
    return _tile_size;
}

// gives back where the wizard starts
sf::Vector2f LevelSystem::get_start_position()
{
    return _start_position;
}

// gives back how far the room is moved on the screen
const sf::Vector2f &LevelSystem::get_offset()
{
    return _offset;
}

// a room is loaded if the list of tiles exists
bool LevelSystem::is_loaded()
{
    return _tiles != nullptr;
}

// moves the whole room on the screen
void LevelSystem::set_offset(const sf::Vector2f &offset)
{
    _offset = offset;

    // if a room is loaded, rebuild it in the new place
    if (is_loaded())
        build_sprites();
}

// Colours

// gives back the colour for a tile type
sf::Color LevelSystem::get_color(LevelSystem::Tile t)
{
    // look for the tile type in the colour list
    // find(), not operator[], lab 4
    auto it = _colors.find(t);

    // if it was not found, give back see-through
    if (it == _colors.end())
    {
        return sf::Color::Transparent;
    }

    // otherwise give back its colour
    return it->second;
}

// stores a colour for a tile type
void LevelSystem::set_color(LevelSystem::Tile t, sf::Color c)
{
    _colors[t] = c;

    // if a room is loaded, rebuild it with the new colour
    if (is_loaded())
        build_sprites();
}

// Pictures

// stores which image the tile pictures are cut from
void LevelSystem::set_texture(const sf::Texture *texture)
{
    _texture = texture;

    // if a room is loaded, rebuild it with the new image
    if (is_loaded())
        build_sprites();
}

// stores which parts of the image a tile type can use
void LevelSystem::set_tile_rects(Tile t, const std::vector<sf::IntRect> &rects)
{
    _rects[t] = rects;

    // if a room is loaded, rebuild it with the new pictures
    if (is_loaded())
        build_sprites();
}

// Loading and drawing

// reads a room from a text file
void LevelSystem::load_level(const std::string &path, float tile_size)
{
    _tile_size = tile_size;

    // open the file, if it will not open stop with an error
    std::ifstream f(path);
    if (!f.good())
    {
        throw std::runtime_error("Couldn't open level file: " + path);
    }

    // read the whole file into one piece of text called buffer, lab 4
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string buffer = ss.str();

    // w = the room's width, h = how many rows so far, x = how far along the current row
    int w = 0, h = 0, x = 0;

    // temp_tiles = the tiles we have read so far
    std::vector<Tile> temp_tiles;

    // go through the text one character at a time
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        const char c = buffer[i];

        // skip the extra line ending character Windows adds (git can convert files to it)
        if (c == '\r')
        {
            continue;
        }

        // if this is the end of a line
        if (c == '\n')
        {
            // if the line was blank, ignore it
            if (x == 0)
                continue;

            // if this is the first row, its length sets the room's width
            if (w == 0)
            {
                w = x;
            }

            // otherwise, if this row is a different length, stop with an error naming the row
            else if (x != w)
            {
                throw std::runtime_error("Level row " + std::to_string(h) + " has the wrong width in " + path);
            }

            // start a new row
            x = 0;
            h++;
            continue;
        }

        // turn the character into a tile type, starting with floor
        Tile t = EMPTY;
        switch (c)
        {
        case 'w':
            t = WALL;
            break;
        case 's':
            t = START;
            break;
        case 'e':
            t = END;
            break;
        case ' ':
            t = EMPTY;
            break;
        case '+':
            t = WAYPOINT;
            break;
        case 'n':
            t = ENEMY;
            break;
        case 'c':
            t = CHEST;
            break;
        case 'b':
            t = BOSS;
            break;
        case 'k':
            t = NPC;
            break;

        // any other character is treated as floor, with a warning
        default:
            std::cout << "Unknown tile '" << c << "' in " << path << " (treated as floor)\n";
        }

        // add the tile to the list and move along the row
        temp_tiles.push_back(t);
        x++;
    }

    // if the file did not end with a new line, count the last row too, lab 4
    if (x != 0)
    {
        if (w == 0)
            w = x;
        h++;
    }

    // if nothing was read, or the number of tiles is not width times height, stop with an error
    if (w == 0 || static_cast<int>(temp_tiles.size()) != w * h)
    {
        throw std::runtime_error("Can't parse level file " + path);
    }

    // make the real list of tiles the right size and copy the tiles into it
    _tiles = std::make_unique<Tile[]>(static_cast<size_t>(w * h));
    _width = w;
    _height = h;
    std::copy(temp_tiles.begin(), temp_tiles.end(), &_tiles[0]);

    // work out where the wizard starts, the middle of the first s tile, lab 4
    _start_position = _offset;
    const auto starts = find_tiles(START);
    if (!starts.empty())
        _start_position = get_tile_center(starts.front());

    std::cout << "Level " << path << " Loaded. " << w << "x" << h << std::endl;

    // build the list of corners so the room can be drawn
    build_sprites();
}

// throws the room away
void LevelSystem::unload()
{
    _tiles.reset();
    _width = 0;
    _height = 0;
    _vertices.clear();
}

// builds the list of corners for every tile, 4 corners per tile
void LevelSystem::build_sprites()
{
    // empty the old list, each group of 4 corners is one square
    _vertices.clear();
    _vertices.setPrimitiveType(sf::Quads);

    // for every row, and every tile in that row
    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
        {
            // t = what kind of tile this is, p = where its top left corner is on the screen
            const Tile t = get_tile({x, y});
            const sf::Vector2f p = get_tile_position({x, y});

            // look for pictures for this tile type
            auto it = _rects.find(t);

            // if there is an image and this tile type has at least one picture
            if (_texture != nullptr && it != _rects.end() && !it->second.empty())
            {
                // v = the pictures this tile type can use
                const auto &v = it->second;

                // mix x and y into one big number called h
                // the same square always gets the same number, but there is no visible pattern
                unsigned int h = static_cast<unsigned int>(x) * 73856093u ^ static_cast<unsigned int>(y) * 19349663u;
                h ^= h >> 13;
                h *= 0x5bd1e995u;
                h ^= h >> 15;

                // use that number to pick one of the pictures
                const sf::IntRect r = v[h % v.size()];

                // l, tp, rr, b = the left, top, right and bottom edges of the picture in the image
                const float l = static_cast<float>(r.left), tp = static_cast<float>(r.top);
                const float rr = l + static_cast<float>(r.width), b = tp + static_cast<float>(r.height);

                // add the 4 corners of the tile, each one matched to a corner of the picture
                _vertices.append(sf::Vertex(p, sf::Vector2f(l, tp)));
                _vertices.append(sf::Vertex({p.x + _tile_size, p.y}, sf::Vector2f(rr, tp)));
                _vertices.append(sf::Vertex({p.x + _tile_size, p.y + _tile_size}, sf::Vector2f(rr, b)));
                _vertices.append(sf::Vertex({p.x, p.y + _tile_size}, sf::Vector2f(l, b)));

                // this tile is done, move to the next one
                continue;
            }

            // no picture, so use the flat colour for this tile type, lab 4
            const sf::Color col = get_color(t);

            // if the colour is see-through, draw nothing
            if (col.a == 0)
                continue;

            // add the 4 corners of the tile in that colour
            _vertices.append(sf::Vertex(p, col));
            _vertices.append(sf::Vertex({p.x + _tile_size, p.y}, col));
            _vertices.append(sf::Vertex({p.x + _tile_size, p.y + _tile_size}, col));
            _vertices.append(sf::Vertex({p.x, p.y + _tile_size}, col));
        }
    }
}

// draws the whole room in one draw call
void LevelSystem::render(sf::RenderTarget &target)
{
    sf::RenderStates states;

    // if there is an image and some pictures are set, draw using the image
    if (_texture != nullptr && !_rects.empty())
        states.texture = _texture;
    target.draw(_vertices, states);
}

// Asking about tiles

// gives back where the top left corner of a tile is on the screen
// tile number times tile size, plus the offset, lab 4
sf::Vector2f LevelSystem::get_tile_position(sf::Vector2i p)
{
    return _offset + sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y)) * _tile_size;
}

// gives back where the middle of a tile is on the screen
// the top left corner plus half a tile across and half a tile down
sf::Vector2f LevelSystem::get_tile_center(sf::Vector2i p)
{
    return get_tile_position(p) + sf::Vector2f(_tile_size * 0.5f, _tile_size * 0.5f);
}

// is this grid position inside the room?
// yes if x and y are not negative and are less than the width and height
bool LevelSystem::is_in_bounds(sf::Vector2i p)
{
    return p.x >= 0 && p.y >= 0 && p.x < _width && p.y < _height;
}

// gives back the tile at a grid position
LevelSystem::Tile LevelSystem::get_tile(sf::Vector2i p)
{
    // if the position is outside the room, stop with an error
    if (!is_in_bounds(p))
    {
        throw std::out_of_range("Tile out of range: (" + std::to_string(p.x) + "," +
                                std::to_string(p.y) + ")");
    }

    // the tiles are one long list, so tile (x, y) is at y times width plus x, lab 4
    return _tiles[static_cast<size_t>((p.y * _width) + p.x)];
}

// gives back which grid position a screen position is inside
sf::Vector2i LevelSystem::world_to_grid(sf::Vector2f v)
{
    // take off the offset and divide by the tile size
    const sf::Vector2f a = (v - _offset) / _tile_size;

    // round both numbers down to whole tiles
    return {static_cast<int>(std::floor(a.x)), static_cast<int>(std::floor(a.y))};
}

// gives back the tile at a screen position
LevelSystem::Tile LevelSystem::get_tile_at(sf::Vector2f v)
{
    // g = which grid position that is
    const sf::Vector2i g = world_to_grid(v);

    // if it is outside the room, treat it as a wall (added)
    if (!is_in_bounds(g))
    {
        return WALL;
    }
    return get_tile(g);
}

// can something walk on this tile?
// yes if it is inside the room and is not a wall
bool LevelSystem::is_walkable(sf::Vector2i p)
{
    return is_in_bounds(p) && get_tile(p) != WALL;
}

// Finding tiles

// gives back the grid position of every tile of one type
std::vector<sf::Vector2i> LevelSystem::find_tiles(Tile type)
{
    // out = the list we will give back, empty for now
    std::vector<sf::Vector2i> out;

    // for every tile in the room, if it is the type we want, add its position to the list
    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
        {
            if (get_tile({x, y}) == type)
                out.push_back({x, y});
        }
    }
    return out;
}

// is this grid position in this list?
bool LevelSystem::in_group(const sf::Vector2i &pos, const std::vector<sf::Vector2i> &group)
{
    return std::find(group.begin(), group.end(), pos) != group.end();
}

// joins touching tiles of one type into rectangles, lab 7
// chapter 6 makes one physics body per rectangle and not one per tile
// fewer bodies is faster, and nothing snags on the joins between tiles
// how it works:
//   1. go along each row and take the longest line of matching tiles
//   2. stretch that line downwards while the row below has the same line
std::vector<LevelSystem::TileRect> LevelSystem::get_groups(Tile type)
{
    // rects = the rectangles we will give back
    std::vector<TileRect> rects;

    // used = for every tile, has it already been put in a rectangle?
    std::vector<bool> used(static_cast<size_t>(_width * _height), false);

    // is_free = a small helper: is this tile the right type and not used yet?
    auto is_free = [&](int x, int y) {
        return get_tile({x, y}) == type && !used[static_cast<size_t>(y * _width + x)];
    };

    // for every tile in the room
    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
        {
            // if this tile is not free, skip it
            if (!is_free(x, y))
                continue;

            // run = how many free tiles in a line going right from here
            int run = 1;
            while (x + run < _width && is_free(x + run, y))
                ++run;

            // rows = how many rows down that same line carries on
            int rows = 1;
            bool grow = true;
            while (grow && y + rows < _height)
            {
                // check every tile of the line in the next row down
                for (int i = 0; i < run; ++i)
                {
                    // if one of them is not free, stop growing
                    if (!is_free(x + i, y + rows))
                    {
                        grow = false;
                        break;
                    }
                }

                // if they were all free, the rectangle gets one row taller
                if (grow)
                    ++rows;
            }

            // mark every tile in the rectangle as used
            for (int yy = y; yy < y + rows; ++yy)
                for (int xx = x; xx < x + run; ++xx)
                    used[static_cast<size_t>(yy * _width + xx)] = true;

            // add the rectangle to the list: where it starts and how big it is
            rects.push_back({{x, y}, {run, rows}});
        }
    }
    return rects;
}
