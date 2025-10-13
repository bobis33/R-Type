# 🎮 Guide Complet du Jeu R-Type

## ✅ Problèmes Corrigés

- ✅ **Menu fonctionnel** : La touche Entrée fonctionne maintenant
- ✅ **Tir normal** : Appuyer brièvement sur Espace tire un projectile normal
- ✅ **Tir superchargé** : Maintenir Espace pour charger, relâcher pour tirer
- ✅ **Architecture modulaire** : Deux plugins séparés (Local/Multi)

## 🕹️ Contrôles du Jeu

### Menu Principal
- **↑ / ↓** : Naviguer dans le menu
- **⏎ (Entrée)** : Sélectionner l'option (jaune/or)
- Options disponibles : **Solo**, Multi, Settings

### Jeu Solo (Mode Local)
- **↑ ↓ ← →** : Déplacer le vaisseau
- **Espace (court)** : Tir normal 🔫
- **Espace (maintenu)** : Charger le tir superchargé ⚡
- **Relâcher Espace** : Tirer le projectile chargé 💥

## 🎯 Système de Tir

### Tir Normal (Basic)
```
┌─────────────────────────────────┐
│ Appui rapide sur Espace        │
│         ↓                       │
│ Projectile normal (speed 800)  │
│ - Dégâts: 10                   │
│ - Cooldown: 0.3s               │
│ - Sprite: shoot.gif            │
└─────────────────────────────────┘
```

### Tir Superchargé
```
┌─────────────────────────────────────┐
│ 1. Maintenir Espace                │
│    → Animation de chargement 🔄    │
│    → Barre de charge se remplit    │
│                                     │
│ 2. Atteindre 50%+ de charge       │
│    → Animation complète ✨         │
│                                     │
│ 3. Relâcher Espace                │
│    → Projectile superchargé 💥    │
│    → Dégâts: 25                    │
│    → Speed: 1200                   │
│    → Animation: 4 frames           │
│    → Sprite: shootcharged.gif      │
└─────────────────────────────────────┘
```

### Charge Insuffisante
```
Si vous relâchez avant 50% de charge:
→ Tire un projectile normal à la place
```

## 📊 Statistiques Affichées

Pendant le jeu, vous voyez :
- **FPS** : Images par seconde (en haut à gauche)
- **Enemies** : Nombre d'ennemis à l'écran
- **Asteroids** : Nombre d'astéroïdes
- **Effet parallax** : Étoiles, comètes, nébuleuses

## 🔧 Architecture Technique

### Plugins de Jeu

```
┌──────────────────────────────────────┐
│         Plugin Local (Solo)          │
│  game_rtype_local.dylib              │
│                                      │
│  Scènes incluses:                    │
│  • Menu principal                    │
│  • GameSolo (jeu complet)           │
│                                      │
│  Systèmes inclus:                    │
│  • WeaponSystem                      │
│  • ProjectileManager                 │
│  • Gestion des événements           │
└──────────────────────────────────────┘

┌──────────────────────────────────────┐
│      Plugin Multi (Multiplayer)      │
│  game_rtype_multi.dylib              │
│                                      │
│  À implémenter:                      │
│  • Lobby multijoueur                │
│  • Synchronisation réseau           │
│  • Salles de jeu                    │
└──────────────────────────────────────┘
```

### Bibliothèque Commune

```
libgame_rtype_common.a
├── AGamePlugin (classe de base)
├── Scènes communes
│   ├── Menu
│   └── GameSolo
├── Systèmes
│   └── WeaponSystem
├── Managers
│   └── ProjectileManager
└── Configurations
    ├── Common.hpp
    └── GameConfig.hpp
```

## 🚀 Utilisation

### Lancer le Jeu

```bash
cd cmake-build-release/bin
./r-type_client
```

### Changer de Plugin

Dans `/client/src/client.cpp` ligne ~40 :

```cpp
// Plugin LOCAL (par défaut)
m_game = m_pluginLoader->loadPlugin<gme::IGameClient>(
    Path::Plugin::PLUGIN_GAME_RTYPE_LOCAL.string()
);

// Plugin MULTI
m_game = m_pluginLoader->loadPlugin<gme::IGameClient>(
    Path::Plugin::PLUGIN_GAME_RTYPE_MULTI.string()
);
```

## 🎨 Ressources Graphiques

### Projectiles
- `shoot.gif` : Tir normal
- `shootcharged.gif` : Tir superchargé (animé)
- `shootchargedloading.gif` : Animation de chargement

### Vaisseau
- `r-typesheet42.gif` : Sprite du joueur
- Taille: 33x17 pixels
- Scale: 2.0x

## 💡 Tips

1. **Optimisation du tir** :
   - Le tir normal a un cooldown de 0.3s
   - Le tir chargé a un cooldown de 0.2s
   - Plus rapide de spam tir normal que d'attendre

2. **Charge maximale** :
   - Atteint en ~1 seconde
   - Peut être relâché à partir de 50%
   - Se réinitialise après le tir

3. **Mouvement** :
   - Diagonales sont 70.7% de la vitesse normale
   - Le vaisseau reste dans les limites de l'écran
   - Vitesse: 500 pixels/seconde

## 🐛 Débogage

Si le tir ne fonctionne pas :

1. **Vérifiez que vous êtes dans GameSolo** (pas dans le menu)
2. **Vérifiez les logs** pour voir si les événements arrivent
3. **Testez les touches** : Espace doit être détecté

Logs utiles (si vous les réactivez) :
```
[INFO] Key pressed: 57  // Code de la touche Espace
[INFO] Weapon system update
[INFO] Fire basic projectile
```

## 📝 Prochaines Étapes

- [ ] Ajouter les ennemis
- [ ] Ajouter les collisions
- [ ] Ajouter les explosions
- [ ] Implémenter le plugin multijoueur
- [ ] Ajouter le menu Settings
- [ ] Ajouter le système de score
- [ ] Ajouter les sons de tir

