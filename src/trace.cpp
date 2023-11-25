#include "trace.h"
#include "clock.h"

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

	static const float s_dTSerialSettleMax = 10.0f; // give TTY time to get initialized
	CUpStamp usSerialSettle;

	while (!Serial && usSerialSettle.DT() < s_dTSerialSettleMax)
	{
		delay(100);
	}

	TRACE(g_fTrace, "\n\n[TRACE] reached '%s' after USB wait of %0.1fs\n", __PRETTY_FUNCTION__, usSerialSettle.DT());
}
