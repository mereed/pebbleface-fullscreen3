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

EffectLayer* effect_layer_inv;
EffectLayer* effect_layer_col_hr;
EffectLayer* effect_layer_col_min;

static AppSync sync;
static uint8_t sync_buffer[256];

//static int blink;
static int invert;
static int bluetoothvibe;
static int hourlyvibe;
//static int dayflip;
static int datesep;
static int hour_col;
static int min_col;
//static int font;

static bool appStarted = false;

enum {
//  BLINK_KEY = 0x0,  //not used
  INVERT_KEY = 0x1,
  BLUETOOTHVIBE_KEY = 0x2,
  HOURLYVIBE_KEY = 0x3,
//  DAYFLIP_KEY = 0x4,
  DATESEP_KEY = 0x5,
  HOUR_COL_KEY = 0x6,
  MIN_COL_KEY = 0x7,
//  FONT_KEY = 0x8
};

	// initializing colors
struct EffectColorpair {
  GColor firstColor;  // first color (target for colorize, one of set in colorswap)
  GColor secondColor; // second color (new color for colorize, other of set in colorswap)
};
  
EffectColorpair colorpair_a;
EffectColorpair colorpair_b;

GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_50;
GBitmap *img_battery_40;
GBitmap *img_battery_30;
GBitmap *img_battery_20;
GBitmap *img_battery_10;
GBitmap *img_battery_charge;
BitmapLayer *layer_batt_img;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

static GBitmap *bluetooth_image_on;
static BitmapLayer *bluetooth_layer_on;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
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



void change_hour() {
	  
    switch (hour_col) {
		
		case 0: //white		
			colorpair_a.firstColor = GColorWhite;		
  			colorpair_a.secondColor = GColorWhite;
		break;	
		
		case 1: //green	
  			colorpair_a.firstColor = GColorWhite;
  			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_a.secondColor = GColorGreen;
		#endif	
		break;

		case 2: //orange
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorOrange;
		#endif	
		break;

		case 3: //GColorCyan
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorCyan ;
		#endif	
		break;
		
		case 4: //yellow
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorYellow;
		#endif	
		break;
		
		case 5: //GColorSunsetOrange 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorSunsetOrange;
		#endif	
		break;
		
		case 6: //GColorVividViolet 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorVividViolet ;
		#endif	
		break;
		
		case 7: //GColorShockingPink  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorShockingPink ;
		#endif	
		break;
		
		case 8: //GColorBrightGreen  
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorBrightGreen  ;
		#endif	
		break;

		case 9: //GColorElectricBlue 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorElectricBlue ;
		#endif	
		break;
		
		case 10: //GColorChromeYellow 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorChromeYellow ;
		#endif	
		break;
		
		case 11: //GColorDarkGrey 
			colorpair_a.firstColor = GColorWhite;
			colorpair_a.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_a.secondColor = GColorDarkGray ;
		#endif	
		break; 
	}    
}

void change_min() {

    switch (min_col) {
		case 0: //white
  			colorpair_b.firstColor = GColorWhite;
  			colorpair_b.secondColor = GColorWhite;
		break;
		
		case 1: // green
  			colorpair_b.firstColor = GColorWhite;
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorGreen;
		#endif	
		break;

		case 2: //orange
  			colorpair_b.firstColor = GColorWhite;	
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorOrange;
		#endif	
		break;

		case 3: //GColorCyan 
  			colorpair_b.firstColor = GColorWhite;	
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorCyan ;
		#endif	
		break;	
		
		case 4: //GColorYellow  
  			colorpair_b.firstColor = GColorWhite;	
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorYellow  ;
		#endif	
		break;
		
		case 5: //GColorSunsetOrange  
  			colorpair_b.firstColor = GColorWhite;	
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorSunsetOrange  ;
		#endif	
		break;

		case 6: //GColorVividViolet  
  			colorpair_b.firstColor = GColorWhite;	
  			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
  			colorpair_b.secondColor = GColorVividViolet  ;
		#endif	
		break;

		case 7: //GColorShockingPink   
			colorpair_b.firstColor = GColorWhite;	
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorShockingPink   ;
		#endif	
		break;
		
		case 8: //GColorBrightGreen    
			colorpair_b.firstColor = GColorWhite;	
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorBrightGreen    ;
		#endif	
		break;
		
		case 9: //GColorElectricBlue    
			colorpair_b.firstColor = GColorWhite;	
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorElectricBlue    ;
		#endif	
		break;
		
		case 10: //GColorChromeYellow   
			colorpair_b.firstColor = GColorWhite;	
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorChromeYellow   ;
		#endif	
		break;
		
		case 11: //GColorDarkGrey   
			colorpair_b.firstColor = GColorWhite;	
			colorpair_b.secondColor = GColorWhite;
		#ifdef PBL_COLOR		
			colorpair_b.secondColor = GColorDarkGray   ;
		#endif	
		break;
    }    
}

