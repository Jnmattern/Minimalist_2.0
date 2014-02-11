#include <pebble.h>

#include "bitmap.h"
#include "Minimalist_2.0.h"

#define DISPLAYMODE_HOUR_AND_MINUTES 0
#define DISPLAYMODE_HOUR_AT_MINUTE_HAND 1
#define DISPLAYMODE_MINUTES_AT_HOUR_HAND 2

#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84
#define DIGIT_SIZE 20
#define DIGIT_SPACE 2

enum {
	CONFIG_KEY_SECONDS     = 40,
	CONFIG_KEY_DISPLAYMODE = 41
};

#define NUM_IMAGES 10
const int digitId[NUM_IMAGES] = {
	RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2, RESOURCE_ID_IMAGE_3,
	RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5, RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7,
	RESOURCE_ID_IMAGE_8, RESOURCE_ID_IMAGE_9
};

const int miniDigitId[NUM_IMAGES] = {
	RESOURCE_ID_IMAGE_M0, RESOURCE_ID_IMAGE_M1, RESOURCE_ID_IMAGE_M2, RESOURCE_ID_IMAGE_M3,
	RESOURCE_ID_IMAGE_M4, RESOURCE_ID_IMAGE_M5, RESOURCE_ID_IMAGE_M6, RESOURCE_ID_IMAGE_M7,
	RESOURCE_ID_IMAGE_M8, RESOURCE_ID_IMAGE_M9
};

GBitmap *digitBmp[NUM_IMAGES];
GBitmap *miniDigitBmp[NUM_IMAGES];
GBitmap *miniPercentBmp, *miniPointBmp, *btOkBmp, *btFailedBmp;

Window *window;
Layer *layer, *rootLayer;
time_t now;
struct tm last = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int radius = SCREENW/2-1;
static int a1, a2, la1, la2;
static const GPoint center = { CX, CX };

char buffer[256] = "";

int showSeconds = true;
int displayMode = 0;

bool showInfo = false;
bool btConnected = false;
bool forceRefresh = true;

static inline void drawSec(GBitmap *bmp, GPoint center, int a1, int a2, GColor c) {
	if (a2 <= 360) {
		bmpDrawArc(bmp, center, radius, 2, a1, a2, c);
	} else {
		bmpDrawArc(bmp, center, radius, 2, a1, 360, c);
		bmpDrawArc(bmp, center, radius, 2, 0, a2-360, c);
	}
}

