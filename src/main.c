/*
Copyright (C) 2016 Mark Reed

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pebble.h"
#include "effect_layer.h"


static Window *window;

EffectLayer* effect_layer_invert;

EffectLayer* effect_layer_col_hr1;
EffectLayer* effect_layer_col_hr2;
EffectLayer* effect_layer_col_min1;
EffectLayer* effect_layer_col_min2;

static AppSync sync;
static uint8_t sync_buffer[256];

static int invert;
static int bluetoothvibe;
static int hourlyvibe;
static int datesep;
static int hour_col1;
static int hour_col2;
static int min_col1;
static int min_col2;
static int health;

static bool appStarted = false;

enum {
  INVERT_KEY = 0x0,
  BLUETOOTHVIBE_KEY = 0x1,
  HOURLYVIBE_KEY = 0x2,
  DATESEP_KEY = 0x3,
  HOUR_COL_KEY = 0x4,
  MIN_COL_KEY = 0x5,
  HOUR_COL2_KEY = 0x6,
  MIN_COL2_KEY = 0x7,
  HEALTH_KEY = 0x8,
};

	// initializing colors
struct EffectColorpair {
  GColor firstColor;  // first color (target for colorize, one of set in colorswap)
  GColor secondColor; // second color (new color for colorize, other of set in colorswap)
};
  
EffectColorpair colorpair_a;
EffectColorpair colorpair_b;
EffectColorpair colorpair_c;
EffectColorpair colorpair_d;


GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_40;
GBitmap *img_battery_20;
GBitmap *img_battery_charge;
BitmapLayer *layer_batt_img;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

static GBitmap *footprint_icon;
static BitmapLayer *footprint_layer;

static GBitmap *bluetooth_image_on;
static BitmapLayer *bluetooth_layer_on;

static GBitmap *month_image;
static BitmapLayer *month_layer;

const int MONTH_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_JAN,
  RESOURCE_ID_IMAGE_FEB,
  RESOURCE_ID_IMAGE_MAR,
  RESOURCE_ID_IMAGE_APR,
  RESOURCE_ID_IMAGE_MAY,
  RESOURCE_ID_IMAGE_JUN,
  RESOURCE_ID_IMAGE_JUL,
  RESOURCE_ID_IMAGE_AUG,
  RESOURCE_ID_IMAGE_SEP,
  RESOURCE_ID_IMAGE_OCT,
  RESOURCE_ID_IMAGE_NOV,
  RESOURCE_ID_IMAGE_DEC
};

#define TOTAL_DATE_DIGITS 2	
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

static GBitmap *time_digitsa_images;
static BitmapLayer *time_digitsa_layers;

const int BIG_DIGIT_A_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUMA_0,
  RESOURCE_ID_IMAGE_NUMA_1,
  RESOURCE_ID_IMAGE_NUMA_2,
};

static GBitmap *time_digitsb_images;
static BitmapLayer *time_digitsb_layers;

const int BIG_DIGIT_B_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUMB_0,
  RESOURCE_ID_IMAGE_NUMB_1,
  RESOURCE_ID_IMAGE_NUMB_2,
  RESOURCE_ID_IMAGE_NUMB_3,
  RESOURCE_ID_IMAGE_NUMB_4,
  RESOURCE_ID_IMAGE_NUMB_5,
  RESOURCE_ID_IMAGE_NUMB_6,
  RESOURCE_ID_IMAGE_NUMB_7,
  RESOURCE_ID_IMAGE_NUMB_8,
  RESOURCE_ID_IMAGE_NUMB_9
};

static GBitmap *time_digitsc_images;
static BitmapLayer *time_digitsc_layers;

const int BIG_DIGIT_C_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUMC_0,
  RESOURCE_ID_IMAGE_NUMC_1,
  RESOURCE_ID_IMAGE_NUMC_2,
  RESOURCE_ID_IMAGE_NUMC_3,
  RESOURCE_ID_IMAGE_NUMC_4,
  RESOURCE_ID_IMAGE_NUMC_5,
};

static GBitmap *time_digitsd_images;
static BitmapLayer *time_digitsd_layers;

const int BIG_DIGIT_D_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUMD_0,
  RESOURCE_ID_IMAGE_NUMD_1,
  RESOURCE_ID_IMAGE_NUMD_2,
  RESOURCE_ID_IMAGE_NUMD_3,
  RESOURCE_ID_IMAGE_NUMD_4,
  RESOURCE_ID_IMAGE_NUMD_5,
  RESOURCE_ID_IMAGE_NUMD_6,
  RESOURCE_ID_IMAGE_NUMD_7,
  RESOURCE_ID_IMAGE_NUMD_8,
  RESOURCE_ID_IMAGE_NUMD_9
};

static PropertyAnimation *animation1, *animation2, *animation3, *animation4;
static TextLayer *steps_label, *dist_label;
static GFont  sq;



static void health_handler(HealthEventType event, void *context) {
  static char s_value_buffer[8];
  static char s_value_buffer2[8];
  if (event == HealthEventMovementUpdate) {
    // display the step count
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
    snprintf(s_value_buffer2, sizeof(s_value_buffer2), "%d", (int)health_service_sum_today(HealthMetricWalkedDistanceMeters));
	  
    text_layer_set_text(steps_label, s_value_buffer);
    text_layer_set_text(dist_label, s_value_buffer2);
  }
}

void change_hour1() {
	  
    switch (hour_col1) {
		
		case 0: //		
			colorpair_a.firstColor = GColorBlack;		
  			colorpair_a.secondColor = GColorBlack;
		break;	
		
		case 1: //default	
  			colorpair_a.firstColor = GColorWhite;
  			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_a.secondColor = GColorWhite;
		#endif	
		break;

		case 2: //
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorIcterine ;
		#endif	
		break;

		case 3: //
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorYellow  ;
		#endif	
		break;
		
		case 4: //
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorOrange;
		#endif	
		break;
		
		case 5: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorSunsetOrange;
		#endif	
		break;
		
		case 6: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorWindsorTan ;
		#endif	
		break;
		
		case 7: //  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorRed  ;
		#endif	
		break;
		
		case 8: //  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorDarkCandyAppleRed  ;
		#endif	
		break;
		
		case 9: //  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorIndigo ;
		#endif	
		break;
		
		case 10: //  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorPurple  ;
		#endif	
		break;

		case 11: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorShockingPink ;
		#endif	
		break;
		
		case 12: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorElectricBlue ;
		#endif	
		break;
		
		case 13: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorPictonBlue ;
		#endif	
		break; 
		
		case 14: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorBlueMoon   ;
		#endif	
		break; 
		
		case 15: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorBlue  ;
		#endif	
		break; 
		
		case 16: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorScreaminGreen   ;
		#endif	
		break; 
		
		case 17: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorGreen ;
		#endif	
		break; 
		
		case 18: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorMayGreen   ;
		#endif	
		break; 
		
		case 19: // 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor =  GColorDarkGreen ;
		#endif	
		break; 
	}    
}

void change_hour2() {
	  
    switch (hour_col2) {
		
		case 0: //		
			colorpair_b.firstColor = GColorBlack;		
  			colorpair_b.secondColor = GColorBlack;
		break;	
		
		case 1: //default	
  			colorpair_b.firstColor = GColorWhite;
  			colorpair_b.secondColor = GColorWhite;	
		break;

		case 2: //
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorIcterine ;
		#endif	
		break;

		case 3: //
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorYellow  ;
		#endif	
		break;
		
		case 4: //
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorOrange;
		#endif	
		break;
		
		case 5: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorSunsetOrange;
		#endif	
		break;
		
		case 6: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorWindsorTan ;
		#endif	
		break;
		
		case 7: //  
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorRed  ;
		#endif	
		break;
		
		case 8: //  
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorDarkCandyAppleRed  ;
		#endif	
		break;
		
		case 9: //  
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorPurple ;
		#endif	
		break;
		
		case 10: //  
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorIndigo  ;
		#endif	
		break;

		case 11: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorShockingPink ;
		#endif	
		break;
		
		case 12: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorElectricBlue ;
		#endif	
		break;
		
		case 13: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorPictonBlue ;
		#endif	
		break; 
		
		case 14: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorBlueMoon   ;
		#endif	
		break; 
		
		case 15: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorBlue  ;
		#endif	
		break; 
		
		case 16: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorScreaminGreen   ;
		#endif	
		break; 
		
		case 17: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorGreen ;
		#endif	
		break; 
		
		case 18: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorMayGreen  ;
		#endif	
		break; 
		
		case 19: // 
			colorpair_b.firstColor = GColorWhite;
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor =  GColorDarkGreen ;
		#endif	
		break; 
	}    
}

void change_min1() {

    switch (min_col1) {
		case 0: //		
			colorpair_c.firstColor = GColorBlack;		
  			colorpair_c.secondColor = GColorBlack;
		break;	
		
		case 1: //default	
  			colorpair_c.firstColor = GColorWhite;
  			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_c.secondColor = GColorWhite;
		#endif	
		break;

		case 2: //
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorIcterine ;
		#endif	
		break;

		case 3: //
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorYellow  ;
		#endif	
		break;
		
		case 4: //
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorOrange;
		#endif	
		break;
		
		case 5: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorSunsetOrange;
		#endif	
		break;
		
		case 6: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorWindsorTan ;
		#endif	
		break;
		
		case 7: //  
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorRed  ;
		#endif	
		break;
		
		case 8: //  
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorDarkCandyAppleRed  ;
		#endif	
		break;
		
		case 9: //  
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorPurple ;
		#endif	
		break;
		
		case 10: //  
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorIndigo  ;
		#endif	
		break;

		case 11: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorShockingPink ;
		#endif	
		break;
		
		case 12: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor =  GColorElectricBlue ;
		#endif	
		break;
		
		case 13: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorPictonBlue ;
		#endif	
		break; 
		
		case 14: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor =  GColorBlueMoon   ;
		#endif	
		break; 
		
		case 15: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor =  GColorBlue  ;
		#endif	
		break; 
		
		case 16: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorScreaminGreen   ;
		#endif	
		break; 
		
		case 17: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor =  GColorGreen ;
		#endif	
		break; 
		
		case 18: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor = GColorMayGreen  ;
		#endif	
		break; 
		
		case 19: // 
			colorpair_c.firstColor = GColorWhite;
			colorpair_c.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_c.secondColor =  GColorDarkGreen  ;
		#endif	
		break; 
    }    
}

void change_min2() {

    switch (min_col2) {
		case 0: //		
			colorpair_d.firstColor = GColorBlack;		
  			colorpair_d.secondColor = GColorBlack;
		break;	
		
		case 1: //default	
  			colorpair_d.firstColor = GColorWhite;
  			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_d.secondColor = GColorWhite;
		#endif	
		break;

		case 2: //
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorIcterine ;
		#endif	
		break;

		case 3: //
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorYellow  ;
		#endif	
		break;
		
		case 4: //
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorOrange;
		#endif	
		break;
		
		case 5: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorSunsetOrange;
		#endif	
		break;
		
		case 6: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorWindsorTan ;
		#endif	
		break;
		
		case 7: //GColorShockingPink  
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorRed  ;
		#endif	
		break;
		
		case 8: //GColorShockingPink  
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorDarkCandyAppleRed  ;
		#endif	
		break;
		
		case 9: //  
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorPurple ;
		#endif	
		break;
		
		case 10: //  
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorIndigo  ;
		#endif	
		break;

		case 11: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorShockingPink ;
		#endif	
		break;
		
		case 12: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorElectricBlue ;
		#endif	
		break;
		
		case 13: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorPictonBlue ;
		#endif	
		break; 
		
		case 14: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorBlueMoon   ;
		#endif	
		break; 
		
		case 15: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorBlue  ;
		#endif	
		break; 
		
		case 16: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor = GColorScreaminGreen   ;
		#endif	
		break; 
		
		case 17: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorGreen ;
		#endif	
		break; 
		
		case 18: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorMayGreen  ;
		#endif	
		break; 
		
		case 19: // 
			colorpair_d.firstColor = GColorWhite;
			colorpair_d.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_d.secondColor =  GColorDarkGreen  ;
		#endif	
		break; 
    }     
}

void change_background(bool invert) {
  if (invert && effect_layer_invert == NULL) {
    // Add inverter layer
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_PLATFORM_CHALK
    effect_layer_invert = effect_layer_create(GRect(0, 0, 180, 180));
#else
	effect_layer_invert = effect_layer_create(GRect(0, 0, 144, 168));
#endif
	layer_add_child(window_layer, effect_layer_get_layer(effect_layer_invert));
    effect_layer_add_effect(effect_layer_invert, effect_invert_bw_only, NULL);
  
  } else if (!invert && effect_layer_invert != NULL) {
    // Remove Inverter layer
   layer_remove_from_parent(effect_layer_get_layer(effect_layer_invert));
   effect_layer_destroy(effect_layer_invert);
   effect_layer_invert = NULL;
  }
   // No action required
}

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
	
  GRect bounds = gbitmap_get_bounds(*bmp_image);

  GRect main_frame = GRect(origin.x, origin.y, bounds.size.w, bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), main_frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

static void handle_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
        bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);
  } else {
		
 		 if (charge_state.charge_percent <= 20) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);

		} else if (charge_state.charge_percent <= 40) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);

        } else if (charge_state.charge_percent <= 60) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);
			 
		} else	if (charge_state.charge_percent <= 70) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_70);
			 
		} else if (charge_state.charge_percent <= 80) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
			 
		} else if (charge_state.charge_percent <= 90) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_90);
			 
		} else if (charge_state.charge_percent <= 99) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
			 
		} else {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
        } 
  }
}

static void toggle_bluetooth_icon(bool connected) {
	
  if(appStarted && !connected && bluetoothvibe) {
    //vibe!
    vibes_long_pulse();  
  }
 layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer_on), !connected);

}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	
if (units_changed & HOUR_UNIT) {

  if(appStarted && hourlyvibe) {
    //vibe!
    vibes_short_pulse();
  }

	
// update month and date
	
#ifdef PBL_PLATFORM_CHALK
set_container_image(&month_image, month_layer, MONTH_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(86, 102));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(86, 60));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(86, 70));
#else
set_container_image(&month_image, month_layer, MONTH_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(68, 95));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(68, 52));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(68, 62));	
#endif

//update hours
	
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);
	 
#ifdef PBL_PLATFORM_CHALK
  set_container_image(&time_digitsa_images, time_digitsa_layers, BIG_DIGIT_A_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(6, 200));
  set_container_image(&time_digitsb_images, time_digitsb_layers, BIG_DIGIT_B_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(46, 200));	
#else
  set_container_image(&time_digitsa_images, time_digitsa_layers, BIG_DIGIT_A_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(2, 200));
  set_container_image(&time_digitsb_images, time_digitsb_layers, BIG_DIGIT_B_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(34, 200));
#endif
	
// hours animation

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame1 = GRect(6, 200, 36, 169);
  GRect to_frame1 = GRect(6, 5, 36, 169);
#else
  GRect from_frame1 = GRect(2, 200, 29, 160);
  GRect to_frame1 = GRect(2, 4, 29, 160);
#endif
  // Schedule the next animation
  animation1 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsa_layers), &from_frame1, &to_frame1);
  animation_set_duration((Animation*)animation1, 1000);
  animation_set_delay((Animation*)animation1, 50);
  animation_set_curve((Animation*)animation1, AnimationCurveLinear);
  animation_schedule((Animation*)animation1);
	

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame2 = GRect(46, 200, 36, 169);
  GRect to_frame2 = GRect(46, 5, 36, 169);
#else
  GRect from_frame2 = GRect(34, 200, 29, 160);
  GRect to_frame2 = GRect(34, 4, 29, 160);
#endif
  // Schedule the next animation
  animation2 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsb_layers), &from_frame2, &to_frame2);
  animation_set_duration((Animation*)animation2, 1000);
  animation_set_delay((Animation*)animation2, 150);
  animation_set_curve((Animation*)animation2, AnimationCurveLinear);
  animation_schedule((Animation*)animation2);
		
	
	if (!clock_is_24h_style()) {

    if (display_hour/10 == 0) {
	      layer_set_hidden(bitmap_layer_get_layer(time_digitsa_layers), true);
	} else {
	      layer_set_hidden(bitmap_layer_get_layer(time_digitsa_layers), false);    
	} 
	}
	
}
	
// update minutes
	
if (units_changed & MINUTE_UNIT) {

#ifdef PBL_PLATFORM_CHALK
  set_container_image(&time_digitsc_images, time_digitsc_layers, BIG_DIGIT_C_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(98, 200));
  set_container_image(&time_digitsd_images, time_digitsd_layers, BIG_DIGIT_D_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(138, 200));		
#else
  set_container_image(&time_digitsc_images, time_digitsc_layers, BIG_DIGIT_C_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(81, 200));
  set_container_image(&time_digitsd_images, time_digitsd_layers, BIG_DIGIT_D_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(113,200));		
#endif
	  
// minutes animation

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame3 = GRect(98, 200, 36, 169);
  GRect to_frame3 = GRect(98, 5, 36, 169);
#else
  GRect from_frame3 = GRect(81, 200, 29, 160);
  GRect to_frame3 = GRect(81, 4, 29, 160);
#endif
  // Schedule the next animation
  animation3 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsc_layers), &from_frame3, &to_frame3);
  animation_set_duration((Animation*)animation3, 1000);
  animation_set_delay((Animation*)animation3, 300);
  animation_set_curve((Animation*)animation3, AnimationCurveEaseIn);
  animation_schedule((Animation*)animation3);
	
#ifdef PBL_PLATFORM_CHALK
  GRect from_frame4 = GRect(138, 358, 36, 169);
  GRect to_frame4 = GRect(138, 5, 36, 169);
#else
  GRect from_frame4 = GRect(113, 358, 29, 160);
  GRect to_frame4 = GRect(113, 4, 29, 160);
#endif
  // Schedule the next animation
  animation4 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsd_layers), &from_frame4, &to_frame4);
  animation_set_duration((Animation*)animation4, 1000);
  animation_set_delay((Animation*)animation4, 600);
  animation_set_curve((Animation*)animation4, AnimationCurveEaseIn);
  animation_schedule((Animation*)animation4);
	
  }
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {

	  
     case INVERT_KEY:
      invert = new_tuple->value->uint8 != 0;
	  persist_write_bool(INVERT_KEY, invert);
      change_background(invert);

      break;
	  
	case BLUETOOTHVIBE_KEY:
      bluetoothvibe = new_tuple->value->uint8 != 0;
	  persist_write_bool(BLUETOOTHVIBE_KEY, bluetoothvibe);
      break;      
	  
    case HOURLYVIBE_KEY:
      hourlyvibe = new_tuple->value->uint8 != 0;
	  persist_write_bool(HOURLYVIBE_KEY, hourlyvibe);	  
      break;	
	  	  
	case DATESEP_KEY:
      datesep = new_tuple->value->uint8 != 0;
	  persist_write_bool(DATESEP_KEY, datesep);	  
	  
	 if (datesep) {
		layer_set_hidden(bitmap_layer_get_layer(separator_layer), false); 
		
		layer_set_hidden(bitmap_layer_get_layer(month_layer), true);
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
		layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), true);
		}
				
	} else {
		layer_set_hidden(bitmap_layer_get_layer(separator_layer), true);

		layer_set_hidden(bitmap_layer_get_layer(month_layer), false);
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
		layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), false);
		}
	}
      break;
	  
    case HOUR_COL_KEY:
		hour_col1 = new_tuple->value->uint8;
		persist_write_bool(HOUR_COL_KEY, hour_col1);
	    change_hour1();

	break;
	  
    case MIN_COL_KEY:
		min_col1 = new_tuple->value->uint8;
		persist_write_bool(MIN_COL_KEY, min_col1);
		change_min1();

	break;
	  
    case HOUR_COL2_KEY:
		hour_col2 = new_tuple->value->uint8;
		persist_write_bool(HOUR_COL2_KEY, hour_col2);
	    change_hour2();

	break;
	  
    case MIN_COL2_KEY:
		min_col2 = new_tuple->value->uint8;
		persist_write_bool(MIN_COL2_KEY, min_col2);
		change_min2();

	break;
	  
	case HEALTH_KEY:
		health = new_tuple->value->uint8 !=0;
		persist_write_bool(HEALTH_KEY, health);
	  
        if (health) {
          health_service_events_subscribe(health_handler, NULL);
 
					layer_set_hidden(bitmap_layer_get_layer(footprint_layer), false); 
					layer_set_hidden(text_layer_get_layer(steps_label), false); 
					layer_set_hidden(text_layer_get_layer(dist_label), false); 
            
        } else {
            health_service_events_unsubscribe();
				
					layer_set_hidden(bitmap_layer_get_layer(footprint_layer), true); 
					layer_set_hidden(text_layer_get_layer(steps_label), true); 
					layer_set_hidden(text_layer_get_layer(dist_label), true); 
				
			}
						
	break;
	  
  }
	
  // Refresh display immediately when changes are made

  // Get current time
  time_t now = time( NULL );
  struct tm *tick_time = localtime( &now );

  // Force update to Refresh display
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );
	
}

static void init(void) {
  memset(&time_digitsa_layers, 0, sizeof(time_digitsa_layers));
  memset(&time_digitsa_images, 0, sizeof(time_digitsa_images)); 
  
  memset(&time_digitsb_layers, 0, sizeof(time_digitsb_layers));
  memset(&time_digitsb_images, 0, sizeof(time_digitsb_images));
	
  memset(&time_digitsc_layers, 0, sizeof(time_digitsc_layers));
  memset(&time_digitsc_images, 0, sizeof(time_digitsc_images));
	
  memset(&time_digitsd_layers, 0, sizeof(time_digitsd_layers));
  memset(&time_digitsd_images, 0, sizeof(time_digitsd_images));
	
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));


  const int inbound_size = 256;
  const int outbound_size = 256;
  app_message_open(inbound_size, outbound_size);  

  window = window_create();
  if (window == NULL) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
	
  window_set_background_color(window, GColorBlack);
		
  Layer *window_layer = window_get_root_layer(window);

  img_battery_100   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_100);
  img_battery_90   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_90);
  img_battery_80   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_80);
  img_battery_70   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_70);
  img_battery_60   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_60);
  img_battery_40   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_40);
  img_battery_20    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_20);
  img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);

#ifdef PBL_PLATFORM_CHALK
  layer_batt_img  = bitmap_layer_create(GRect(83, 6, 14, 168));
#else
  layer_batt_img  = bitmap_layer_create(GRect(65, 0, 14, 168));
#endif  
  bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
	
	
  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEPARATOR);
  GRect bitmap_bounds = gbitmap_get_bounds(separator_image);
#ifdef PBL_PLATFORM_CHALK
  GRect frame = GRect(83, 60, bitmap_bounds.size.w, bitmap_bounds.size.h);
#else
  GRect frame = GRect(65, 53, bitmap_bounds.size.w, bitmap_bounds.size.h);
#endif
  separator_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(separator_layer));   



	// Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
   month_layer = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(month_layer));
	
    time_digitsa_layers = bitmap_layer_create(dummy_frame);
	layer_add_child(window_layer, bitmap_layer_get_layer(time_digitsa_layers));
	
    time_digitsb_layers = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digitsb_layers));
	
    time_digitsc_layers = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digitsc_layers));
	
    time_digitsd_layers = bitmap_layer_create(dummy_frame);
	layer_add_child(window_layer, bitmap_layer_get_layer(time_digitsd_layers));
	
	
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
	
	
  colorpair_a.firstColor = GColorWhite;
  colorpair_a.secondColor = GColorWhite;
	
#ifdef PBL_PLATFORM_CHALK
  effect_layer_col_hr1  = effect_layer_create(GRect(6,0,36,180));
#else
  effect_layer_col_hr1  = effect_layer_create(GRect(2,0,29,168));
#endif	
  effect_layer_add_effect(effect_layer_col_hr1, effect_colorize, &colorpair_a);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_hr1));

	
  colorpair_b.firstColor = GColorWhite;
  colorpair_b.secondColor = GColorWhite;
	
#ifdef PBL_PLATFORM_CHALK
  effect_layer_col_min1  = effect_layer_create(GRect(46,0,36,180));
#else
  effect_layer_col_min1  = effect_layer_create(GRect(34,0,29,168));
#endif	
  effect_layer_add_effect(effect_layer_col_min1, effect_colorize, &colorpair_b);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_min1));
	
	
  colorpair_c.firstColor = GColorWhite;
  colorpair_c.secondColor = GColorWhite;
	
#ifdef PBL_PLATFORM_CHALK
  effect_layer_col_hr2  = effect_layer_create(GRect(98,0,36,180));
#else
  effect_layer_col_hr2  = effect_layer_create(GRect(81,0,29,168));
#endif	
  effect_layer_add_effect(effect_layer_col_hr2, effect_colorize, &colorpair_c);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_hr2));

	
  colorpair_d.firstColor = GColorWhite;
  colorpair_d.secondColor = GColorWhite;
	
#ifdef PBL_PLATFORM_CHALK
  effect_layer_col_min2  = effect_layer_create(GRect(138,0,36,180));
#else
  effect_layer_col_min2  = effect_layer_create(GRect(113,0,29,168));
#endif	
  effect_layer_add_effect(effect_layer_col_min2, effect_colorize, &colorpair_d);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_min2));
	
	
  bluetooth_image_on = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHON);
  GRect bitmap_bounds_bt_on = gbitmap_get_bounds(bluetooth_image_on);		
#ifdef PBL_PLATFORM_CHALK
  GRect frame_bton = GRect(86, 82, bitmap_bounds_bt_on.size.w, bitmap_bounds_bt_on.size.h);
#else
  GRect frame_bton = GRect(67, 74, bitmap_bounds_bt_on.size.w, bitmap_bounds_bt_on.size.h);
#endif	
  bluetooth_layer_on = bitmap_layer_create(frame_bton);
  bitmap_layer_set_bitmap(bluetooth_layer_on, bluetooth_image_on);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer_on));
	

  footprint_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOOTPRINT);
  GRect footprintbounds = gbitmap_get_bounds(footprint_icon);		
#ifdef PBL_PLATFORM_CHALK
  GRect footprintframe = GRect(0, 120, footprintbounds.size.w, footprintbounds.size.h);
#else
  GRect footprintframe = GRect(-12, 118, footprintbounds.size.w, footprintbounds.size.h);
#endif	
  footprint_layer = bitmap_layer_create(footprintframe);
  bitmap_layer_set_bitmap(footprint_layer, footprint_icon);
  layer_add_child(window_layer, bitmap_layer_get_layer(footprint_layer));
	
	
	sq =  fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SQ_14));
	
  steps_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(0, 124, 66, 21),
    GRect(-10, 125, 64, 21)));
  text_layer_set_background_color(steps_label, GColorClear);
  text_layer_set_text_color(steps_label, GColorWhite);
		  text_layer_set_text_alignment(steps_label, GTextAlignmentRight);
  text_layer_set_font(steps_label, sq);
//  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(steps_label));

  dist_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(108, 124, 80, 21),
    GRect(95, 125, 80, 21)));
  text_layer_set_background_color(dist_label, GColorClear);
  text_layer_set_text_color(dist_label, GColorWhite);
	  text_layer_set_text_alignment(dist_label, GTextAlignmentLeft);
  text_layer_set_font(dist_label, sq);
//  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(dist_label));	
	
	
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  handle_battery( battery_state_service_peek() );

  Tuplet initial_values[] = {
    TupletInteger(INVERT_KEY, persist_read_bool(INVERT_KEY)),
    TupletInteger(BLUETOOTHVIBE_KEY, persist_read_bool(BLUETOOTHVIBE_KEY)),
    TupletInteger(HOURLYVIBE_KEY, persist_read_bool(HOURLYVIBE_KEY)),
    TupletInteger(DATESEP_KEY, persist_read_bool(DATESEP_KEY)),
	TupletInteger(HOUR_COL_KEY, persist_read_bool(HOUR_COL_KEY)),
	TupletInteger(MIN_COL_KEY, persist_read_bool(MIN_COL_KEY)),
	TupletInteger(HOUR_COL2_KEY, persist_read_bool(HOUR_COL2_KEY)),
	TupletInteger(MIN_COL2_KEY, persist_read_bool(MIN_COL2_KEY)),
	TupletInteger(HEALTH_KEY, persist_read_bool(HEALTH_KEY)),
  };
  
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, NULL, NULL);
   
  appStarted = true;

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(handle_battery);

	  // subscribe to health events
  if(health_service_events_subscribe(health_handler, NULL)) {
    // force initial steps display
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
	
  window_stack_push(window, true /* Animated */);

}


