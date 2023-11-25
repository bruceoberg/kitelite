#include "common.h"
#include "bluetooth.h"

#if ENABLE_BLUETOOTH

#define ENABLE_BLUETOOTH_SCANNER 0

#include "clock.h"
#include "input.h"
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
	bool g_fTraceClient = g_fTrace && true;
	bool g_fTraceSecurity = g_fTrace && true;

	static const char * s_pChzController = "JBL Click";
	static const BLEUUID s_bleuuidHumanInterfaceDeviceService(U16(ESP_GATT_UUID_HID_SVC));
	static const BLEUUID s_bleuuidHumanInterfaceDeviceReport(U16(ESP_GATT_UUID_HID_REPORT));

	class CScanner : public BLEAdvertisedDeviceCallbacks // tag: scanner
	{
	public:
						CScanner()
						: m_fIsActive(false)
							{ ; }
		
#if ENABLE_BLUETOOTH_SCANNER
		void	Init();
		void	Update();
#else // !ENABLE_BLUETOOTH_SCANNER
		inline void	Init() { ; };
		inline void	Update() { ; };
#endif // !ENABLE_BLUETOOTH_SCANNER

	protected:

		// for BLEAdvertisedDeviceCallbacks

	    void onResult(BLEAdvertisedDevice blead);

		void OnScanComplete(BLEScanResults blesr);
		static void OnScanCompleteStatic(BLEScanResults blesr);

		bool				m_fIsActive;
		
		static const int	s_dTScan = 5;
	};

	CScanner g_scanner;

	class CClient : public BLEClientCallbacks // tag: client
	{
	public:
					CClient()
					: m_state(STATE_Idle),
					  m_usecState(UsecNow()),
					  m_blea(""),
					  m_ebat(),
					  m_pBlec(BLEDevice::createClient())
						{ m_pBlec->setClientCallbacks(this); }
		
		void		RequestConnect(BLEAddress blea, esp_ble_addr_type_t ebat);
		void		RequestConnect(BLEAdvertisedDevice & blead)
						{ RequestConnect(blead.getAddress(), blead.getAddressType()); }

		void		Update();

	protected:

		enum STATE
		{
			STATE_Idle,
			STATE_ReadyToConnect,
			STATE_Connected,
			STATE_ReadyToIdle,

			STATE_Max,
			STATE_Min = 0,
			STATE_Nil = -1
		};

		const char * PChzFromState(STATE state)
		{
			if (state == STATE_Nil)
				return "Nil";

			if (state >= STATE_Min && state < STATE_Max)
			{
				static const char * s_mpStatePchz[] =
				{
					"Idle",
					"ReadyToConnect",
					"Connected",
					"ReadyToIdle",
				};

				return s_mpStatePchz[state];
			}

			return "<Unknown>";
		}

		// for BLEClientCallbacks

  		void		onConnect(BLEClient * pBlec);
  		void		onDisconnect(BLEClient * pBlec);

		void		OnNotify(
						BLERemoteCharacteristic * pBlerc,
						uint8_t * pB,
						size_t cB,
						bool fIsNotify);
		static void	OnNotifyStatic(
						BLERemoteCharacteristic * pBlerc,
						uint8_t * pB,
						size_t cB,
						bool fIsNotify);

		void		SetState(STATE state);

		STATE		m_state;
		USEC		m_usecState;

		BLEAddress	m_blea;		// address of device to connect to
		esp_ble_addr_type_t		// type of address
					m_ebat;
		BLEClient *	m_pBlec;	// client (never freed because BLEDevice keeps a pointer)
		
		static const int	s_dTScan = 5;
	};

	CClient g_client;

	class CSecurity : public BLESecurity, BLESecurityCallbacks // tag: security
	{
	public:
					CSecurity()
					: BLESecurity(),
					  BLESecurityCallbacks()
						{ ; }
		
		void		Init()
						{
							BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
							BLEDevice::setSecurityCallbacks(this);

							setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
							setCapability(ESP_IO_CAP_NONE);
							setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
						}

	protected:

		// BLESecurityCallbacks

		uint32_t onPassKeyRequest();
		bool onConfirmPIN(uint32_t pin);
		void onPassKeyNotify(uint32_t pass_key);
		bool onSecurityRequest();
		void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl);
	};

	CSecurity g_security;
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

	if (!blead.isAdvertisingService(s_bleuuidHumanInterfaceDeviceService))
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

	g_client.RequestConnect(blead);
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

#if ENABLE_BLUETOOTH_SCANNER
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
#endif // ENABLE_BLUETOOTH_SCANNER

void CClient::RequestConnect(BLEAddress blea, esp_ble_addr_type_t ebat)
{
	if (m_state != STATE_Idle)
		return;

	m_blea = blea;
	m_ebat = ebat;

	TRACE(
		g_fTraceClient,
		"[BT] Client Request for: %s(%d)\n",
		m_blea.toString().c_str(),
		m_ebat);

	SetState(STATE_ReadyToConnect);
}