void update_display(Layer *layer, GContext *ctx) {
	graphics_draw_bitmap_in_rect(ctx, &bitmap2, GRect(0,12,SCREENW,SCREENW));
	
	if (showInfo) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "update_display() -> showInfo");	

		GRect r = { { 0, 0 }, { 10, 10 } };
		GRect btRect;
		GBitmap *btBmp;
		BatteryChargeState chargeState = battery_state_service_peek();
		char batteryLevel[] = "100%";
		int len = snprintf(batteryLevel, sizeof(batteryLevel), "%d%%", chargeState.charge_percent);
		
		for (int i=0; i<len; i++) {
			if (batteryLevel[i] != '%') {
				graphics_draw_bitmap_in_rect(ctx, miniDigitBmp[batteryLevel[i] - '0'], r);
			} else {
				graphics_draw_bitmap_in_rect(ctx, miniPercentBmp, r);
			}
			
			r.origin.x += 11;
		}
		
		if (btConnected) {
			btBmp = btOkBmp;
		} else {
			btBmp = btFailedBmp;
		}
		btRect.size = btBmp->bounds.size;
		btRect.origin.x = 144-btRect.size.w;
		btRect.origin.y = 0;
		graphics_draw_bitmap_in_rect(ctx, btBmp, btRect);
		
		char date[] = "dd mm yyyy";
		len = snprintf(date, sizeof(date), "%.2d %.2d %d", last.tm_mday, last.tm_mon+1, last.tm_year+1900);
		r.origin.x = 72 - len*11/2;
		r.origin.y = 158;
		for (int i=0; i<len; i++) {
			if (date[i] != ' ') {
				graphics_draw_bitmap_in_rect(ctx, miniDigitBmp[date[i] - '0'], r);
			} else {
				graphics_draw_bitmap_in_rect(ctx, miniPointBmp, r);
			}
			r.origin.x += 11;
		}		
	}
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
	static GRect clipRect = { {0, 0}, {SCREENW, DIGIT_SIZE} };
	int i, a, digit[4], x;
	
	if (!forceRefresh && !showSeconds && !(units_changed & MINUTE_UNIT)) {
		return;
	}
	
	if (forceRefresh || (now->tm_min != last.tm_min)) {
	    if (displayMode == DISPLAYMODE_MINUTES_AT_HOUR_HAND) {
			now->tm_hour = now->tm_hour%12;
			
			digit[0] = now->tm_min/10;
			digit[1] = now->tm_min%10;
			
			bmpFill(&bitmap, GColorBlack);
			
			if (now->tm_hour < 6) {
				a = 30*(now->tm_hour-3) + now->tm_min/2;
				for (i=0;i<2;i++) {
					x = CX + 69 + (i-2)*(DIGIT_SIZE+DIGIT_SPACE);
					bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
				}
				clipRect.origin.x = CX + 69 - 2*(DIGIT_SIZE+DIGIT_SPACE);
			} else {
				a = 30*(now->tm_hour-9) + now->tm_min/2;
				for (i=0;i<2;i++) {
					x = CX - 69 + DIGIT_SPACE + i*(DIGIT_SIZE+DIGIT_SPACE);
					bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
				}
				clipRect.origin.x = CX - 69 + DIGIT_SPACE;
			}
			clipRect.size.w = 2*DIGIT_SIZE + DIGIT_SPACE;
	    } else {
			if (!clock_is_24h_style()) {
				now->tm_hour = now->tm_hour%12;
				if (now->tm_hour == 0) now->tm_hour = 12;
			}
			
			if (displayMode == DISPLAYMODE_HOUR_AT_MINUTE_HAND) {
				digit[0] = now->tm_hour/10;
				digit[1] = now->tm_hour%10;
			} else {
				digit[0] = now->tm_hour/10;
				digit[1] = now->tm_hour%10;
				digit[2] = now->tm_min/10;
				digit[3] = now->tm_min%10;
			}
			
			bmpFill(&bitmap, GColorBlack);
			
			if (now->tm_min < 30) {
				a = 6*(now->tm_min-15);
				if (displayMode == DISPLAYMODE_HOUR_AT_MINUTE_HAND) {
					for (i=0; i<2; i++) {
						if (i != 0 || digit[i] != 0) {
							x = CX + 69 + (i-2)*(DIGIT_SIZE+DIGIT_SPACE);
							bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
						}
					}
					clipRect.origin.x = CX + 69 - 2*(DIGIT_SIZE+DIGIT_SPACE);
				} else {
					for (i=0; i<4; i++) {
						if (i != 0 || digit[i] != 0) {
							x = CX-DIGIT_SIZE+(DIGIT_SIZE+DIGIT_SPACE)*i+(DIGIT_SPACE*(i>1));
							bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
						}
					}
					clipRect.origin.x = CX - DIGIT_SIZE;
				}
			} else {
				a = 6*(now->tm_min-45);
				if (displayMode == DISPLAYMODE_HOUR_AT_MINUTE_HAND) {
					for (i=0; i<2; i++) {
						if (i != 0 || digit[i] != 0) {
							if (digit[0] == 0) {
								x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(5-i)-(DIGIT_SPACE*(i<2));
							} else {
								x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(4-i)-(DIGIT_SPACE*(i<2));
							}
							bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
						}
					}
					clipRect.origin.x = CX - 4*DIGIT_SPACE - 3*DIGIT_SIZE + 1;
				} else {
					for (i=0; i<4; i++) {
						if (i != 0 || digit[i] != 0) {
							if (digit[0] == 0) {
								x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(5-i)-(DIGIT_SPACE*(i<2));
							} else {
								x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(4-i)-(DIGIT_SPACE*(i<2));
							}
							bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
						}
					}
					clipRect.origin.x = CX - 4*DIGIT_SPACE - 3*DIGIT_SIZE + 1;
				}
			}
			clipRect.size.w = 4*(DIGIT_SIZE + DIGIT_SPACE);
	    }

		bmpFill(&bitmap2, GColorBlack);		
		bmpRotate(&bitmap, &bitmap2, a, &clipRect, grect_center_point(&bitmap.bounds), GPoint(0,CX-bitmap.bounds.size.h/2));
		bmpDrawArc(&bitmap2, center, radius, 2, 0, 360, GColorWhite);

		if (showSeconds && (last.tm_hour != -1)) {
			drawSec(&bitmap2, center, 267, 273, GColorBlack);
		}
	} else {
		if (forceRefresh) {
			bmpDrawArc(&bitmap2, center, radius, 2, 0, 360, GColorWhite);
		}
		la1 = (267+6*last.tm_sec)%360;
		la2 = la1+6;
		a1 = (267+6*now->tm_sec)%360;
		a2 = a1+6;
		drawSec(&bitmap2, center, la1, la2, GColorWhite);
		drawSec(&bitmap2, center, a1, a2, GColorBlack);
	}
	
	layer_mark_dirty(layer);
	forceRefresh = false;
	last = *now;
}

