//adapted from https://github.com/TKJElectronics/KalmanFilter/blob/master/Kalman.h

typedef struct kalman {
	float angle; // The angle calculated by the Kalman filter - part of the 2x1 state vector
	float bias; // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
	float rate; // Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate
	float P[2][2]; // Error covariance matrix - This is a 2x2 matrix
} Kalman;


// The angle should be in degrees and the rate should be in degrees per second and the delta time in seconds
// modifies the kalman values in place (struct passed by ref)
float kalman_update_angle(Kalman *kalman, float newAngle, float newRate, float dt);
