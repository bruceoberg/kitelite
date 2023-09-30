# extra script to expose platformio.ini values to C++ code

env = DefaultEnvironment()

env.Append(CPPDEFINES=[
	("MONITOR_SPEED", env.GetProjectOption("monitor_speed"))
])