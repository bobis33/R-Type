# Utils Module

Ce module fournit des utilitaires essentiels pour le projet R-Type, notamment un système d'EventBus thread-safe pour la communication inter-composants.

## EventBus

L'EventBus est un système de messagerie décentralisé qui permet aux différents composants de l'application de communiquer de manière asynchrone et découplée.

### Caractéristiques

- **Thread-safe** : Peut être utilisé en toute sécurité dans un environnement multi-threadé
- **Pattern Singleton** : Une seule instance globale accessible partout
- **Gestion des priorités** : Support des événements avec différents niveaux de priorité
- **Filtrage d'événements** : Possibilité de filtrer certains types d'événements
- **Statistiques** : Suivi détaillé des performances et de l'utilisation
- **Sérialisation automatique** : Support de différents types de données
- **Expiration d'événements** : Les événements peuvent expirer automatiquement

### Types d'événements disponibles

L'EventBus supporte de nombreux types d'événements prédéfinis :

#### Événements Réseau (0x1000 - 0x1FFF)

- `WORLD_STATE_RECEIVED` - État du monde reçu
- `PLAYER_CONNECTED` - Joueur connecté
- `PLAYER_DISCONNECTED` - Joueur déconnecté
- `PLAYER_INPUT_RECEIVED` - Input joueur reçu

#### Événements Audio (0x3000 - 0x3FFF)

- `PLAY_SOUND` - Jouer un son
- `STOP_SOUND` - Arrêter un son
- `SET_VOLUME` - Définir le volume

#### Événements Rendu (0x4000 - 0x4FFF)

- `SPAWN_VISUAL_EFFECT` - Créer un effet visuel
- `UPDATE_CAMERA` - Mettre à jour la caméra
- `RENDER_TEXT` - Afficher du texte

#### Événements Système (0x5000 - 0x5FFF)

- `PAUSE_GAME` - Mettre en pause
- `QUIT_GAME` - Quitter le jeu
- `SAVE_GAME` - Sauvegarder

#### Événements Input (0x6000 - 0x6FFF)

- `KEY_PRESSED` - Touche pressée
- `MOUSE_CLICKED` - Clic souris

## Comment les composants communiquent via l'EventBus

### Pattern Singleton - Une seule instance pour tous

L'EventBus utilise le **pattern Singleton**, ce qui signifie qu'il n'existe qu'**une seule instance globale** partagée par tous les composants de l'application. C'est ce qui permet la communication entre GameClient, AsioClient, GameServer, etc.

```cpp
// Tous ces appels retournent la MÊME instance
class GameClient {
    utl::EventBus& m_eventBus = utl::EventBus::getInstance();  // Instance A
};

class AsioClient {
    utl::EventBus& m_eventBus = utl::EventBus::getInstance();  // Même instance A !
};

class GameServer {
    utl::EventBus& m_eventBus = utl::EventBus::getInstance();  // Toujours instance A !
};

// Vérification pratique
void testSingleton() {
    utl::EventBus& bus1 = utl::EventBus::getInstance();
    utl::EventBus& bus2 = utl::EventBus::getInstance();

    std::cout << "Adresse bus1: " << &bus1 << std::endl;  // 0x7fff123456
    std::cout << "Adresse bus2: " << &bus2 << std::endl;  // 0x7fff123456 (même !)

    // Test fonctionnel
    bus1.registerComponent(1000, "Test");
    auto stats = bus2.getStats();  // Voit immédiatement le composant !
}
```

### Flux de communication complet

```cpp
// 1. GameClient publie un événement
{
    utl::EventBus& gameEventBus = utl::EventBus::getInstance();
    gameEventBus.publish(utl::EventType::SEND_PLAYER_INPUT,
                        inputData,
                        1000,  // GameClient ID
                        2000); // AsioClient ID
}

// 2. AsioClient consomme automatiquement (même instance !)
{
    utl::EventBus& networkEventBus = utl::EventBus::getInstance();
    auto events = networkEventBus.consumeForTarget(2000);
    // Trouve l'événement publié par GameClient !

    // AsioClient répond
    networkEventBus.publish(utl::EventType::WORLD_STATE_RECEIVED,
                           responseData,
                           2000,  // AsioClient ID
                           1000); // GameClient ID
}

// 3. GameClient reçoit la réponse (toujours même instance !)
{
    utl::EventBus& gameEventBus = utl::EventBus::getInstance();
    auto responses = gameEventBus.consumeForTarget(1000);
    // Trouve la réponse d'AsioClient !
}
```

