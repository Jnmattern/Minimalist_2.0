#include <pebble.h>

#include "bitmap.h"
#include "Minimalist_2.0.h"

//#define DRAW_SECONDS false
//#define MINUTES_AT_HOUR_HAND false
//#define HOUR_AT_MINUTE_HAND true

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

GBitmap *digitBmp[NUM_IMAGES];
Window *window;
Layer *layer, *rootLayer;
bool clock12;
time_t now;
struct tm last = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int radius = SCREENW/2-1;
static int a1, a2, la1, la2;
static const GPoint center = { CX, CX };

char buffer[256] = "";

int showSeconds = true;
int displayMode = 0;

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
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
	static GRect clipRect = { {0, 0}, {SCREENW, DIGIT_SIZE} };
	int i, a, digit[4], x;

	if (!forceRefresh && !showSeconds && !(units_changed & MINUTE_UNIT)) {
		return;
	}
	
	if (forceRefresh || (now->tm_min != last.tm_min)) {
	    if (displayMode == 2) {
		//#if MINUTES_AT_HOUR_HAND
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
		//#else // MINUTES_AT_HOUR_HAND
		if (clock12) {
			now->tm_hour = now->tm_hour%12;
			if (now->tm_hour == 0) now->tm_hour = 12;
		}

		if (displayMode == 1) {
			//#if HOUR_AT_MINUTE_HAND
			digit[0] = now->tm_hour/10;
			digit[1] = now->tm_hour%10;
		} else {
			//#else // HOUR_AT_MINUTE_HAND
			digit[0] = now->tm_hour/10;
			digit[1] = now->tm_hour%10;
			digit[2] = now->tm_min/10;
			digit[3] = now->tm_min%10;
			//#endif // HOUR_AT_MINUTE_HAND
		}
		
		bmpFill(&bitmap, GColorBlack);
		
		if (now->tm_min < 30) {
			a = 6*(now->tm_min-15);
			if (displayMode == 1) {
				//#if HOUR_AT_MINUTE_HAND
				for (i=0; i<2; i++) {
					if (i != 0 || digit[i] != 0) {
						x = CX + 69 + (i-2)*(DIGIT_SIZE+DIGIT_SPACE);
						bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
					}
				}
				clipRect.origin.x = CX + 69 - 2*(DIGIT_SIZE+DIGIT_SPACE);
			} else {
				//#else // HOUR_AT_MINUTE_HAND
				for (i=0; i<4; i++) {
					if (i != 0 || digit[i] != 0) {
						x = CX-DIGIT_SIZE+(DIGIT_SIZE+DIGIT_SPACE)*i+(DIGIT_SPACE*(i>1));
						bmpSub(digitBmp[digit[i]], &bitmap, digitBmp[digit[i]]->bounds, GPoint(x, 0));
					}
				}
				clipRect.origin.x = CX - DIGIT_SIZE;
				//#endif // HOUR_AT_MINUTE_HAND
			}
		} else {
			a = 6*(now->tm_min-45);
			if (displayMode == 1) {
				//#if HOUR_AT_MINUTE_HAND
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
				//#else // HOUR_AT_MINUTE_HAND
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
				//#endif // HOUR_AT_MINUTE_HAND
			}
		}
		clipRect.size.w = 4*(DIGIT_SIZE + DIGIT_SPACE);
	    }	
	    //#endif // MINUTES_AT_HOUR_HAND
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



void handle_init() {
	int i;
	
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);
	
    app_message_init();
    readConfig();

	clock12 = !clock_is_24h_style();
	
	for (i=0; i<NUM_IMAGES; i++) {
		digitBmp[i] = gbitmap_create_with_resource(digitId[i]);
	}
	
	rootLayer = window_get_root_layer(window);
	layer = layer_create(GRect(0,0,SCREENW,SCREENH));
	layer_set_update_proc(layer, update_display);
	layer_add_child(rootLayer, layer);
	
	now = time(NULL);
	handle_tick(localtime(&now), 0);
	
	// Register for tick updates
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

void handle_deinit() {
	int i;

	tick_timer_service_unsubscribe();

	layer_destroy(layer);

	for (i=0; i<NUM_IMAGES; i++) {
		gbitmap_destroy(digitBmp[i]);
	}
	
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
