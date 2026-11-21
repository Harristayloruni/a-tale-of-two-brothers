// ecm.hpp entity component module
// lab 5 - PacMan 2

#pragma once
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

class Component;
class Scene;

class Entity 
{
    public:
    explicit Entity(Scene *scene = nullptr);
    virtual ~Entity();

    virtual void update(const float &dt);
    virtual void render();

    const sf::Vector2f &get_position() const;
    void set_position(const sf::Vector2f &position);
    float get_rotation() const;
    void set_rotation (float rotation);
    bool is_alive() const; // should be updated
    void set_alive(bool alive);
    bool is_visible() const; // should be rendered
    void set_visible(bool visible);
    bool is_for_deletion() const; // should be deleted
    void set_for_delete();

    Scene *get_scene() const; // which scene it owns
    void add_tag(const std::string &tag); // "player", "enemy", ... 
    bool has_tag(const std::string &tag) const;

    // Create component of type T, attatch it, return it
    // Extra arguments go to T's constructor after entity pointer
    // Lab 5 - adding a component
    template <typename T, typename... Targs>
    // shared_ptr is a pointer that deletes the object when the last owner lets go
    // passes extra arguments to the components constructor
    std::shared_ptr<T> add_component(Targs &&...params)
    {
        // compile error when adding something that isn't in the component
        static_assert(std::is_base_of<Component, T>::value, "T != component");
        std::shared_ptr<T> sp(std::make_shared<T>(this, std::forward<Targs>(params)...));
        _components.push_back(sp);
        return sp;
    }

    // Components of exactly type T
    // lab 5 - uses typeid
    template <typename T>
    std::vector<std::shared_ptr<T>> get_components() const
    {
        static_assert(std::is_base_of<Component, T>::Value, "T != component");
        std::vector<std::shared_ptr<T>> ret;
        for (const auto &c : _components)
        {
            if (typeid(*c) == typeid(T))
            {
                ret.push_back(std::static_pointer_cast<T>(c));
            }
        }
        return ret;
    }

    // Components of type T OR anything derived for T
    // lab 5 - uses dynamic_cast
    template <typename T>
    std::vector<std::shared_ptr<T>> get_compatible_components() const
    {
        static_assert(std::is_base_of<Component, T>::value, "T != component");
        std::vector<std::shared_ptr<T>> ret;
        for (const auto &c : _components)
        {
            if (auto dd = std::dynamic_pointer_cast<T>(c))
            {
                ret.push_back(dd);
            }
        }
        return ret;
    }

    // The first compatible component, or nullptr
    template <typename T>
    std::shared_ptr<T> get_component() const
    {
        for (const auto &c : _components)
        {
            if (auto dd = std::dynamic_pointer_cast<T>(c))
            {
                return dd;
            }
        }
        return nullptr;
    }

    protected:
        std::vector<std::shared_ptr<Component>> _components;
        sf::Vector2f _position;
        float _rotation;
        bool _alive;
        bool _visible;
        bool _for_deletion;
        Scene *_scene;
        std::vector<std::string> _tags;
};

class Component
{
    public:
        Component() = delete; // a component must always have an owner
        bool is_for_deletion() const;
        void set_for_delete();
        virtual void update(const float &dt) = 0; // =0: every comonent must write these
        virtual void render() = 0;
        virtual ~Component();
        Entity *get_parent() const;

    protected:
        Entity *const _parent; // the entity this component belongs to
        bool _for_deletion;
        explicit Component(Entity *const p);
};

// All the entities in one scene 
// lab 5 - entity management
struct EntityManager 
{
    std::vector<std::shared_ptr<Entity>> list;
    void update(const float &dt); // removes entites flagged for deletion
    void render();
    std::vector<std::shared_ptr<Entity>> find(const std::string &tag) const;
};