### Architecture visuelle

```
┌─────────────────────────────────────────────────────────────┐
│                    EVENTBUS SINGLETON                        │
│  ┌─────────────────┐    ┌─────────────────┐                │
│  │   Subscribers   │    │   Components    │                │
│  │ SEND_INPUT →    │    │ 1000: GameClient│                │
│  │  [2000]         │    │ 2000: AsioClient│                │
│  │ WORLD_STATE →   │    │ 3000: Renderer  │                │
│  │  [1000]         │    │                 │                │
│  └─────────────────┘    └─────────────────┘                │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ Event Queue: [GameInput] [WorldState] [AudioEvent] ... ││
│  └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
           ↑                    ↑                    ↑
    ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
    │ GameClient  │     │ AsioClient  │     │  Renderer   │
    │getInstance()│     │getInstance()│     │getInstance()│
    └─────────────┘     └─────────────┘     └─────────────┘
                               │
                     MÊME INSTANCE GLOBALE
```

### Utilisation de base

#### 1. Obtenir l'instance singleton

```cpp
#include "Utils/EventBus.hpp"

utl::EventBus& eventBus = utl::EventBus::getInstance();
```

#### 2. Enregistrer un composant

```cpp
// Enregistrer un composant avec un ID unique et un nom
uint32_t componentId = 1001;
eventBus.registerComponent(componentId, "GameEngine");
```

#### 3. S'abonner à des événements

```cpp
// S'abonner à des types d'événements spécifiques
eventBus.subscribe(componentId, utl::EventType::KEY_PRESSED);
eventBus.subscribe(componentId, utl::EventType::PLAYER_CONNECTED);
```

#### 4. Publier des événements

##### Événement simple

```cpp
// Créer et publier un événement basique
utl::Event event(utl::EventType::PAUSE_GAME, componentId);
eventBus.publish(event);
```

##### Événement avec données

```cpp
// Publier avec des données sérialisées
std::string playerName = "Player1";
eventBus.publish(utl::EventType::PLAYER_CONNECTED, playerName, componentId);

// Publier avec des données numériques
float volume = 0.8f;
eventBus.publish(utl::EventType::SET_VOLUME, volume, componentId);

// Événement ciblé vers un composant spécifique
uint32_t targetId = 2002;
eventBus.publish(utl::EventType::RENDER_TEXT, "Score: 1000", componentId, targetId);
```

##### Événement avec priorité

```cpp
// Événement critique avec haute priorité
eventBus.publish(utl::EventType::SERVER_DISCONNECTED,
                 std::vector<uint8_t>{},
                 componentId,
                 0, // broadcast
                 utl::EventPriority::CRITICAL);
```

#### 5. Consommer des événements

##### Consommation non-bloquante

```cpp
// Récupérer jusqu'à 50 événements
std::vector<utl::Event> events = eventBus.consume(50);

for (const auto& event : events) {
    switch (event.type) {
        case utl::EventType::KEY_PRESSED:
            handleKeyPress(event);
            break;
        case utl::EventType::PLAYER_CONNECTED:
            handlePlayerConnection(event);
            break;
        default:
            break;
    }
}
```

##### Consommation avec timeout

```cpp
// Attendre des événements avec timeout de 100ms
auto events = eventBus.waitForEvents(std::chrono::milliseconds(100));
```

##### Consommation par type

```cpp
// Récupérer seulement les événements d'un type spécifique
auto inputEvents = eventBus.consumeType(utl::EventType::KEY_PRESSED);
```

##### Consommation par cible

```cpp
// Récupérer les événements destinés à ce composant ou broadcast
auto myEvents = eventBus.consumeForTarget(componentId);
```

### Désérialisation des données

```cpp
void handlePlayerConnection(const utl::Event& event) {
    if (!event.data.empty()) {
        rnp::Serializer deserializer(event.data);
        std::string playerName = deserializer.readString();

        std::cout << "Player connected: " << playerName << std::endl;
    }
}

void handleVolumeChange(const utl::Event& event) {
    if (event.data.size() >= sizeof(float)) {
        rnp::Serializer deserializer(event.data);
        float volume = deserializer.readFloat();

        setAudioVolume(volume);
    }
}
```

