#include "trace.h"

void Trace::Startup()
{
	Serial.begin(MONITOR_SPEED);
}