static void deinit(void) {
  app_sync_deinit(&sync);
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  health_service_events_unsubscribe();
	
  text_layer_destroy(steps_label);
  text_layer_destroy(dist_label);

  layer_remove_from_parent(bitmap_layer_get_layer(footprint_layer));
  bitmap_layer_destroy(footprint_layer);
  gbitmap_destroy(footprint_icon);
	
	
  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);
	
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_charge);	
	
 layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer_on));
 bitmap_layer_destroy(bluetooth_layer_on);
 gbitmap_destroy(bluetooth_image_on);

  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);

  layer_remove_from_parent(bitmap_layer_get_layer(month_layer));
  bitmap_layer_destroy(month_layer);
  gbitmap_destroy(month_image);

  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
  }
	
  layer_remove_from_parent(bitmap_layer_get_layer(time_digitsa_layers));
  bitmap_layer_destroy(time_digitsa_layers);
  gbitmap_destroy(time_digitsa_images);
	
  layer_remove_from_parent(bitmap_layer_get_layer(time_digitsb_layers));
  bitmap_layer_destroy(time_digitsb_layers);
  gbitmap_destroy(time_digitsb_images);
	
  layer_remove_from_parent(bitmap_layer_get_layer(time_digitsc_layers));
  bitmap_layer_destroy(time_digitsc_layers);
  gbitmap_destroy(time_digitsc_images);
	
  layer_remove_from_parent(bitmap_layer_get_layer(time_digitsd_layers));
  bitmap_layer_destroy(time_digitsd_layers);
  gbitmap_destroy(time_digitsd_images);
	
  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_hr1));
  effect_layer_destroy(effect_layer_col_hr1);
  effect_layer_col_hr1 = NULL;
	
  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_min1));
  effect_layer_destroy(effect_layer_col_min1);
  effect_layer_col_min1 = NULL;

  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_hr2));
  effect_layer_destroy(effect_layer_col_hr2);
  effect_layer_col_hr2 = NULL;
	
  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_min2));
  effect_layer_destroy(effect_layer_col_min2);
  effect_layer_col_min2 = NULL;
	
  fonts_unload_custom_font(sq);  
  
	
	window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}