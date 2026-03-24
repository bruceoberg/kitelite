# pre-build script to expose platformio.ini values (and other stuff) to C++ code

import json
import re
from pathlib import Path

env = DefaultEnvironment() # pyright: ignore [reportUndefinedVariable]

pathProject = Path(env.Dir('#').abspath)
strProject = pathProject.name.lower()

# verify FusionPlatformIO shim version matches Fusion's pyproject.toml

pathShim = pathProject / 'lib' / 'FusionPlatformIO' / 'library.json'
pathPyproject = pathProject / 'lib' / 'Fusion' / 'pyproject.toml'

with open(pathShim) as f:
	strShimVersion = json.load(f)['version']

strPyprojectText = pathPyproject.read_text()
matchVersion = re.search(r'^version\s*=\s*"([^"]+)"', strPyprojectText, re.MULTILINE)
if not matchVersion:
	raise RuntimeError(f"could not find version in {pathPyproject}")

strFusionVersion = matchVersion.group(1)

if strShimVersion != strFusionVersion:
	raise RuntimeError(
		f"FusionPlatformIO shim version ({strShimVersion}) does not match "
		f"Fusion pyproject.toml version ({strFusionVersion}) — update {pathShim}"
	)

env.Append(
	CPPDEFINES=[
		("PROJECT_NAME", env.StringifyMacro(strProject)),
		("UPLOAD_VIA_ESP_PROG", 1 if env.GetProjectOption("upload_protocol") == "esp-prog" else 0),
	]
)