void CClient::Update()
{
	switch (m_state)
	{
	case STATE_Idle:
		if (Input::FIsKeyDown(Input::KEY_OnBoard0))
		{
			BLEAddress blea("ac:b1:ee:94:a0:ed");
			esp_ble_addr_type_t ebat = BLE_ADDR_TYPE_PUBLIC;

			RequestConnect(blea, ebat);
		}
		break;
	case STATE_ReadyToConnect:
		{
			assert(!m_pBlec->isConnected());

			if (!m_pBlec->connect(m_blea, m_ebat))
			{
				TRACE(g_fTraceClient, "[BT] Client can't connect\n");

				SetState(STATE_Idle);
				return;
			}

			// set state now so any service/characteristic errors get cleaned up by SetState()

			SetState(STATE_Connected);

		    BLERemoteService * pBlers = m_pBlec->getService(s_bleuuidHumanInterfaceDeviceService);

			if (pBlers == nullptr)
			{
				TRACE(g_fTraceClient, "[BT] Client can't get HID service\n");

				SetState(STATE_Idle);
				return;
			}

			BLERemoteCharacteristic * pBlerc = pBlers->getCharacteristic(s_bleuuidHumanInterfaceDeviceReport);
      		
			if (pBlerc == nullptr)
			{
				TRACE(g_fTraceClient, "[BT] Client can't get report characteristic\n");

				SetState(STATE_Idle);
				return;
			}

			// NOTE sample code here loops until canNotify() returns true.
			//	https://github.com/chegewara/esp32-hid-keyboard-client/blob/master/src/main.cpp

			if (!pBlerc->canNotify())
			{
				TRACE(g_fTraceClient, "[BT] Client can't set report to notify\n");

				SetState(STATE_Idle);
				return;
			}

			pBlerc->registerForNotify(OnNotifyStatic);
		}
		break;

	case STATE_ReadyToIdle:
		{
			SetState(STATE_Idle);
		}
		break;
	}
}

void CClient::onConnect(BLEClient * pBlec)
{
	if (m_pBlec != pBlec)
		return;

	if (m_state != STATE_ReadyToConnect)
		return;
}

void CClient::onDisconnect(BLEClient * pBlec)
{
	if (m_pBlec != pBlec)
		return;

	if (m_state != STATE_Connected)
		return;

	SetState(STATE_ReadyToIdle);
}

void CClient::OnNotify(
	BLERemoteCharacteristic * pBlerc,
	uint8_t * pB,
	size_t cB,
	bool fIsNotify)
{
	TRACE(
		g_fTraceClient,
		"[BT] Notify from %s (%d bytes)",
		pBlerc->getUUID().toString().c_str(),
		cB);
	for (int iB = 0; iB < cB; iB++)
	{
		TRACE(g_fTraceClient && (iB % 16) == 0, "\n[BT]   ");
		TRACE(g_fTraceClient, " %#02x", pB[iB]);
	}
	TRACE(g_fTraceClient, "\n");
}

void CClient::OnNotifyStatic(
	BLERemoteCharacteristic * pBlerc,
	uint8_t * pB,
	size_t cB,
	bool fIsNotify)
{
	g_client.OnNotify(pBlerc, pB, cB, fIsNotify);
}

void CClient::SetState(STATE state)
{
	if (state == m_state)
		return;

	TRACE(g_fTraceClient, "[BT] Client state %s -> %s\n", PChzFromState(m_state), PChzFromState(state));

	STATE statePrev = m_state;
	m_state = state;
	m_usecState = UsecNow();

	switch (statePrev)
	{
	case STATE_Connected:
		if (m_pBlec->isConnected())
		{
			m_pBlec->disconnect();
		}
		break;
	}
}

uint32_t CSecurity::onPassKeyRequest()
{
	TRACE(g_fTraceSecurity, "[BT] reached %s\n", __PRETTY_FUNCTION__);
	return 987654;
}

bool CSecurity::onConfirmPIN(uint32_t pin)
{
	TRACE(g_fTraceSecurity, "[BT] reached %s, pin: %d\n", __PRETTY_FUNCTION__, pin);

	return true;
}

void CSecurity::onPassKeyNotify(uint32_t pass_key)
{
	TRACE(g_fTraceSecurity, "[BT] reached %s\n", __PRETTY_FUNCTION__);
}

bool CSecurity::onSecurityRequest()
{
	TRACE(g_fTraceSecurity, "[BT] reached %s\n", __PRETTY_FUNCTION__);

	return true; //?
}

void CSecurity::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
{
	TRACE(g_fTraceSecurity, "[BT] reached %s\n", __PRETTY_FUNCTION__);

	if (auth_cmpl.success)
	{
		TRACE(g_fTraceSecurity, "[BT]   auth_cmpl.success\n");
	}
	else
	{
		TRACE(g_fTraceSecurity, "[BT]   auth_cmpl.fail_reason: %#x\n", auth_cmpl.fail_reason);
	}
}

void BlueTooth::Startup()
{
	TRACE(g_fTrace, "[BT] Advertising as %s\n", PROJECT_NAME);

	BLEDevice::init(PROJECT_NAME);

	g_security.Init();
	g_scanner.Init();
}

void BlueTooth::Update()
{
	g_scanner.Update();
	g_client.Update();
}

#endif // ENABLE_BLUETOOTH
