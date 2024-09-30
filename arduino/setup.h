// =========================================
// ==== WATER RESERVOIR LEVEL DETECTION ====
// =========================================

// Water Reservoir probe Threshold value
const int WATER_RESERVOIR_THRESHOLD = 300; // Set this value slightly higher than the probe's value when NOT touching water 

// Ultrasonic Level Detection settings
const int LOW_WATER_THRESHOLD = 20; // If enabled, minimum water percentage before triggering "Refill water reservoir"
const long distanceFull = 50;    // Distance (mm) between the Ultrasonic sensor and water level with a full reservoir
const long distanceEmpty = 200;  // Distance (mm) between the Ultrasonic sensor and the bottom of the empty reservoir
