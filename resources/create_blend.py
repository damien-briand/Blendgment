"""
Script Blender pour créer un fichier .blend vierge.
Utilisation : blender -b --python-script create_blend.py -- output_path
"""

import bpy
import sys
import os

# Récupère les arguments passés après "--"
argv = sys.argv
try:
    index = argv.index("--") + 1
    output_path = argv[index] if index < len(argv) else None
except ValueError:
    output_path = None

if not output_path:
    print("ERROR: No output path specified")
    sys.exit(1)

# Assure que le répertoire parent existe
os.makedirs(os.path.dirname(output_path), exist_ok=True)

# Sauvegarde le fichier par défaut de Blender
try:
    bpy.ops.wm.save_as_mainfile(filepath=output_path)
    print(f"SUCCESS: Created {output_path}")
except Exception as e:
    print(f"ERROR: {str(e)}")
    sys.exit(1)
