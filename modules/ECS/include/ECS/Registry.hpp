///
/// @file Registry.hpp
/// @brief This file contains the Registry class declaration
/// @namespace ecs
///

#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "ECS/Entity.hpp"

namespace ecs
{
    ///
    /// @class Registry
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class Registry
    {
        public:
            Registry() = default;
            ~Registry() = default;

            Registry(const Registry &) = delete;
            Registry &operator=(const Registry &) = delete;
            Registry(Registry &&) = delete;
            Registry &operator=(Registry &&) = delete;

            class EntityBuilder
            {
                public:
                    EntityBuilder(Registry &reg, Entity e) : m_registry(reg), m_entity(e) {}

                    template <typename T, typename... Args> EntityBuilder &with(Args &&...args)
                    {
                        m_registry.addComponent<T>(m_entity, std::forward<Args>(args)...);
                        return *this;
                    }

                    Entity build() const { return m_entity; }

                private:
                    Registry &m_registry;
                    Entity m_entity;
            };

            EntityBuilder createEntity()
            {
                const Entity entity = ++m_lastEntity;
                m_entities.push_back(entity);
                return EntityBuilder(*this, entity);
            }

            void destroyEntity(Entity e)
            {
                m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
                for (auto &[_, pool] : m_components)
                    pool->remove(e);
            }

            template <typename T, typename... Args> T &addComponent(Entity e, Args &&...args)
            {
                auto &pool = getPool<T>();
                T &comp = pool.add(e, std::forward<Args>(args)...);
                for (auto &cb : m_onComponentAddedCallbacks)
                {
                    cb(e, typeid(T));
                }
                return comp;
            }

            template <typename T> T *getComponent(Entity e)
            {
                auto &pool = getPool<T>();
                return pool.get(e);
            }

            template <typename T> std::unordered_map<Entity, T> &getAll() { return getPool<T>().data; }

            template <typename T> bool hasComponent(Entity e)
            {
                auto &pool = getPool<T>();
                return pool.has(e);
            }

            template <typename T> void removeComponent(Entity e)
            {
                auto &pool = getPool<T>();
                pool.remove(e);
            }

            void onComponentAdded(std::function<void(Entity, const std::type_info &)> cb)
            {
                m_onComponentAddedCallbacks.push_back(std::move(cb));
            }

        private:
            class IPool
            {
                public:
                    virtual ~IPool() = default;
                    virtual void remove(Entity e) = 0;
            };

            template <typename T> class Pool final : public IPool
            {
                public:
                    std::unordered_map<Entity, T> data;

                    template <typename... Args> T &add(Entity e, Args &&...args)
                    {
                        return data.emplace(e, T{std::forward<Args>(args)...}).first->second;
                    }

                    T *get(Entity e)
                    {
                        auto it = data.find(e);
                        if (it != data.end())
                        {
                            return &it->second;
                        }
                        return nullptr;
                    }

                    bool has(Entity e) { return data.contains(e); }

                    void remove(Entity e) override { data.erase(e); }
            };

            template <typename T> Pool<T> &getPool()
            {
                const std::type_index ti(typeid(T));
                if (!m_components.contains(ti))
                {
                    m_components[ti] = std::make_unique<Pool<T>>();
                }
                return *static_cast<Pool<T> *>(m_components[ti].get());
            }
            Entity m_lastEntity = INVALID_ENTITY;
            std::vector<Entity> m_entities;
            std::unordered_map<std::type_index, std::unique_ptr<IPool>> m_components;
            std::vector<std::function<void(Entity, const std::type_info &)>> m_onComponentAddedCallbacks;

    }; // class Registry

} // namespace ecs