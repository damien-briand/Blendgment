# Blendgment — Developer README (src/)

Ce document décrit la structure interne de `src/` pour aider à comprendre, maintenir et faire évoluer le projet.

## 1. Vue d'ensemble

Blendgment est une application desktop C++17 basée sur :

- **Vulkan** (rendu)
- **GLFW** (fenêtre + événements)
- **ImGui** (interface)
- **libcurl** (HTTP)
- **libarchive** (extraction d'archives)

Le projet suit une architecture modulaire :

- boucle d'application (`Application`)
- orchestration Vulkan (`VulkanContext`)
- UI découpée par pages/modales (`UI/`)
- services réseau (`services/`)
- utilitaires téléchargement/extraction (`utils/`)

---

## 2. Structure du dossier `src/`

```text
src/
├── main.cpp
├── Application.h/.cpp
├── VulkanContext.h/.cpp
├── core/
│   ├── Window.h/.cpp
│   ├── Instance.h/.cpp
│   └── Device.h/.cpp
├── rendering/
│   ├── SwapChain.h/.cpp
│   └── Renderer.h/.cpp
├── UI/
│   ├── UIManager.h/.cpp
│   ├── Theme.h
│   ├── InstalledVersion.h/.cpp
│   ├── pages/
│   │   ├── DashboardPage.h/.cpp
│   │   ├── VersionsPage.h/.cpp
│   │   ├── ProjectsPage.h/.cpp
│   │   └── SettingsPage.h/.cpp
│   └── modals/
│       ├── InstallModal.h/.cpp
│       ├── DeleteModal.h/.cpp
│       └── NewProjectModal.h/.cpp
├── services/
│   └── BlenderFetcher.h/.cpp
└── utils/
		├── SynchronousDownloader.h/.cpp
		└── Extractor.h/.cpp
```

> Note: `utils/external/imgui/` peut exister en local, mais le build principal utilise ImGui via `FetchContent` dans le `CMakeLists.txt` racine.

---

## 3. Pipeline d'exécution

1. `main.cpp`
	 - initialise `curl_global_init`
	 - crée `Application`
	 - appelle `run()`

2. `Application`
	 - crée `Window`, `VulkanContext`, `UIManager`
	 - initialise les backends ImGui (GLFW + Vulkan)
	 - boucle principale :
		 - poll events
		 - gestion resize swapchain
		 - `ImGui::NewFrame()`
		 - `UIManager::render()`
		 - submit/present Vulkan

3. `UIManager`
	 - dessine le layout global (topbar, sidebar, content)
	 - route vers la page active
	 - rend les modales (installation, suppression, nouveau projet)

---

## 4. Détails des modules

## 4.1 `core/`

- **Window**: encapsule GLFWwindow + resize flags.
- **Instance**: crée/détruit l'instance Vulkan.
- **Device**: sélection GPU, queues, device logique.

## 4.2 `rendering/`

- **SwapChain**: images, render pass, framebuffers, recréation au resize.
- **Renderer**: command pool/buffers, sync objects, soumission frame.

## 4.3 `VulkanContext`

Façade légère qui compose :

`Instance -> Device -> SwapChain -> Renderer`

Expose l'API utilisée par `Application` et ImGui (queues, render pass, descriptor pool, image count, begin/end frame, etc.).

## 4.4 `services/BlenderFetcher`

Responsabilités:

- fetch asynchrone des versions Blender (`major.minor`)
- fetch asynchrone des releases patch (`major.minor.patch`)
- parsing HTML de download.blender.org
- cache thread-safe des résultats

## 4.5 `utils/`

- **SynchronousDownloader**:
	- détection plateforme (`PlatOS`, `PlatArch`)
	- construction URL/fichier par OS
	- téléchargement bloquant + callback de progression
	- annulation via `atomic<bool>`

- **Extractor**:
	- extraction bloquante via libarchive
	- callback de progression
	- annulation via `atomic<bool>`

Ces 2 classes sont exécutées dans des threads dédiés depuis `InstallModal` pour ne pas bloquer l'UI.

---

## 5. UI Architecture (`UI/`)

## 5.1 `UIManager`

Contient:

- état global UI (`NavPage`, chemins install/projets, cache versions installées)
- instances de pages (`DashboardPage`, `VersionsPage`, `ProjectsPage`, `SettingsPage`)
- instances de modales (`InstallModal`, `DeleteModal`, `NewProjectModal`)

## 5.2 Pages

- **DashboardPage**: stats + versions installées (lancer/supprimer).
- **VersionsPage**: listing online + action installer.
- **ProjectsPage**: listing projets + ouvrir/supprimer.
- **SettingsPage**: édition des chemins de travail.

## 5.3 Modales

- **InstallModal**: sélection patch/format + download + extract + progression.
- **DeleteModal**: confirmation suppression version Blender.
- **NewProjectModal**:
	- création dossier projet
	- création `textures/`
	- copie `../BasicBlendFile.blend` vers `<Projet>/<Projet>.blend`

## 5.4 Thème

`Theme.h` centralise la palette (`namespace Col`).

---

## 6. Gestion des chemins et données locales

Par défaut:

- `m_installPath = ../blender`
- `m_projectsPath = ../projects`

Implications:

- si l'app est lancée depuis `build/`, ces chemins pointent vers la racine repo.
- `NewProjectModal` s'appuie aussi sur `../BasicBlendFile.blend`.

Si le mode de lancement change, valider les chemins relatifs.

---

## 7. Bonnes pratiques pour contribuer

- Garder `UIManager` mince: layout + routing uniquement.
- Toute nouvelle logique UI métier doit aller dans une **page** ou une **modale** dédiée.
- Éviter les blocs monolithiques: privilégier petites fonctions privées.
- Pour l'asynchrone:
	- protéger l'état partagé avec `std::mutex`
	- ne jamais bloquer le thread UI
- Préserver le style existant (naming, commentaires FR, organisation).

---

## 8. Ajouter une nouvelle page

1. Créer `UI/pages/NewPage.h/.cpp`
2. Ajouter l'instance dans `UIManager.h`
3. Ajouter une entrée sidebar (`renderSidebar`)
4. Router dans `renderMainContent`
5. Brancher la logique métier via service/utilitaire si nécessaire

---

## 9. Ajouter une nouvelle modale

1. Créer `UI/modals/NewModal.h/.cpp`
2. Ajouter l'instance dans `UIManager.h`
3. Appeler `render()` après `ImGui::End()` de la fenêtre racine
4. Déclencher l'ouverture depuis la page concernée

---

## 10. Débogage rapide

- **UI freeze**: vérifier qu'aucun I/O bloquant n'est exécuté sur le thread UI.
- **Resize cassé**: vérifier la séquence `handleResize()` + recréation swapchain.
- **Install incomplète**: vérifier callbacks `DownloadProgress` / `ExtractProgress`.
- **Projet sans `.blend`**: vérifier présence de `BasicBlendFile.blend` à la racine attendue.

---

## 11. Dette technique connue

- Parsing HTML Blender fragile aux changements du site (idéalement passer à une source API stable si disponible).
- Persistance des paramètres encore basique (champ "Sauvegarder" à compléter selon stratégie choisie).

---

Ce README est la référence développeur pour le dossier `src/`.
