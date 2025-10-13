# Guide d'Utilisation du Jeu R-Type

## 🎮 Comment Jouer

### Menu Principal

1. **Lancer le jeu** : `./r-type_client`

2. **Navigation dans le menu** :
   - **↑ (Flèche Haut)** : Monter dans le menu
   - **↓ (Flèche Bas)** : Descendre dans le menu
   - **⏎ (Entrée)** : Sélectionner l'option actuelle

3. **Indicateur visuel** :
   - L'option **sélectionnée** apparaît en **JAUNE/OR** 🟡
   - Les autres options sont en **BLANC** ⚪

### Options du Menu

- **Solo** : Lance le jeu en mode solo/local
- **Multi** : Lance le jeu en mode multijoueur (à implémenter)
- **Settings** : Paramètres (à implémenter)

### Mode Solo - Contrôles du Jeu

Une fois dans le jeu solo :

- **↑ ↓ ← →** : Déplacer le vaisseau
- **Espace** : Tirer

Le jeu affiche :
- FPS en temps réel
- Nombre d'ennemis
- Nombre d'astéroïdes  
- Effet parallax avec étoiles et comètes

## 🔧 Architecture des Plugins

Le jeu utilise maintenant **2 plugins séparés** :

### Plugin Local (`game_rtype_local.dylib`)
- **Usage** : Jeu en solo/local
- **Scènes incluses** :
  - Menu principal
  - Jeu solo avec vaisseau jouable
  - Effet parallax et ennemis

### Plugin Multi (`game_rtype_multi.dylib`)
- **Usage** : Jeu multijoueur
- **À implémenter** : Lobby, salles de jeu, synchronisation réseau

### Comment changer de plugin

Editez `/client/src/client.cpp` ligne ~40 :

```cpp
// Pour le mode LOCAL (par défaut)
m_game = m_pluginLoader->loadPlugin<gme::IGameClient>(Path::Plugin::PLUGIN_GAME_RTYPE_LOCAL.string());

// Pour le mode MULTI
m_game = m_pluginLoader->loadPlugin<gme::IGameClient>(Path::Plugin::PLUGIN_GAME_RTYPE_MULTI.string());
```

## 🐛 Résolution de Problèmes

### "Je ne peux pas sélectionner dans le menu"

✅ **Solution** : La touche Entrée fonctionne ! Vérifiez les logs dans le terminal. Vous devriez voir :
```
[INFO] Enter key detected! Selected index: X
[INFO] Selected option: Solo
[INFO] Switching to solo game scene...
```

Si vous voyez ces messages, cela signifie que le jeu **change bien de scène** et vous devriez voir le jeu solo se lancer.

### "Le jeu reste bloqué après sélection"

Vérifiez les logs pour confirmer que :
1. L'événement Entrée est reçu ✅
2. Le callback est appelé ✅  
3. Le changement de scène se fait ✅

### "Je ne vois pas le vaisseau"

Le vaisseau apparaît à la position (200, 100). Assurez-vous que la fenêtre est assez grande pour le voir.

## 📝 Logs de Débogage

Les logs affichent :
- `[INFO] Client received event` - Le client reçoit l'événement
- `[INFO] Forwarding event to game plugin` - Transmission au plugin
- `[INFO] AGamePlugin received event` - Le plugin reçoit l'événement
- `[INFO] Menu received event` - Le menu reçoit l'événement
- `[INFO] Key pressed: X` - La touche X est pressée
- `[INFO] Enter key detected!` - La touche Entrée est détectée
- `[INFO] Switching to solo game scene...` - Changement vers la scène de jeu

Pour désactiver les logs de débogage, supprimez les lignes `utl::Logger::log()` dans :
- `client/src/event.cpp`
- `plugins/Game/R-Type/src/AGamePlugin.cpp`
- `plugins/Game/R-Type/src/Scenes/Menu.cpp`

## 🚀 Prochaines Étapes

- [ ] Implémenter le mode multijoueur complet
- [ ] Ajouter la scène Settings
- [ ] Ajouter le système de tir (WeaponSystem)
- [ ] Ajouter les ennemis et collisions
- [ ] Ajouter le système de score