void change_background(bool invert) {
  if (invert && effect_layer_inv == NULL) {
    // Add inverter layer
    Layer *window_layer = window_get_root_layer(window);
#ifdef PBL_PLATFORM_CHALK
    effect_layer_inv = effect_layer_create(GRect(0, 0, 180, 180));
#else
    effect_layer_inv = effect_layer_create(GRect(0, 0, 144, 168));
#endif
    layer_add_child(window_layer, effect_layer_get_layer(effect_layer_inv));
    effect_layer_add_effect(effect_layer_inv, effect_invert, NULL);
	  
  } else if (!invert && effect_layer_inv != NULL) {
    // Remove Inverter layer
   layer_remove_from_parent(effect_layer_get_layer(effect_layer_inv));
   effect_layer_destroy(effect_layer_inv);
   effect_layer_inv = NULL;
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
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_mon], GPoint(86, 102));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(86, 60));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(86, 70));
#else
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(68, 95));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(68, 52));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(68, 62));	
#endif

//update hours
	
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);
	 
#ifdef PBL_PLATFORM_CHALK
  set_container_image(&time_digitsa_images, time_digitsa_layers, BIG_DIGIT_A_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(6, 5));
  set_container_image(&time_digitsb_images, time_digitsb_layers, BIG_DIGIT_B_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(46, 5));	
#else
  set_container_image(&time_digitsa_images, time_digitsa_layers, BIG_DIGIT_A_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(1, 3));
  set_container_image(&time_digitsb_images, time_digitsb_layers, BIG_DIGIT_B_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(33, 3));
#endif
	
// hours animation

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame1 = GRect(46, 358, 36, 169);
  GRect to_frame1 = GRect(46, 5, 36, 169);
#else
  GRect from_frame1 = GRect(1, 358, 29, 160);
  GRect to_frame1 = GRect(1, 3, 29, 160);
#endif
  // Schedule the next animation
  animation1 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsa_layers), &from_frame1, &to_frame1);
  animation_set_duration((Animation*)animation1, 1500);
  animation_set_delay((Animation*)animation1, 50);
  animation_set_curve((Animation*)animation1, AnimationCurveLinear);
  animation_schedule((Animation*)animation1);
	

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame2 = GRect(46, 358, 36, 169);
  GRect to_frame2 = GRect(46, 5, 36, 169);
#else
  GRect from_frame2 = GRect(33, 358, 29, 160);
  GRect to_frame2 = GRect(33, 3, 29, 160);
#endif
  // Schedule the next animation
  animation2 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsb_layers), &from_frame2, &to_frame2);
  animation_set_duration((Animation*)animation2, 1500);
  animation_set_delay((Animation*)animation2, 100);
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
  set_container_image(&time_digitsc_images, time_digitsc_layers, BIG_DIGIT_C_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(98, 5));
  set_container_image(&time_digitsd_images, time_digitsd_layers, BIG_DIGIT_D_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(138, 5));		
#else
  set_container_image(&time_digitsc_images, time_digitsc_layers, BIG_DIGIT_C_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(82, 3));
  set_container_image(&time_digitsd_images, time_digitsd_layers, BIG_DIGIT_D_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(114,3));		
#endif
	  
// minutes animation

#ifdef PBL_PLATFORM_CHALK
  GRect from_frame3 = GRect(98, 358, 36, 169);
  GRect to_frame3 = GRect(98, 5, 36, 169);
#else
  GRect from_frame3 = GRect(82, 358, 29, 160);
  GRect to_frame3 = GRect(82, 3, 29, 160);
#endif
  // Schedule the next animation
  animation3 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsc_layers), &from_frame3, &to_frame3);
  animation_set_duration((Animation*)animation3, 1500);
  animation_set_delay((Animation*)animation3, 200);
  animation_set_curve((Animation*)animation3, AnimationCurveLinear);
  animation_schedule((Animation*)animation3);
	
#ifdef PBL_PLATFORM_CHALK
  GRect from_frame4 = GRect(138, 358, 36, 169);
  GRect to_frame4 = GRect(138, 5, 36, 169);
#else
  GRect from_frame4 = GRect(114, 358, 29, 160);
  GRect to_frame4 = GRect(114, 3, 29, 160);
#endif
  // Schedule the next animation
  animation4 = property_animation_create_layer_frame(bitmap_layer_get_layer(time_digitsd_layers), &from_frame4, &to_frame4);
  animation_set_duration((Animation*)animation4, 1500);
  animation_set_delay((Animation*)animation4, 500);
  animation_set_curve((Animation*)animation4, AnimationCurveLinear);
  animation_schedule((Animation*)animation4);
	
	
  }
}


static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
 //   case BLINK_KEY:
//	  blink = new_tuple->value->uint8 !=0;
//	  persist_write_bool(INVERT_KEY, blink);
//      break;
	  
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
		
		layer_set_hidden(bitmap_layer_get_layer(day_name_layer), true);
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
		layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), true);
		}
				
	} else {
		layer_set_hidden(bitmap_layer_get_layer(separator_layer), true);

		layer_set_hidden(bitmap_layer_get_layer(day_name_layer), false);
		for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
		layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), false);
		}
	}
      break;
	  
