// user constants
const float TEMP_THRESHOLD = 30.00;  // units: deg celsius
const float GYRO_THRESHOLD = 300.00;	// units: deg/sec
const float ACCEL_THRESHOLD = 0.1;	//units: g
const float GYRO_THRESHOLD_SQUARED = GYRO_THRESHOLD*GYRO_THRESHOLD;
const float ACCEL_THRESHOLD_SQUARED = ACCEL_THRESHOLD*ACCEL_THRESHOLD;
const float MAG_THRESHOLD = 90;

// internal constants
const uint16_t DISPLAY_DT = 500;

struct Warnings {//record last warning time so it doesn't flood uart
	uint32_t fall;
	uint32_t sudden_move;
	uint32_t fever;
	uint32_t posture;
};
struct Warnings warning_dt = {
		.fall = 5000,
		.sudden_move = 5000,
		.fever = 5000,
		.posture = 5000
};

const float RAD_TO_DEG = 57.295779513082321;
const float DEG_TO_RAD = 0.0174532925199433;

//buzzer stuff
#define TIM_FREQ 80000000
#define BUZZER_FREQ 1000

//cube stuff
int points[8][2]; // eight 2D points for the cube, values will be calculated in the code

int orig_points [8][3] = {  // eight 3D points - set values for 3D cube
	{-1,-1, 1},
	{1,-1,1},
	{1,1,1},
	{-1,1,1},
	{-1,-1,-1},
	{1,-1,-1},
	{1,1,-1},
	{-1,1,-1}
};

float rotated_3d_points [8][3];
float z_offset = -4;            // offset on Z axis
float cube_size = 50.0;           // cube size (multiplier)
