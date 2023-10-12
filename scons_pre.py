# extra script to expose platformio.ini values (and other stuff) to C++ code

from pathlib import Path

env = DefaultEnvironment() # pyright: ignore [reportUndefinedVariable]

strProject = Path(env.Dir('#').abspath).name

env.Append(
	CPPDEFINES=[
		("PROJECT_NAME", env.StringifyMacro(strProject)),
		("MONITOR_SPEED", env.GetProjectOption("monitor_speed")),
		("UPLOAD_VIA_ESP_PROG", 1 if env.GetProjectOption("upload_protocol") == "esp-prog" else 0),
	]
)