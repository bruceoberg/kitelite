# pre-build script to expose platformio.ini values (and other stuff) to C++ code

from pathlib import Path

env = DefaultEnvironment() # pyright: ignore [reportUndefinedVariable]

strProject = Path(env.Dir('#').abspath).name.lower()

env.Append(
	CPPDEFINES=[
		("PROJECT_NAME", env.StringifyMacro(strProject)),
		("UPLOAD_VIA_ESP_PROG", 1 if env.GetProjectOption("upload_protocol") == "esp-prog" else 0),
	]
)