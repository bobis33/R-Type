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

            Entity createEntity()
            {
                const Entity entity = ++m_lastEntity;
                m_entities.push_back(entity);
                for (auto &cb : m_onEntityCreatedCallbacks)
                    cb(entity);
                return entity;
            }

            template <typename T, typename... Args> T &addComponent(Entity e, Args &&...args)
            {
                auto &pool = getPool<T>();
                T &comp = pool.add(e, std::forward<Args>(args)...);
                for (auto &cb : m_onComponentAddedCallbacks)
                    cb(e, typeid(T));
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
            void onEntityCreated(std::function<void(Entity)> cb)
            {
                m_onEntityCreatedCallbacks.push_back(std::move(cb));
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
                            return &it->second;
                        return nullptr;
                    }

                    bool has(Entity e) { return data.find(e) != data.end(); }

                    void remove(Entity e) override { data.erase(e); }
            };

            template <typename T> Pool<T> &getPool()
            {
                std::type_index ti(typeid(T));
                if (!m_components.contains(ti))
                {
                    m_components[ti] = std::make_unique<Pool<T>>();
                }
                return *static_cast<Pool<T> *>(m_components[ti].get());
            }
            Entity m_lastEntity = INVALID_ENTITY;
            std::vector<Entity> m_entities;
            std::unordered_map<std::type_index, std::unique_ptr<IPool>> m_components;
            std::vector<std::function<void(Entity)>> m_onEntityCreatedCallbacks;
            std::vector<std::function<void(Entity, const std::type_info &)>> m_onComponentAddedCallbacks;

    }; // class Registry

} // namespace ecs