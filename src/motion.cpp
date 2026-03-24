#include "motion.h"

#if ENABLE_MOTION

#include "common.h"
#include "clock.h"
#include "trace.h"

#include "libcalib/protocol.h"

#include "Adafruit_Sensor_Calibration.h"
#include "Adafruit_LIS3MDL.h"
#include "Adafruit_ISM330DHCX.h"

using namespace libcalib;
using namespace libcalib::Protocol;

namespace Motion
{
	constexpr bool s_fTrace = true;
	bool g_fTraceCalibration = true;

	constexpr bool s_fLoadSaveCalibration = true;

	Adafruit_Sensor * g_pSensAccel = nullptr;
	Adafruit_Sensor * g_pSensGyro = nullptr;
	Adafruit_Sensor * g_pSensMagno = nullptr;

	Adafruit_LIS3MDL g_lis3mdl;
	Adafruit_ISM330DHCX g_ism330dhcx;

	Adafruit_Sensor_Calibration_EEPROM g_calib;
}

using namespace Motion;

namespace MotionCal
{
	// protocol adapter: serial writer

	struct CSerialWriter : IWriter	// tag = serialwtr
	{
		void Write(size_t cB, const uint8_t * pB) override
		{
			Serial.write(pB, cB);
		}
	};

	// protocol adapter: serial reader

	struct CSerialReader : IReader	// tag = serialrdr
	{
		size_t CbRead(size_t cBMax, uint8_t * pB) override
		{
			size_t cBAvail = Serial.available();
			if (cBAvail == 0)
				return 0;

			if (cBAvail > cBMax)
				cBAvail = cBMax;

			return Serial.readBytes(pB, cBAvail);
		}
	};

	// protocol adapter: calibration receiver

	struct CCalReceiver : IReceiver	// tag = calrcvr
	{
		void OnMagCal(const Mag::SCal & cal) override
		{
			// convert Mag::SCal → Adafruit_Sensor_Calibration fields

			g_calib.mag_hardiron[0] = cal.m_vecV.x;
			g_calib.mag_hardiron[1] = cal.m_vecV.y;
			g_calib.mag_hardiron[2] = cal.m_vecV.z;

			g_calib.mag_field = cal.m_sB;

			const SMatrix3 & w = cal.m_matWInv;
			g_calib.mag_softiron[0] = w.vecX.x;
			g_calib.mag_softiron[1] = w.vecX.y;
			g_calib.mag_softiron[2] = w.vecX.z;
			g_calib.mag_softiron[3] = w.vecY.x;
			g_calib.mag_softiron[4] = w.vecY.y;
			g_calib.mag_softiron[5] = w.vecY.z;
			g_calib.mag_softiron[6] = w.vecZ.x;
			g_calib.mag_softiron[7] = w.vecZ.y;
			g_calib.mag_softiron[8] = w.vecZ.z;

			if (s_fLoadSaveCalibration)
			{
				bool fSaved = g_calib.saveCalibration();
				TRACE(s_fTrace, "[MOTION] saveCalibration: %s\n", fSaved ? "ok" : "failed");
			}
		}
	};

	CSerialWriter s_serialwtr;
	CSerialReader s_serialrdr;
	CCalReceiver s_calrcvr;
	CManager s_protomgr(VER_Imucal);
}

