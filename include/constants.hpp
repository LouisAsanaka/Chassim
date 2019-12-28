#pragma once

// UI constants
#define MENU_BAR_HEIGHT 30
#define POINTS_LIST_COLUMN_WIDTH 110
#define POINTS_LIST_WIDTH (POINTS_LIST_COLUMN_WIDTH * 3 + 5)

#define POINT_CIRCLE_RADIUS 12

// Max frame rate
#define FRAMERATE 120.0f
#define IDEAL_DT (1 / FRAMERATE)

// Meters to inches
#define METERS2INCHES 39.3701
// Inches to meters
#define INCHES2METERS 0.0254
// Inches / 100 ms to m/s
#define INPP100MS2MPS 0.254f
// 100 ms to s
#define HUNDREDMS2S 0.1f