//adapted from https://github.com/TKJElectronics/KalmanFilter/blob/master/Kalman.cpp

#include "kalman.h"

const float Q_angle = 0.005f; // Process noise variance for the accelerometer (increase to follow input, decrease to smooth output)
const float Q_bias = 0.003f; // Process noise variance for the gyro bias (increase to follow input, decrease to smooth output)
const float R_measure = 0.01f; // Measurement noise variance - this is actually the variance of the measurement noise

float kalman_update_angle(Kalman *kalman, float newAngle, float newRate, float dt) {

	// Discrete Kalman filter time update equations - Time Update ("Predict")
	// Update xhat - Project the state ahead
	/* Step 1 */
	kalman->rate = newRate - kalman->bias;
	kalman->angle += dt * kalman->rate;

	// Correct angle over/underflow
	if(kalman->angle > 180) {
		kalman->angle -= 360.0;
	} else if(kalman->angle < -180) {
		kalman->angle += 360.0;
	}

	// Update estimation error covariance - Project the error covariance ahead
	/* Step 2 */
	kalman->P[0][0] += dt * (dt*kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + Q_angle);
	kalman->P[0][1] -= dt * kalman->P[1][1];
	kalman->P[1][0] -= dt * kalman->P[1][1];
	kalman->P[1][1] += Q_bias * dt;

	// Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
	// Calculate Kalman gain - Compute the Kalman gain
	/* Step 4 */
	float S = kalman->P[0][0] + R_measure; // Estimate error
	/* Step 5 */
	float K[2]; // Kalman gain - This is a 2x1 vector
	K[0] = kalman->P[0][0] / S;
	K[1] = kalman->P[1][0] / S;

	// Calculate angle and bias - Update estimate with measurement zk (newAngle)
	/* Step 3 */
	float y = newAngle - kalman->angle; // Angle difference
	// Assume angle has overflowed from 180 to -180 or vice versa when difference is larger than 270
	if(y > 270.0) {
		y -= 360.0;
	} else if(y < -270.0) {
		y += 360.0;
	}

	/* Step 6 */
	kalman->angle += K[0] * y;
	kalman->bias += K[1] * y;
	// Correct angle over/underflow
	if(kalman->angle > 180) {
		kalman->angle -= 360.0;
	} else if(kalman->angle < -180) {
		kalman->angle += 360.0;
	}

	// Calculate estimation error covariance - Update the error covariance
	/* Step 7 */
	float P00_temp = kalman->P[0][0];
	float P01_temp = kalman->P[0][1];

	kalman->P[0][0] -= K[0] * P00_temp;
	kalman->P[0][1] -= K[0] * P01_temp;
	kalman->P[1][0] -= K[1] * P00_temp;
	kalman->P[1][1] -= K[1] * P01_temp;
}
