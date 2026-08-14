# GDENG03 Finals

Prepared by: Arvin Lawrence B. Dacanay

Based on PardCode's C++ 3D Game Engine tutorial series: https://github.com/PardCode/CPP-3D-Game-Tutorial-Series

## How to run the program

1. Open `DirectXGameEngine/DirectXGameEngine.sln` in Visual Studio.
2. Set the configuration to `Debug` and platform to `x64`.
3. Build the solution.
4. Run the program using `Ctrl + F5` or the Visual Studio run/debug button.

The entry point is:

```text
DirectXGameEngine/Game/main.cpp
```

The application window is named `Dacanay Finals`.

## Scene saving and loading

Scenes are saved as `.level` JSON text files.

Scene files are stored in:

```text
DirectXGameEngine/Assets/Scenes
```

To save a scene:

1. Open the editor.
2. Use `File > Save Level` to save to the currently loaded scene file.
3. Use `File > Save Level As New` to create a new scene file.

To load a scene:

1. Open the editor.
2. Use `File > Load Level`.
3. Select one of the `.level` files listed in the menu.

## Unity import/export

Unity support is handled by the editor script:

```text
DirectXGameEngine/Tools/Unity/LevelImporterExporter.cs
```

Unity setup:

1. Open your Unity project.
2. Create an `Editor` folder inside the Unity project's `Assets` folder if it does not already exist.
3. Copy this file from the engine repository:

```text
DirectXGameEngine/Tools/Unity/LevelImporterExporter.cs
```

4. Paste it into the Unity project here:

```text
Assets/Editor/LevelImporterExporter.cs
```

5. Let Unity recompile scripts.
6. Place `.level` files from this editor anywhere inside the Unity project's `Assets` folder, for example:

```text
Assets/Levels
```

After the script is installed, use the Unity menu items added by the script to import or export `.level` files. If the menu item does not appear, check that the script is inside `Assets/Editor`, not just `Assets`.

The Unity script supports:

- primitive objects
- transforms
- enabled/disabled state
- rigid body data
- scene export back to the same `.level` JSON schema

## Unreal import/export

Unreal support is handled by the Python script:

```text
DirectXGameEngine/Tools/Unreal/level_importer_exporter.py
```

Unreal setup:

1. Open your Unreal project.
2. Enable Python support if it is not already enabled:
   - Go to `Edit > Plugins`.
   - Enable `Python Editor Script Plugin`.
   - Restart Unreal if prompted.
3. Create a `Python` folder inside the Unreal project's `Content` folder if it does not already exist.
4. Copy this file from the engine repository:

```text
DirectXGameEngine/Tools/Unreal/level_importer_exporter.py
```

5. Paste it into the Unreal project here:

```text
Content/Python/level_importer_exporter.py
```

6. Open Unreal's Python console.
7. Load or reload the script:

```python
import importlib
import level_importer_exporter
importlib.reload(level_importer_exporter)
```

If `import level_importer_exporter` fails, verify that the file is exactly in `Content/Python/level_importer_exporter.py`, then restart Unreal and try again.

To import a `.level` file from this editor into Unreal, use this template:

```python
level_importer_exporter.import_level(r"<path-to-your-level-file>")
```

Example:

```python
level_importer_exporter.import_level(r"C:\School\GDENG03\Engine Repli\GDENG03_Engine_Feature_Replication\DirectXGameEngine\Assets\Scenes\scene_1.level")
```

To export the current Unreal scene back to a `.level` file, use this template:

```python
level_importer_exporter.export_level(r"<path-where-you-want-to-save-the-level-file>")
```

Example:

```python
level_importer_exporter.export_level(r"C:\School\GDENG03\Engine Repli\GDENG03_Engine_Feature_Replication\DirectXGameEngine\Assets\Scenes\unreal_export.level")
```

The Unreal script supports:

- primitive/static mesh actors
- transforms with editor-to-Unreal axis conversion
- enabled/visibility state
- rigid body/physics simulation state
- scene export back to the same `.level` JSON schema


Note: I had Codex assist with Readme Formatting