//	case DAYFLIP_KEY:
//      dayflip = new_tuple->value->uint8 != 0;
//	  persist_write_bool(DAYFLIP_KEY, dayflip);	  
     // break;

    case HOUR_COL_KEY:
		hour_col = new_tuple->value->uint8;
		persist_write_bool(HOUR_COL_KEY, hour_col);
	    change_hour();

	break;
	  
    case MIN_COL_KEY:
		min_col = new_tuple->value->uint8;
		persist_write_bool(MIN_COL_KEY, min_col);
		change_min();

	break;
	  
//	case FONT_KEY:
//		font = new_tuple->value->uint8 !=0;
//		persist_write_bool(FONT_KEY, font);
//	break;
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
  layer_batt_img  = bitmap_layer_create(GRect(86, 6, 8, 168));
#else
  layer_batt_img  = bitmap_layer_create(GRect(68, 0, 8, 168));
#endif  
	bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
	
	
	
  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEPARATOR);
  GRect bitmap_bounds = gbitmap_get_bounds(separator_image);
#ifdef PBL_PLATFORM_CHALK
  GRect frame = GRect(83, 60, bitmap_bounds.size.w, bitmap_bounds.size.h);
#else
  GRect frame = GRect(66, 10, bitmap_bounds.size.w, bitmap_bounds.size.h);
#endif
  separator_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(separator_layer));   



	// Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
   day_name_layer = bitmap_layer_create(dummy_frame);
   layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
	
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
  effect_layer_col_hr  = effect_layer_create(GRect(0,0,87,180));
#else
  effect_layer_col_hr  = effect_layer_create(GRect(0,0,62,168));
#endif	
  effect_layer_add_effect(effect_layer_col_hr, effect_colorize, &colorpair_a);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_hr));

  colorpair_b.firstColor = GColorWhite;
  colorpair_b.secondColor = GColorWhite;
	
#ifdef PBL_PLATFORM_CHALK
  effect_layer_col_min  = effect_layer_create(GRect(98,0,82,180));
#else
  effect_layer_col_min  = effect_layer_create(GRect(78,0,70,168));
#endif	
  effect_layer_add_effect(effect_layer_col_min, effect_colorize, &colorpair_b);
  layer_add_child(window_layer, effect_layer_get_layer(effect_layer_col_min));
	
	
	
  bluetooth_image_on = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHON);
  GRect bitmap_bounds_bt_on = gbitmap_get_bounds(bluetooth_image_on);		
#ifdef PBL_PLATFORM_CHALK
  GRect frame_bton = GRect(86, 82, bitmap_bounds_bt_on.size.w, bitmap_bounds_bt_on.size.h);
#else
  GRect frame_bton = GRect(68, 74, bitmap_bounds_bt_on.size.w, bitmap_bounds_bt_on.size.h);
#endif	
  bluetooth_layer_on = bitmap_layer_create(frame_bton);
  bitmap_layer_set_bitmap(bluetooth_layer_on, bluetooth_image_on);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer_on));
	
	
	
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  handle_battery( battery_state_service_peek() );

  Tuplet initial_values[] = {
 //   TupletInteger(BLINK_KEY, persist_read_bool(BLINK_KEY)),
    TupletInteger(INVERT_KEY, persist_read_bool(INVERT_KEY)),
    TupletInteger(BLUETOOTHVIBE_KEY, persist_read_bool(BLUETOOTHVIBE_KEY)),
    TupletInteger(HOURLYVIBE_KEY, persist_read_bool(HOURLYVIBE_KEY)),
//    TupletInteger(DAYFLIP_KEY, persist_read_bool(DAYFLIP_KEY)),
    TupletInteger(DATESEP_KEY, persist_read_bool(DATESEP_KEY)),
	TupletInteger(HOUR_COL_KEY, persist_read_bool(HOUR_COL_KEY)),
	TupletInteger(MIN_COL_KEY, persist_read_bool(MIN_COL_KEY)),
//	TupletInteger(FONT_KEY, persist_read_bool(FONT_KEY)),
  };
  
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, NULL, NULL);
   
  appStarted = true;
/*  
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );
*/
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(handle_battery);

  window_stack_push(window, true /* Animated */);

}


static void deinit(void) {
  app_sync_deinit(&sync);
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
	
  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);
	
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_50);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_30);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_10);
  gbitmap_destroy(img_battery_charge);	
	
 layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer_on));
 bitmap_layer_destroy(bluetooth_layer_on);
 gbitmap_destroy(bluetooth_image_on);


  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);


  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
	


	 for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
  }
	
 
  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_hr));
  effect_layer_destroy(effect_layer_col_hr);
  effect_layer_col_hr = NULL;
	
  layer_remove_from_parent(effect_layer_get_layer(effect_layer_col_min));
  effect_layer_destroy(effect_layer_col_min);
  effect_layer_col_min = NULL;

	
	  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}