void logVariables(const char *msg) {
	snprintf(buffer, 256, "MSG: %s\n\tshowSeconds=%d\n\tdisplayMode=%d\n", msg, showSeconds, displayMode);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, buffer);
}

void applyConfig() {
	forceRefresh = true;
	last.tm_hour = last.tm_min = last.tm_sec = -1;
	
	tick_timer_service_unsubscribe();
	if (showSeconds) {
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	} else {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	}

	layer_mark_dirty(rootLayer);
}

bool checkAndSaveInt(int *var, int val, int key) {
	if (*var != val) {
		*var = val;
		persist_write_int(key, val);
		return true;
	} else {
		return false;
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) {
}

void in_received_handler(DictionaryIterator *received, void *context) {
	bool somethingChanged = false;
	
	Tuple *seconds = dict_find(received, CONFIG_KEY_SECONDS);
	Tuple *displaymode = dict_find(received, CONFIG_KEY_DISPLAYMODE);
	
	if (seconds && displaymode) {
		somethingChanged |= checkAndSaveInt(&showSeconds, seconds->value->int32, CONFIG_KEY_SECONDS);
		somethingChanged |= checkAndSaveInt(&displayMode, displaymode->value->int32, CONFIG_KEY_DISPLAYMODE);
		
		logVariables("ReceiveHandler");
		
		if (somethingChanged) {
			applyConfig();
		}
	}
}

void readConfig() {
	if (persist_exists(CONFIG_KEY_SECONDS)) {
		showSeconds = persist_read_int(CONFIG_KEY_SECONDS);
	} else {
		showSeconds = true;
	}
	
	if (persist_exists(CONFIG_KEY_DISPLAYMODE)) {
		displayMode = persist_read_int(CONFIG_KEY_DISPLAYMODE);
	} else {
		displayMode = 1;
	}
	
	logVariables("readConfig");
}

static void app_message_init(void) {
        app_message_register_inbox_received(in_received_handler);
        app_message_register_inbox_dropped(in_dropped_handler);
        app_message_open(64, 64);
}

void hideBattery(void *data) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "hideBattery()");
	
	showInfo = false;
	layer_mark_dirty(layer);
}

void handle_tap(AccelAxisType axis, int32_t direction) {
	if (showInfo) return;

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_tap()");
	
	showInfo = true;
	layer_mark_dirty(layer);

	app_timer_register(4000, hideBattery, NULL);
}

void bt_handler(bool connected) {
	btConnected = connected;
	if (!connected) {
		vibes_double_pulse();
	}
}

void handle_init() {
	int i;
	
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);
	
    app_message_init();
    readConfig();
	
	for (i=0; i<NUM_IMAGES; i++) {
		digitBmp[i] = gbitmap_create_with_resource(digitId[i]);
		miniDigitBmp[i] = gbitmap_create_with_resource(miniDigitId[i]);
	}
	miniPercentBmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MPERCENT);
	miniPointBmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MPOINT);
	btOkBmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BTOK);
	btFailedBmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BTFAILED);
	
	rootLayer = window_get_root_layer(window);
	layer = layer_create(GRect(0,0,SCREENW,SCREENH));
	layer_set_update_proc(layer, update_display);
	layer_add_child(rootLayer, layer);
	
	now = time(NULL);
	handle_tick(localtime(&now), 0);
	
	if (showSeconds) {
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	} else {
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	}
	
	accel_tap_service_subscribe(handle_tap);
	
	bluetooth_connection_service_subscribe(bt_handler);
	btConnected = bluetooth_connection_service_peek();
}

void handle_deinit() {
	int i;

	bluetooth_connection_service_unsubscribe();
	accel_tap_service_unsubscribe();
	tick_timer_service_unsubscribe();

	layer_destroy(layer);

	for (i=0; i<NUM_IMAGES; i++) {
		gbitmap_destroy(digitBmp[i]);
		gbitmap_destroy(miniDigitBmp[i]);
	}
	gbitmap_destroy(miniPercentBmp);
	gbitmap_destroy(miniPointBmp);
	gbitmap_destroy(btOkBmp);
	gbitmap_destroy(btFailedBmp);
	
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
