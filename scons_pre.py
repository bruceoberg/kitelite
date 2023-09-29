# extra script to expose platformio.ini values to C++ code

# alias of `env = DefaultEnvironment()`
Import('env')

env.Append(CPPDEFINES=[
	("MONITOR_SPEED", env.GetProjectOption("monitor_speed"))
])