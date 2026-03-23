#include "motion.h"

#if ENABLE_MOTION

#include "common.h"
#include "clock.h"
#include "trace.h"

#include "etl/vector.h"

#include "Adafruit_Sensor_Calibration.h"
#include "Adafruit_LIS3MDL.h"
#include "Adafruit_ISM330DHCX.h"



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
}


namespace MotionCal
{

	class CReader // tag: reader
	{
	public:
					CReader()
					: m_state(STATE_Header),
					  m_usecState(UsecNow()),
					  m_iB(0),
					  m_aB()
						{ ; }
		
		void		Update();

	protected:

		enum STATE
		{
			STATE_Header,
			STATE_Values,

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
					"Header",
					"Values",
				};
				static_assert(STATE_Max == DIM(s_mpStatePchz));

				return s_mpStatePchz[state];
			}

			return "<Unknown>";
		}

		void		SetState(STATE state);

		// see receiveCalibration() and crc16_update() here:
		//	https://github.com/adafruit/Adafruit_AHRS/blob/master/examples/calibration/calibration.ino

		bool		FIsCrcCorrect() const;
		void		SetCalib(Adafruit_Sensor_Calibration * pCalib) const;

		STATE		m_state;
		USEC		m_usecState;

		int			m_iB;			// may point into s_aBHeader or m_aB
		U8			m_aB[68];		// 16 floats (4 bytes each) + 2 byte header + 2 byte crc correction

		static U8	s_aBHeader[];	// byte
	};

	U8 CReader::s_aBHeader[] = { 117, 84 };
	CReader g_reader;


	void CReader::Update()
	{
		STATE statePrev = m_state;

		while (true)
		{
			statePrev = m_state;

			switch (m_state)
			{
			case STATE_Header:
				if (Serial.available())
				{
					assert(m_iB < DIM(s_aBHeader));

					if (Serial.read() == s_aBHeader[m_iB])
					{
						// keep header in buffer for crc later

						m_aB[m_iB] = s_aBHeader[m_iB];

						++m_iB;

						if (m_iB == DIM(s_aBHeader))
						{
							SetState(STATE_Values);
						}
					}
					else
					{
						m_iB = 0;
					}
				}
				break;

			case STATE_Values:
				if (Serial.available())
				{
					assert(m_iB < DIM(m_aB));

					m_aB[m_iB] = Serial.read();
					++m_iB;

					if (m_iB < DIM(m_aB))
						break;

					if (FIsCrcCorrect())
					{
						SetCalib(&g_calib);
					}

					SetState(STATE_Header);
				}
				break;
			}

			if (statePrev == m_state)
				break;
		};
	}

	void CReader::SetState(STATE state)
	{
		if (state == m_state)
			return;

		m_state = state;
		m_usecState = UsecNow();

		// new state

		switch (m_state)
		{
		case STATE_Header:
			m_iB = 0;
			break;

		case STATE_Values:
			assert(m_iB == DIM(s_aBHeader));
			break;
		}
	}

	bool CReader::FIsCrcCorrect() const
	{
		U16 crc = 0xFFFF;

		for (const auto & b : m_aB)
		{
			crc = Adafruit_Sensor_Calibration::crc16_update(crc, b);
		}

		return crc == 0;
	}

	void CReader::SetCalib(Adafruit_Sensor_Calibration *pCalib) const
	{
		// incoming floats as written by:
		//	MotionCal - https://github.com/PaulStoffregen/MotionCal/blob/master/rawdata.c
		//	SensorCal - https://github.com/bruceoberg/SensorCal/blob/main/src/serialdata.cpp
		// see: send_calibration() in both files

		enum IG
		{

			IG_AccelZeroG_X,
			IG_AccelZeroG_Y,
			IG_AccelZeroG_Z,

			IG_GyroZeroRate_X,
			IG_GyroZeroRate_Y,
			IG_GyroZeroRate_Z,

			IG_MagHardIron_X,
			IG_MagHardIron_Y,
			IG_MagHardIron_Z,

			IG_MagField,

			IG_MagSoftIron_XX,
			IG_MagSoftIron_YY,
			IG_MagSoftIron_ZZ,
			IG_MagSoftIron_XY,
			IG_MagSoftIron_XZ,
			IG_MagSoftIron_YZ,

			IG_Max,

			// only 6 values for the soft iron matrix are sent because it's symetric

			IG_MagSoftIron_YX = IG_MagSoftIron_XY,
			IG_MagSoftIron_ZX = IG_MagSoftIron_XZ,
			IG_MagSoftIron_ZY = IG_MagSoftIron_YZ,

			IG_Min = 0,
			IG_Nil = -1
		};

		float aG[IG_Max];

		// why not a union? because there are 2 bytes before and after the floats (header and crc),
		//	and these mess up alignment of the floats.

		static_assert(sizeof(m_aB) == sizeof(aG) + 4);
		memcpy(aG, &m_aB[2], sizeof(aG));

		pCalib->accel_zerog[0] = aG[IG_AccelZeroG_X];
		pCalib->accel_zerog[1] = aG[IG_AccelZeroG_Y];
		pCalib->accel_zerog[2] = aG[IG_AccelZeroG_Z];

		pCalib->gyro_zerorate[0] = aG[IG_GyroZeroRate_X];
		pCalib->gyro_zerorate[1] = aG[IG_GyroZeroRate_Y];
		pCalib->gyro_zerorate[2] = aG[IG_GyroZeroRate_Z];

		pCalib->mag_hardiron[0] = aG[IG_MagHardIron_X];
		pCalib->mag_hardiron[1] = aG[IG_MagHardIron_Y];
		pCalib->mag_hardiron[2] = aG[IG_MagHardIron_Z];

		pCalib->mag_field = aG[IG_MagField];

		pCalib->mag_softiron[0] = aG[IG_MagSoftIron_XX];
		pCalib->mag_softiron[1] = aG[IG_MagSoftIron_YX];
		pCalib->mag_softiron[2] = aG[IG_MagSoftIron_XZ];
		pCalib->mag_softiron[3] = aG[IG_MagSoftIron_YX];
		pCalib->mag_softiron[4] = aG[IG_MagSoftIron_YY];
		pCalib->mag_softiron[5] = aG[IG_MagSoftIron_YZ];
		pCalib->mag_softiron[6] = aG[IG_MagSoftIron_ZX];
		pCalib->mag_softiron[7] = aG[IG_MagSoftIron_ZY];
		pCalib->mag_softiron[8] = aG[IG_MagSoftIron_ZZ];
	
		if (s_fLoadSaveCalibration)
		{
			bool fSaved = pCalib->saveCalibration();
			TRACE(s_fTrace, "[MOTION] saveCalibration: %s\n", fSaved ? "ok" : "failed");
		}
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

		// motioncal app wants integers, and the factors below are how the adafruit ahrs
		//	calibration code converts from float data from the sensors.
		//	https://github.com/adafruit/Adafruit_AHRS/blob/master/examples/calibration/calibration.ino
		// they are (roughly) the inverses of the *_PER_COUNT constants defined in MotionCal:
		//	https://github.com/PaulStoffregen/MotionCal/blob/master/imuread.h

		std::pair<Adafruit_Sensor *, float> s_aPairPSensRS[] = {
			{ g_pSensAccel, 8192.0f / 9.8f },
			{ g_pSensGyro, SENSORS_RADS_TO_DPS * 16.0f}, 
			{ g_pSensMagno, 10.0f },
		};

		// unwind readings into two arrays that we'll then print.

		etl::vector<float, DIM(s_aPairPSensRS) * 3> aryS;
		etl::vector<S32, DIM(s_aPairPSensRS) * 3> aryN;

		for (const auto & pair : s_aPairPSensRS)
		{
			auto & pSens = pair.first;
			auto & rS = pair.second;

			sensors_event_t senevt;
			pSens->getEvent(&senevt);

			for (int i = 0; i < 3; ++i)
			{
				aryS.push_back(senevt.data[i]);
				aryN.push_back(S32(rS * aryS.back()));
			}
		}

		TRACE(
			"Raw:"
				"%d,%d,%d,"
				"%d,%d,%d,"
				"%d,%d,%d\r\n",
			aryN[0], aryN[1], aryN[2],
			aryN[3], aryN[4], aryN[5],
			aryN[6], aryN[7], aryN[8]);

		TRACE(
			"Uni:"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f\r\n",
			aryS[0], aryS[1], aryS[2],
			aryS[3], aryS[4], aryS[5],
			aryS[6], aryS[7], aryS[8]);

		constexpr int s_cTraceCal1 = 50;
		constexpr int s_cTraceCal2 = 100;
		static int s_cTrace = 0;
		
		TRACE(
			(s_cTrace % s_cTraceCal1) == 0,
			"Cal1:"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f,"
				"%.5f\r\n",
			g_calib.accel_zerog[0], g_calib.accel_zerog[1], g_calib.accel_zerog[2],
			g_calib.gyro_zerorate[0], g_calib.gyro_zerorate[1], g_calib.gyro_zerorate[2],
			g_calib.mag_hardiron[0], g_calib.mag_hardiron[1], g_calib.mag_hardiron[2],
			g_calib.mag_field);

		TRACE(
			(s_cTrace % s_cTraceCal2) == 0,
			"Cal2:"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f,"
				"%.5f,%.5f,%.5f\r\n",
			g_calib.mag_softiron[0], g_calib.mag_softiron[1], g_calib.mag_softiron[2],
			g_calib.mag_softiron[3], g_calib.mag_softiron[4], g_calib.mag_softiron[5],
			g_calib.mag_softiron[6], g_calib.mag_softiron[7], g_calib.mag_softiron[8]);

		s_cTrace += 1;

		g_reader.Update();
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