### Fonctionnalités avancées

#### Filtrage d'événements

```cpp
// Filtrer un type d'événement (sera ignoré)
eventBus.filterEventType(utl::EventType::RENDER_TEXT);

// Supprimer le filtre
eventBus.unfilterEventType(utl::EventType::RENDER_TEXT);
```

#### Configuration

```cpp
// Définir la taille maximale de la queue
eventBus.setMaxQueueSize(5000);

// Vérifier si l'EventBus fonctionne
if (eventBus.isRunning()) {
    // Publier des événements...
}
```

#### Statistiques et monitoring

```cpp
// Obtenir les statistiques
utl::EventStats stats = eventBus.getStats();

std::cout << "Événements publiés: " << stats.totalEventsPublished << std::endl;
std::cout << "Événements consommés: " << stats.totalEventsConsumed << std::endl;
std::cout << "Taille actuelle de la queue: " << stats.currentQueueSize << std::endl;
std::cout << "Événements par seconde: " << stats.getEventsPerSecond() << std::endl;

// Réinitialiser les statistiques
eventBus.clearStats();
```

#### Nettoyage

```cpp
// Se désabonner d'un type d'événement
eventBus.unsubscribe(componentId, utl::EventType::KEY_PRESSED);

// Désenregistrer un composant (se désabonne automatiquement de tout)
eventBus.unregisterComponent(componentId);

// Vider la queue d'événements
eventBus.clear();

// Arrêter l'EventBus
eventBus.stop();
```

### Exemple complet : Système de jeu

```cpp
#include "Utils/EventBus.hpp"
#include "Utils/Event.hpp"

class GameEngine {
private:
    uint32_t m_componentId = 1000;
    utl::EventBus& m_eventBus;

public:
    GameEngine() : m_eventBus(utl::EventBus::getInstance()) {
        // Enregistrement du composant
        m_eventBus.registerComponent(m_componentId, "GameEngine");

        // Abonnements aux événements
        m_eventBus.subscribe(m_componentId, utl::EventType::KEY_PRESSED);
        m_eventBus.subscribe(m_componentId, utl::EventType::PLAYER_CONNECTED);
        m_eventBus.subscribe(m_componentId, utl::EventType::QUIT_GAME);
    }

    void update() {
        // Consommer les événements
        auto events = m_eventBus.consume(100);

        for (const auto& event : events) {
            handleEvent(event);
        }
    }

    void handleEvent(const utl::Event& event) {
        switch (event.type) {
            case utl::EventType::KEY_PRESSED:
                processInput(event);
                break;

            case utl::EventType::PLAYER_CONNECTED:
                onPlayerJoined(event);
                break;

            case utl::EventType::QUIT_GAME:
                shutdown();
                break;
        }
    }

    void processInput(const utl::Event& event) {
        // Traiter l'input et potentiellement publier de nouveaux événements
        m_eventBus.publish(utl::EventType::PLAY_SOUND,
                          std::string("button_click.wav"),
                          m_componentId);
    }

    void onPlayerJoined(const utl::Event& event) {
        // Informer le système de rendu
        m_eventBus.publish(utl::EventType::RENDER_TEXT,
                          std::string("New player joined!"),
                          m_componentId,
                          2000); // ID du renderer
    }

    ~GameEngine() {
        m_eventBus.unregisterComponent(m_componentId);
    }
};
```

### Bonnes pratiques

1. **IDs uniques** : Utilisez des IDs uniques pour chaque composant
2. **Gestion d'erreurs** : Vérifiez toujours le retour de `publish()`
3. **Performance** : Limitez le nombre d'événements traités par frame
4. **Nettoyage** : Désenregistrez toujours vos composants à la destruction
5. **Thread-safety** : L'EventBus est thread-safe, pas besoin de mutex supplémentaires
6. **Données** : Gardez les données d'événements petites pour de meilleures performances

### Limitations

- La queue a une taille maximale (par défaut 10000 événements)
- Les événements peuvent expirer (par défaut après 5 secondes)
- La sérialisation n'est supportée que pour certains types de données
- Un seul EventBus global (pattern singleton)
