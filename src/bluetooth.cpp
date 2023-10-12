#include "common.h"

#include "bluetooth.h"
#include "trace.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

namespace BlueTooth
{
	bool g_fTrace = true;
	bool g_fTraceScan = g_fTrace && false;
	bool g_fTraceDiscovery = g_fTrace && true;

	static const char * s_pChzController = "JBL Click";
	static const BLEUUID s_bleuuidHumanInterfaceDevice(U16(ESP_GATT_UUID_HID_SVC));

	class CScanner : public BLEAdvertisedDeviceCallbacks // tag: scanner
	{
	public:
						CScanner()
						: m_fIsActive(false)
							{ ; }
		
		void	Init();
		void	Update();

	protected:

		// for BLEAdvertisedDeviceCallbacks

	    void onResult(BLEAdvertisedDevice blead);

		void OnScanComplete(BLEScanResults blesr);
		static void OnScanCompleteStatic(BLEScanResults blesr);

		bool				m_fIsActive;
		
		static const int	s_dTScan = 5;
	};

	CScanner g_scanner;
}

using namespace BlueTooth;

void CScanner::onResult(BLEAdvertisedDevice blead)
{
	const std::string & strName = blead.getName();
	
	if (strName.empty())
		return;

	if (strName != s_pChzController)
	{
		TRACE(g_fTraceDiscovery, "[BT] Skipping Device: %s\n", strName.c_str());
		return;
	}

	if (!blead.isAdvertisingService(s_bleuuidHumanInterfaceDevice))
	{
		TRACE(g_fTraceDiscovery, "[BT] %s @ %s: NO HID SUPPORT \n", strName.c_str(), blead.getAddress().toString().c_str());
		return;
	}

	TRACE(g_fTraceDiscovery, "[BT] %s @ %s FOUND\n", strName.c_str(), blead.getAddress().toString().c_str());
	for (int i = 0; i < blead.getServiceUUIDCount(); ++i)
	{
		BLEUUID uuid = blead.getServiceUUID(i);
		TRACE(g_fTraceDiscovery, "[BT]   Uuid(%d): %s \n", i, uuid.toString().c_str());
	}
}

void CScanner::OnScanComplete(BLEScanResults blesr)
{
	TRACE(g_fTraceScan, "[BT] Scan complete!\n");
	m_fIsActive = false;
}

void CScanner::OnScanCompleteStatic(BLEScanResults blesr)
{
	g_scanner.OnScanComplete(blesr);
}

void CScanner::Init()
{
	BLEScan * pBlescan = BLEDevice::getScan();	// BLE lib maintains this as a singleton

	pBlescan->setAdvertisedDeviceCallbacks(this);

	// NOTE bruceo: sample code has this comment about setActiveScan:
	//	"active scan uses more power, but get results faster"
	// this may be correct, but setActiveScan(false) also appears to
	// limit the number of reported services when onResult()
	// is called.

	pBlescan->setActiveScan(true);
	pBlescan->setInterval(100);
	pBlescan->setWindow(99);  // less or equal setInterval value

	TRACE(g_fTraceScan, "[BT] Scan starting...\n");
	static const bool s_fContinuePreviousScan = false;	// aka "leave the extant list as is?"

	pBlescan->start(s_dTScan, OnScanCompleteStatic, s_fContinuePreviousScan);
	m_fIsActive = true;
}

void CScanner::Update()
{
	if (m_fIsActive)
		return;

	TRACE(g_fTraceScan, "[BT] Rescan starting...\n");
	static const bool s_fContinuePreviousScan = true;	// aka "leave the extant list as is?"
	BLEScan * pBlescan = BLEDevice::getScan();	// BLE lib maintains this as a singleton

	pBlescan->start(s_dTScan, OnScanCompleteStatic, s_fContinuePreviousScan);
	m_fIsActive = true;
}

void BlueTooth::Startup()
{
	TRACE(g_fTrace, "[BT] Advertising as %s\n", PROJECT_NAME);

	BLEDevice::init(PROJECT_NAME);

	g_scanner.Init();
}

void BlueTooth::Update()
{
	g_scanner.Update();
}

