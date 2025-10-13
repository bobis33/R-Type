# R-Type Game Plugins

## Architecture

Le système de jeu R-Type est maintenant divisé en **deux plugins distincts** :

### 1. **R-Type Local** (`game_rtype_local.dylib` / `.so` / `.dll`)
- Plugin pour le jeu en **mode solo/local**
- Gère les parties hors ligne
- Contient la logique du jeu solo

### 2. **R-Type Multi** (`game_rtype_multi.dylib` / `.so` / `.dll`)  
- Plugin pour le jeu en **mode multijoueur**
- Gère les connexions réseau et les parties en ligne
- Contient la logique du jeu multijoueur

### 3. **R-Type Common** (`libgame_rtype_common.a`)
- Bibliothèque statique contenant le **code commun** partagé entre les deux plugins
- Contient `AGamePlugin` (classe de base abstraite)
- Contient les scènes communes (Menu, etc.)
- Contient les constantes et utilitaires partagés

## Structure du Code

```
plugins/Game/
├── R-Type/                      # Bibliothèque commune
│   ├── include/
│   │   └── RTypeGame/
│   │       ├── AGamePlugin.hpp  # Classe de base abstraite
│   │       ├── Common.hpp       # Constantes et utilitaires
│   │       └── Scenes/          # Scènes communes
│   │           └── Menu.hpp
│   └── src/
│       ├── AGamePlugin.cpp
│       └── Scenes/
│           └── Menu.cpp
│
├── R-Type-Local/                # Plugin Solo
│   ├── include/
│   │   └── RTypeGameLocal/
│   │       └── RTypeGameLocal.hpp
│   └── src/
│       ├── RTypeGameLocal.cpp
│       └── entrypoint.cpp
│
└── R-Type-Multi/                # Plugin Multijoueur
    ├── include/
    │   └── RTypeGameMulti/
    │       └── RTypeGameMulti.hpp
    └── src/
        ├── RTypeGameMulti.cpp
        └── entrypoint.cpp
```

## Interface IGameClient

Les deux plugins implémentent l'interface `IGameClient` qui hérite de `IPlugin` :

```cpp
class IGameClient : public utl::IPlugin
{
    public:
        // Méthodes de l'interface
        virtual void initialize(renderer, audio) = 0;
        virtual void update(deltaTime, width, height) = 0;
        virtual void handleEvent(event) = 0;
        virtual IScene* getCurrentEngineScene() const = 0;
};
```

## Utilisation dans le Client

Le client charge le plugin approprié selon le mode :

```cpp
// Pour le mode local/solo
m_game = pluginLoader->loadPlugin<IGameClient>("game_rtype_local.dylib");

// Pour le mode multijoueur
m_game = pluginLoader->loadPlugin<IGameClient>("game_rtype_multi.dylib");
```

## Comment Ajouter une Nouvelle Scène

1. Créer la scène dans `R-Type/src/Scenes/` et son header dans `R-Type/include/RTypeGame/Scenes/`
2. L'ajouter à la bibliothèque commune dans `R-Type/CMakeLists.txt`
3. L'utiliser dans `initializeScenes()` du plugin approprié

## Avantages de cette Architecture

✅ **Séparation claire** entre le jeu solo et multijoueur
✅ **Code réutilisable** dans la bibliothèque commune  
✅ **Plugins indépendants** - chaque mode peut être développé séparément
✅ **Flexibilité** - facile d'ajouter de nouveaux modes de jeu
✅ **Modularité** - conforme à l'architecture plugin du projet

## Bugs Corrigés

- ✅ La touche **Entrée** fonctionne maintenant dans le menu
- ✅ Les événements sont correctement transmis au plugin de jeu
- ✅ Les callbacks du menu déclenchent des actions (logging pour l'instant)

