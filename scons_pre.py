# extra script to expose platformio.ini values to C++ code

env = DefaultEnvironment()

env.Append(CPPDEFINES=[
	("MONITOR_SPEED", env.GetProjectOption("monitor_speed")),
	("UPLOAD_VIA_ESP_PROG", 1 if env.GetProjectOption("upload_protocol") == "esp-prog" else 0),
])