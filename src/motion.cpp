#include "motion.h"

#if ENABLE_MOTION

#include "common.h"
#include "trace.h"

#include "etl/vector.h"

#include "Adafruit_Sensor_Calibration.h"
#include "Adafruit_LIS3MDL.h"
#include "Adafruit_ISM330DHCX.h"



namespace Motion
{
	constexpr bool s_fTrace = true;
	bool g_fTraceCalibration = false;

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
}


namespace MotionCal
{
	void TraceCalibration()
	{
		if (!s_fTrace)
			return;

		if (!g_fTraceCalibration)
			return;

		if (!g_pSensAccel || !g_pSensGyro || !g_pSensAccel)
		{
			TRACE("[MOTION] can't calibrate - missing a sensor\n");
			g_fTraceCalibration = false;
			return;
		}

		// motioncal app wants integers, and the factors below are how the adafruit ahrs
		//	calibration code converts from float data from the sensors.
		//	https://github.com/adafruit/Adafruit_AHRS/blob/master/examples/calibration/calibration.ino

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
				"%.2f,%.2f,%.2f,"
				"%.4f,%.4f,%.4f,"
				"%.2f,%.2f,%.2f\r\n",
			aryS[0], aryS[1], aryS[2],
			aryS[3], aryS[4], aryS[5],
			aryS[6], aryS[7], aryS[8]);

		constexpr int s_cTraceCal1 = 50;
		static int s_cTrace = 0;
		
		TRACE(
			(s_cTrace % s_cTraceCal1) == 0,
			"Cal1:"
				"%.3f,%.3f,%.3f,"
				"%.3f,%.3f,%.3f,"
				"%.3f,%.3f,%.3f,"
				"%.3f\r\n",
			g_calib.accel_zerog[0], g_calib.accel_zerog[1], g_calib.accel_zerog[2],
			g_calib.gyro_zerorate[0], g_calib.gyro_zerorate[1], g_calib.gyro_zerorate[2],
			g_calib.mag_hardiron[0], g_calib.mag_hardiron[1], g_calib.mag_hardiron[2],
			g_calib.mag_field);

		s_cTrace += 1;

		delay(10);
 	}
}


void Motion::Update()
{
	MotionCal::TraceCalibration();
}

#endif // ENABLE_MOTION