#include "trace.h"

namespace Trace
{
	bool g_fTrace = true;
}

using namespace Trace;

void Trace::Startup()
{
	// NOTE bruceo: when using the ESP-PROG board, serial output can go to either the onboard
	//	USB port or via the onboard UART pins though the ESP-PROG "PROG" interface.

	bool fTraceViaOnboardUart = UPLOAD_VIA_ESP_PROG;
	S8 pinRx = fTraceViaOnboardUart ? RX : -1;
	S8 pinTx = fTraceViaOnboardUart ? TX : -1;
	static const U32 s_configDefault = SERIAL_8N1;	// from arduino's HardwareSerial::begin() declaration

	Serial.begin(MONITOR_SPEED, s_configDefault, pinRx, pinTx);

	TRACE(g_fTrace, "reached '%s'\n", __PRETTY_FUNCTION__);
}
