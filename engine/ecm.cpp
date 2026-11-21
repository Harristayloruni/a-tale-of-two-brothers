// ecm.cpp - see ecm.hpp
#include "ecm.hpp"
#include <algorithm>

// --------Entity---------

Entity::Entity(Scene *scene)
    : _position(0.f, 0.f), _rotation(0.f), _alive(true), 
    _visible(true), _for_deletion(false), _scene(scene) {}

Entity::~Entity() = default;

void Entity::update(const float &dt)
{
    if (!_alive)
    {
        return; // dead/paused enemies don't update
    }
    // Remove components flagged for deletion, then update the rest
    _components.erase(std::remove_if(_components.begin(), _components.end(),
                                     [](const std::shared_ptr<Component> &c)
                                     {
                                         return c->is_for_deletion();
                                     }),
                      _components.end());
    // Index loop: component may add another component while updating
    for (size_t i = 0; i < _components.size(); ++i)
    {
        _components[i]->update(dt);
    }
}

void Entity::render()
{
    if (!_visible)
    {
        return;
    }
    for (auto &c : _components)
    {
        c->render();
    }
}

const sf::Vector2f &Entity::get_position() const 
{
    return _position;
}
void Entity::set_position(const sf::Vector2f &position)
{
    _position = position;
}
float Entity::get_rotation() const 
{
    return _rotation;
}
void Entity::set_rotation(float rotation)
{
    _rotation = rotation;
}
bool Entity::is_alive() const 
{
    return _alive;
}
void Entity::set_alive(bool alive)
{
    _alive = alive;
}
bool Entity::is_visible() const 
{
    return _visible;
}
void Entity::set_visible(bool visible)
{
    _visible = visible;
}
bool Entity::is_for_deletion() const
{
    return _for_deletion;
}

void Entity::set_for_delete()
{
    _for_deletion = true;
    _alive = false;
    _visible = false;
}

Scene *Entity::get_scene() const
{
    return _scene;
}

void Entity::add_tag(const std::string &tag)
{
    if (!has_tag(tag))
    {
        _tags.push_back(tag);
    }
}

bool Entity::has_tag(const std::string &tag) const
{
    return std::find(_tags.begin(), _tags.end(), tag) !=_tags.end();
}

// ---------Component------------

Component::Component(Entity *const p) : _parent(p), _for_deletion(false) {}
Component::~Component() = default;
bool Component::is_for_deletion() const
{
    return _for_deletion;
}
void Component::set_for_delete() 
{
    _for_deletion = true;
}
Entity *Component::get_parent() const 
{
    return _parent;
}

// ---------Entity Manager-----------
void EntityManager::update(const float &dt)
{
    // Delete flagged enteties first (their components are destroyed with them)
    //pac-man 3
    list.erase(std::remove_if(list.begin(), list.end(),
                              [](const std::shared_ptr<Entity> &e)
                              {
                                  return e->is_for_deletion();
                              }),
               list.end());
    // Index loop: entities can be created while updating (e.g spawning)
    for (size_t i=0; i < list.size(); ++i)
    {
        list[i]->update(dt);
    }                            
}

void EntityManager::render()
{
    for (auto &e : list)
    {
        e->render();
    }
}

std::vector<std::shared_ptr<Entity>> EntityManager::find(const std::string &tag) const
{
    std::vector<std::shared_ptr<Entity>> ret;
    for (const auto &e :list) 
    {
        if (!e->is_for_deletion() && e->has_tag(tag))
        {
            ret.push_back(e);
        }
    }
    return ret;
}

