# Blendgment — User Guide

Blendgment is an application for managing Blender versions and projects.

Main goals:
- easily install Blender versions,
- launch/delete installed versions,
- create and organize projects.

---

## 1) Getting started

When the app starts, you will see 4 pages in the sidebar:
- **Dashboard**
- **Versions**
- **Projets**
- **Parametres**

If this is your first launch, start by checking the paths in **Parametres**.

---

## 2) **Parametres** page

This page lets you configure:
- **Repertoire d'installation Blender**: where Blender versions are extracted.
- **Repertoire des projets**: where project folders are created.

Default values:
- Blender: `../blender`
- Projects: `../projects`

---

## 3) **Versions** page (install Blender)

On this page:
- Click **Actualiser** to load available versions.
- You can filter with **Masquer versions < 2.80**.
- Click **Installer** on a version.

In the install window:
- select the patch version,
- check the archive format,
- check the destination folder,
- click **Telecharger**.

During installation:
- download progress is shown,
- extraction is automatic,
- cancellation is available.

When installation is complete, the version appears in the Dashboard.

---

## 4) **Dashboard** page

The Dashboard shows:
- number of installed versions,
- latest available version,
- installed versions list.

Available actions for each installed version:
- **Lancer**: opens Blender.
- **Suppr.**: removes the version (with confirmation).

---

## 5) **Projets** page

### Create a project

Click **+ Nouveau projet**, enter a name, then click **Creer**.

When a project is created, Blendgment:
1. creates the project folder,
2. automatically creates a **textures** subfolder,
3. copies **BasicBlendFile.blend** into the project folder,
4. renames it to **ProjectName.blend**.

Example for a project named `MyProject`:
- `MyProject/`
- `MyProject/textures/`
- `MyProject/MyProject.blend`

> If `BasicBlendFile.blend` is missing, the project is still created, but without the initial `.blend` file.

### Manage projects

For each project in the list:
- **Ouvrir**: opens the project folder in your file explorer.
- **Suppr.**: deletes the project folder (with confirmation).

---

## 6) Usage tips

- Keep project names simple (avoid special characters).
- Verify paths in **Parametres** before installing Blender or creating projects.
- Deletion is **permanent**.

---

## 7) Quick troubleshooting

- **Version list does not load**: check your internet connection, then click **Actualiser**.
- **Blender does not launch**: verify that installation completed and the executable exists.
- **Base .blend file is not copied**: check that `BasicBlendFile.blend` is present.

---

# How to Contribute

You can contribute to Blendgment by coding new features, fixing bugs, or improving documentation.

For anderstand the structure of the codebase, go to `/src` and check the README.md inside.