void Motion::Startup()
{
	TRACE(s_fTrace, "[MOTION] reached '%s'\n", __PRETTY_FUNCTION__);

	if (!g_ism330dhcx.begin_I2C())
	{
		TRACE("[MOTION] can't find ISM330DHCX\n");
		return;
	}

	if (!g_lis3mdl.begin_I2C())
	{
		TRACE("[MOTION] can't find LIS3MDL\n");
		return;
	}

	g_ism330dhcx.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
	g_ism330dhcx.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
	g_lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);

	// set slightly above refresh rate

	g_ism330dhcx.setAccelDataRate(LSM6DS_RATE_104_HZ);
	g_ism330dhcx.setGyroDataRate(LSM6DS_RATE_104_HZ);
	g_lis3mdl.setDataRate(LIS3MDL_DATARATE_1000_HZ);
	g_lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
	g_lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);

	g_pSensAccel = g_ism330dhcx.getAccelerometerSensor();
	g_pSensGyro = g_ism330dhcx.getGyroSensor();
	g_pSensMagno = &g_lis3mdl;

	TRACE(s_fTrace, "[MOTION] found all sensors. previous i2c clock: %d\n", Wire.getClock());

	if (g_fTraceCalibration)
	{
		g_pSensAccel->printSensorDetails();
		g_pSensGyro->printSensorDetails();
		g_pSensMagno->printSensorDetails();
	}

	// NOTE bruceo: this is from the adafruit ahrs "calibration" example.
	//	https://github.com/adafruit/Adafruit_AHRS/blob/master/examples/calibration/calibration.ino
	// dunno if this is only necessary for calibration, but i believe it only affects the I2C bus speed.

	constexpr U32 s_hzI2CBus = 400000; // 400KHz

	Wire.setClock(s_hzI2CBus);

	if (s_fLoadSaveCalibration)
	{
		bool fLoaded = g_calib.loadCalibration();
		TRACE(s_fTrace, "[MOTION] loadCalibration: %s\n", fLoaded ? "ok" : "no stored data");
	}

	MotionCal::s_protomgr.Init(&MotionCal::s_serialwtr, &MotionCal::s_serialrdr, &MotionCal::s_calrcvr);
}


namespace MotionCal
{
	// build a Mag::SCal from the current Adafruit_Sensor_Calibration state

	Mag::SCal CalFromAdafruit()
	{
		Mag::SCal cal;

		cal.m_vecV.x = g_calib.mag_hardiron[0];
		cal.m_vecV.y = g_calib.mag_hardiron[1];
		cal.m_vecV.z = g_calib.mag_hardiron[2];

		cal.m_sB = g_calib.mag_field;

		cal.m_matWInv.vecX.x = g_calib.mag_softiron[0];
		cal.m_matWInv.vecX.y = g_calib.mag_softiron[1];
		cal.m_matWInv.vecX.z = g_calib.mag_softiron[2];
		cal.m_matWInv.vecY.x = g_calib.mag_softiron[3];
		cal.m_matWInv.vecY.y = g_calib.mag_softiron[4];
		cal.m_matWInv.vecY.z = g_calib.mag_softiron[5];
		cal.m_matWInv.vecZ.x = g_calib.mag_softiron[6];
		cal.m_matWInv.vecZ.y = g_calib.mag_softiron[7];
		cal.m_matWInv.vecZ.z = g_calib.mag_softiron[8];

		return cal;
	}

	void TraceCalibration()
	{
		if (!s_fTrace)
			return;

		if (!g_fTraceCalibration)
			return;

		if (!g_pSensAccel || !g_pSensGyro || !g_pSensMagno)
		{
			TRACE("[MOTION] can't calibrate - missing a sensor\n");
			g_fTraceCalibration = false;
			return;
		}

		// read sensors — SI units straight from Adafruit: m/s², rad/s, µT

		sensors_event_t evtAccel, evtGyro, evtMag;
		g_pSensAccel->getEvent(&evtAccel);
		g_pSensGyro->getEvent(&evtGyro);
		g_pSensMagno->getEvent(&evtMag);

		s_protomgr.SendSensorData(
			evtAccel.data[0], evtAccel.data[1], evtAccel.data[2],
			evtGyro.data[0],  evtGyro.data[1],  evtGyro.data[2],
			evtMag.data[0],   evtMag.data[1],   evtMag.data[2]);

		// echo stored calibration periodically

		constexpr int s_cTraceCal = 50;
		static int s_cTrace = 0;

		if ((s_cTrace % s_cTraceCal) == 0)
		{
			s_protomgr.SendMagCal(CalFromAdafruit());
		}

		s_cTrace += 1;

		// drain incoming serial (receives binary calibration packets from SensorCal)

		s_protomgr.Update();
	}
}

void Motion::Update()
{
	MotionCal::TraceCalibration();
}

bool Motion::FIsSpecial()
{
#ifdef ADAFRUIT_SENSOR_CALIBRATION_ACCEL_GYRO_ALIGN
	return true;
#else
	return false;
#endif

}

#endif // ENABLE_MOTION