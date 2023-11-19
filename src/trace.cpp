#include "trace.h"

namespace Trace
{
	bool g_fTrace = true;
}

using namespace Trace;

void Trace::Startup()
{
#if PLAT_FEATHER_V2
	// NOTE bruceo: when using the ESP-PROG board, serial output can go to either the onboard
	//	USB port or via the onboard UART pins though the ESP-PROG "PROG" interface.

	bool fTraceViaOnboardUart = UPLOAD_VIA_ESP_PROG;
	S8 pinRx = fTraceViaOnboardUart ? RX : -1;
	S8 pinTx = fTraceViaOnboardUart ? TX : -1;
	static const U32 s_configDefault = SERIAL_8N1;	// from arduino's HardwareSerial::begin() declaration

	Serial.begin(MONITOR_SPEED, s_configDefault, pinRx, pinTx);
#else // !PLAT_FEATHER_V2
	// NOTE bruceo: still investigating ESP-PROG on S3 feathers.

	Serial.begin(MONITOR_SPEED);
#endif // !PLAT_FEATHER_V2

	while (!Serial) {
		delay(100); // wait for native usb
	}

	TRACE(g_fTrace, "reached '%s'\n", __PRETTY_FUNCTION__);
}
