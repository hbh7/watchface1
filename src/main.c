#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_timeSeconds_layer, *s_date_layer, *s_date2_layer, *s_textfile_layer, *s_batterytext_layer, *s_bluetoothtext_layer, *s_batterypercent_layer, *s_schedule1_layer, *s_schedule2_layer;
static BitmapLayer *s_background_layer, *s_bt_active_layer, *s_bt_inactive_layer;
static GBitmap *s_background_bitmap, *s_bt_active_bitmap, *s_bt_inactive_bitmap;
static GFont s_hack_35_font, s_hack_20_font, s_hack_25_font, s_hack_15_font, s_hack_10_font, s_hack_18_font, s_hack_12_font;
static Layer *s_battery_layer;
static int s_battery_level;
static char *s_buffer;

// TODO LIST:
// Battery percentage - done
// Block ending time
// Rearrange date and potato to be above block information - done
// Weather

static void bluetooth_callback(bool connected) {
	// Show icon if disconnected
	layer_set_hidden(bitmap_layer_get_layer(s_bt_inactive_layer), connected);

	// layer_set_hidden(bitmap_layer_get_layer(s_bt_active_layer), connected);
	if(!connected) {
		// Issue a vibrating alert. test if i like it!
		//vibes_double_pulse();
		//vibes_double_pulse();
		//layer_set_hidden(bitmap_layer_get_layer(s_bt_active_layer), connected);
	} else {
		//vibes_double_pulse();
		//vibes_double_pulse();
	}
}

static void battery_callback(BatteryChargeState state) {
	// Record the new battery level
	s_battery_level = state.charge_percent;

	// Update meter
	layer_mark_dirty(s_battery_layer);

	// Display Percentage
	static char batbuffer[16];
	snprintf(batbuffer, sizeof(batbuffer), "%d", state.charge_percent);

	static char battery_text[] = "80%";
	snprintf(battery_text, sizeof(battery_text), "%d%%", state.charge_percent);
	text_layer_set_text(s_batterypercent_layer, battery_text); 
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	// Find the width of the bar
	int width = (int)(float)(((float)s_battery_level / 100.0F) * 60.0F);

	// Draw the background
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorLightGray);
	#elif PBL_BW
		graphics_context_set_fill_color(ctx, GColorWhite);
	#endif
	graphics_fill_rect(ctx, bounds, GCornerNone, 0);

	// Draw the bar
	// Separate color code for each platform
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorDarkGray);
	#elif PBL_BW
		graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
	graphics_fill_rect(ctx, GRect(0,0, width, bounds.size.h), GCornerNone, 0);	
}

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);

	// Create a long-lived buffer
	static char buffer[] = "00:00";
	static char bufferSeconds[] = "00";

	// Write the current hours and minutes into the buffer
	if(clock_is_24h_style() == true) {
		// Use 24h format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
		strftime(bufferSeconds, sizeof("00"), "%S", tick_time);
	} else {
		// Use 12h format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
		strftime(bufferSeconds, sizeof("00"), "%S", tick_time);
	}

	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, buffer);
	text_layer_set_text(s_timeSeconds_layer, bufferSeconds);

	// Datestuff
	// Copy date into buffer from tm structure
	static char date_buffer[16];
	static char date2_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%a, %b %d", tick_time);
	strftime(date2_buffer, sizeof(date2_buffer), "%D", tick_time);

	// Show the date
	text_layer_set_text(s_date_layer, date_buffer);
	text_layer_set_text(s_date2_layer, date2_buffer);
}

static void update_block() {
	// Get some indicators for the day and week
	// Time
	time_t temp2 = time(NULL);
	struct tm *tick_time2 = localtime(&temp2);
	static char buffer2[] = "00.00";
	strftime(buffer2, sizeof("00.00"), "%H.%M", tick_time2);
	double timeOfDay = atof(buffer2);
	// Day
	static char date_buffer2[16];
	strftime(date_buffer2, sizeof(date_buffer2), "%w", tick_time2);
	double dayOfWeek = atof(date_buffer2);

	static char MonBlock1Layer1[64] = "B1: Cisco Net";
	static char MonBlock1Layer2[64] = "Ends 9:10,";
	static char MonBlock2Layer1[64] = "B2: AP Biology";
	static char MonBlock2Layer2[64] = "Ends 10:50,";
	static char MonBlock3P1Layer1[64] = "B3: Graphic Des";
	static char MonBlock3P1Layer2[64] = "Ends 11:40,";
	static char MonBlock3P2Layer1[64] = "B3: Graphic Des";
	static char MonBlock3P2Layer2[64] = "Ends 12:50,";
	static char MonBlock4Layer1[64] = "B4: Digital V&M";
	static char MonBlock4Layer2[64] = "Ends 2:14,";
	static char TueBlock5Layer1[64] = "B5: English";
	static char TueBlock5Layer2[64] = "Ends 9:10,";
	static char TueBlock6Layer1[64] = "B6: Civics";
	static char TueBlock6Layer2[64] = "Ends 10:50,";
	static char TueBlock7P1Layer1[64] = "B7: Precalc H";
	static char TueBlock7P1Layer2[64] = "Ends 11:30,";
	static char TueBlock7P2Layer1[64] = "B7: Precalc H";
	static char TueBlock7P2Layer2[64] = "Ends 12:40,";
	static char TueBlock8Layer1[64] = "B8: Psychology";
	static char TueBlock8Layer2[64] = "Ends 1:34,";
	static char TueBlock9Layer1[64] = "B9: Advistory";
	static char TueBlock9Layer2[64] = "Ends 2:14,";
	static char BlockBreakLayer1[64] = "Block Break";
	static char BlockBreakLayer2[64] = "Ends 9:21,";
	static char MonLunchLayer1[64] = "Lunch";
	static char MonLunchLayer2[64] = "Ends 12:10,";
	static char TueLunchLayer1[64] = "Lunch";
	static char TueLunchLayer2[64] = "Ends 12:00,";
	static char FriLunchLayer1[64] = "Lunch";
	static char FriLunchLayer2[64] = "Ends 11:29,";
	static char FriBlock1Layer1[64] = "B1: Cisco Net";
	static char FriBlock1Layer2[64] = "Ends 8:30,";
	static char FriBlock2Layer1[64] = "B2: AP Biology";
	static char FriBlock2Layer2[64] = "Ends 9:19,";
	static char FriBlock3Layer1[64] = "B3: Graphic Des";
	static char FriBlock3Layer2[64] = "Ends 10:08,";
	static char FriBlock4Layer1[64] = "B4: Digital V&M";
	static char FriBlock4Layer2[64] = "Ends 10:57,";
	static char FriBlock5Layer1[64] = "B5: English";
	static char FriBlock5Layer2[64] = "Ends 12:36,";
	static char FriBlock6Layer1[64] = "B6: Civics";
	static char FriBlock6Layer2[64] = "Ends 1:25,";
	static char FriBlock7Layer1[64] = "B7: Precalc H";
	static char FriBlock7Layer2[64] = "Ends 2:14,";

	// Ending block times
	static float Block1End = 9.10;
	static float Block2End = 10.50;
	static float Block3End1 = 11.40;
	static float Block3End2 = 12.50;
	static float Block4End = 2.14;
	static float Block5End = 9.10;
	static float Block6End = 10.50;
	static float Block7End1 = 11.30;
	static float Block7End2 = 12.40;
	static float BlockLunchMonEnd = 12.50;
	static float BlockLunchTueEnd = 12.00;
	static float BlockLunchFriEnd = 11.29;
	static float BlockBreakEnd = 9.21;
	static float TestTimeEnd = 12.29;
	
	if (dayOfWeek == 0 || dayOfWeek == 6) {
			text_layer_set_text(s_schedule1_layer, "School is not");
			text_layer_set_text(s_schedule2_layer, "in session.");
	} else if (dayOfWeek == 1 || dayOfWeek == 3) {
		if (timeOfDay >= 7.35 && timeOfDay <= 9.09) {
			text_layer_set_text(s_schedule1_layer, MonBlock1Layer1);
			text_layer_set_text(s_schedule2_layer, MonBlock1Layer2);
		} else if (timeOfDay >= 9.10 && timeOfDay <= 9.20) {
			text_layer_set_text(s_schedule1_layer, BlockBreakLayer1);
			text_layer_set_text(s_schedule2_layer, BlockBreakLayer2);
		} else if (timeOfDay >= 9.21 && timeOfDay <= 10.49) {
			text_layer_set_text(s_schedule1_layer, MonBlock2Layer1);
			text_layer_set_text(s_schedule2_layer, MonBlock2Layer2);
		} else if (timeOfDay >= 10.50 && timeOfDay <= 11.39) {
			text_layer_set_text(s_schedule1_layer, MonBlock3P1Layer1);
			text_layer_set_text(s_schedule2_layer, MonBlock3P1Layer2);
		} else if (timeOfDay >= 11.40 && timeOfDay <= 12.09) {
			text_layer_set_text(s_schedule1_layer, MonLunchLayer1);
			text_layer_set_text(s_schedule2_layer, MonLunchLayer2);
		} else if (timeOfDay >= 12.10 && timeOfDay <= 12.49) {
			text_layer_set_text(s_schedule1_layer, MonBlock3P2Layer1);
			text_layer_set_text(s_schedule2_layer, MonBlock3P2Layer2);
		} else if (timeOfDay >= 12.50 && timeOfDay <= 14.13) {
			text_layer_set_text(s_schedule1_layer, MonBlock4Layer1);
			text_layer_set_text(s_schedule2_layer, MonBlock4Layer2);
		} else {
			text_layer_set_text(s_schedule1_layer, "School is not");
			text_layer_set_text(s_schedule2_layer, "in session.");
			
			//static char TestTimeEndStr[] = "00000000000";
			//snprintf(TestTimeEndStr, sizeof(TestTimeEndStr), "%lf", TestTimeEnd);
			//static char *newVar = strcat("in session", TestTimeEndStr);

			//text_layer_set_text(s_schedule2_layer, newVar);
		}
	} else if (dayOfWeek == 2 || dayOfWeek == 4) {
		if (timeOfDay >= 7.35 && timeOfDay <= 9.09) {
			text_layer_set_text(s_schedule1_layer, TueBlock5Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock5Layer2);
		} else if (timeOfDay >= 9.10 && timeOfDay <= 9.20) {
			text_layer_set_text(s_schedule1_layer, BlockBreakLayer1);
			text_layer_set_text(s_schedule2_layer, BlockBreakLayer2);
		} else if (timeOfDay >= 9.21 && timeOfDay <= 10.49) {
			text_layer_set_text(s_schedule1_layer, TueBlock6Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock6Layer2);
		} else if (timeOfDay >= 10.50 && timeOfDay <= 11.29) {
			text_layer_set_text(s_schedule1_layer, TueBlock7P1Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock7P1Layer2);
		} else if (timeOfDay >= 11.30 && timeOfDay <= 11.59) {
			text_layer_set_text(s_schedule1_layer, TueLunchLayer1);
			text_layer_set_text(s_schedule2_layer, TueLunchLayer2);
		} else if (timeOfDay >= 12.00 && timeOfDay <= 12.39) {
			text_layer_set_text(s_schedule1_layer, TueBlock7P2Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock7P2Layer2);
		} else if (timeOfDay >= 12.40 && timeOfDay <= 13.33) {
			text_layer_set_text(s_schedule1_layer, TueBlock8Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock8Layer2);
		} else if (timeOfDay >= 13.34 && timeOfDay <= 14.13) {
			text_layer_set_text(s_schedule1_layer, TueBlock9Layer1);
			text_layer_set_text(s_schedule2_layer, TueBlock9Layer2);
		} else {
			text_layer_set_text(s_schedule1_layer, "School is not");
			text_layer_set_text(s_schedule2_layer, "in session.");
		}
	} else if (dayOfWeek == 5) {
		if (timeOfDay >= 7.35 && timeOfDay <= 8.29) {
			text_layer_set_text(s_schedule1_layer, FriBlock1Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock1Layer2);
		} else if (timeOfDay >= 8.31 && timeOfDay <= 9.18) {
			text_layer_set_text(s_schedule1_layer, FriBlock2Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock2Layer2);
		} else if (timeOfDay >= 9.19 && timeOfDay <= 10.07) {
			text_layer_set_text(s_schedule1_layer, FriBlock3Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock3Layer2);
		} else if (timeOfDay >= 10.08 && timeOfDay <= 10.56) {
			text_layer_set_text(s_schedule1_layer, FriBlock4Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock4Layer2);
		} else if (timeOfDay >= 10.57 && timeOfDay <= 11.28) {
			text_layer_set_text(s_schedule1_layer, FriLunchLayer1);
			text_layer_set_text(s_schedule2_layer, FriLunchLayer2);
		} else if (timeOfDay >= 11.29 && timeOfDay <= 12.35) {
			text_layer_set_text(s_schedule1_layer, FriBlock5Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock5Layer2);
		} else if (timeOfDay >= 12.36 && timeOfDay <= 13.24) {
			text_layer_set_text(s_schedule1_layer, FriBlock6Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock6Layer2);
		} else if (timeOfDay >= 13.25 && timeOfDay <= 14.13) {
			text_layer_set_text(s_schedule1_layer, FriBlock7Layer1);
			text_layer_set_text(s_schedule2_layer, FriBlock7Layer2);
		} else {
			text_layer_set_text(s_schedule1_layer, "School is not");
			text_layer_set_text(s_schedule2_layer, "in session.");
		}

	} else {
			text_layer_set_text(s_schedule1_layer, "How did you even");
			text_layer_set_text(s_schedule2_layer, "manage this error?");
}	
}

static void main_window_load(Window *window) {
	// Create GBitmap, then set to created BitmapLayer - Background Layer
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	s_background_layer = bitmap_layer_create(GRect(0,0,144,168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

	// Create the Bluetooth icon GBitmap
	s_bt_active_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ACTIVE_BLACK);
	s_bt_inactive_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_INACTIVE_BLACK);

	// Create the BitmapLayer to display the GBitmap - Bluetooth Status Indicator Layer
	s_bt_active_layer = bitmap_layer_create(GRect(85,22,15,15));
	s_bt_inactive_layer = bitmap_layer_create(GRect(85,22,15,15));
	bitmap_layer_set_bitmap(s_bt_active_layer, s_bt_active_bitmap);
	bitmap_layer_set_bitmap(s_bt_inactive_layer, s_bt_inactive_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_active_layer));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_inactive_layer));

	// Attempt to set transparency on bluetooth indicator layer
	bitmap_layer_set_compositing_mode(s_bt_active_layer, GCompOpClear);
	bitmap_layer_set_compositing_mode(s_bt_inactive_layer, GCompOpClear);

	// Create time TextLayer
	s_time_layer = text_layer_create(GRect(0, 45, 139, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text(s_time_layer, "00:00:00");

	// Create time seconds TextLayer
	s_timeSeconds_layer = text_layer_create(GRect(123, 68, 30, 20));
	text_layer_set_background_color(s_timeSeconds_layer, GColorClear);
	text_layer_set_text_color(s_timeSeconds_layer, GColorBlack);
	text_layer_set_text(s_timeSeconds_layer, "00:00:00");

	// Create date TextLayer 120
	s_date_layer = text_layer_create(GRect(0, 85, 144, 30));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_text(s_date_layer, "DatePlaceholder");

	// Create date2 text TextLayer 140
	s_date2_layer = text_layer_create(GRect(0, 106, 144, 30));
	text_layer_set_background_color(s_date2_layer, GColorClear);
	text_layer_set_text_color(s_date2_layer, GColorBlack);
	text_layer_set_text(s_date2_layer, "DatePlaceHolder");
	
	// Create battery text TextLayer
	s_batterytext_layer = text_layer_create(GRect(1, 1, 144, 30));
	text_layer_set_background_color(s_batterytext_layer, GColorClear);
	text_layer_set_text_color(s_batterytext_layer, GColorBlack);
	text_layer_set_text(s_batterytext_layer, "Battery:");
	
	// Create battery percentage TextLayer
	s_batterypercent_layer = text_layer_create(GRect(100,21,60,30));
	text_layer_set_background_color(s_batterypercent_layer, GColorClear);
	text_layer_set_text_color(s_batterypercent_layer, GColorBlack);
	text_layer_set_text(s_batterypercent_layer, "100%"); 
	
	// Create bluetooth text TextLayer
	s_bluetoothtext_layer = text_layer_create(GRect(0, 21, 144, 30));
	text_layer_set_background_color(s_bluetoothtext_layer, GColorClear);
	text_layer_set_text_color(s_bluetoothtext_layer, GColorBlack);
	text_layer_set_text(s_bluetoothtext_layer, "Status:OK");

	// Create Block Schedule text TextLayers 85 105
	s_schedule1_layer = text_layer_create(GRect(1,130,144,30));
	s_schedule2_layer = text_layer_create(GRect(1,148,144,30));
	text_layer_set_background_color(s_schedule1_layer, GColorClear);
	text_layer_set_background_color(s_schedule2_layer, GColorClear);
	text_layer_set_text_color(s_schedule1_layer, GColorBlack);
	text_layer_set_text_color(s_schedule2_layer, GColorBlack);
	text_layer_set_text(s_schedule1_layer, "Block 1:");	
	text_layer_set_text(s_schedule2_layer, "Ends 9:30,0:05m");	
	
	// Create battery meter layer			115 2
	s_battery_layer = layer_create(GRect(75,9,60,6));
	layer_set_update_proc(s_battery_layer, battery_update_proc);

	// Create GFont
	s_hack_35_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_35));
	s_hack_20_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_20));
	s_hack_25_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_25));
	s_hack_15_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_15));
	s_hack_10_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_10));
	s_hack_18_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_18));
	s_hack_12_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HACK_12));

	// Apply font to TextLayer
	text_layer_set_font(s_time_layer, s_hack_35_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_font(s_timeSeconds_layer, s_hack_12_font);
	text_layer_set_text_alignment(s_timeSeconds_layer, GTextAlignmentLeft);

	// Apply font to date TextLayer
	text_layer_set_font(s_date_layer, s_hack_20_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

	// Apply font to date2 TextLayer
	text_layer_set_font(s_date2_layer, s_hack_20_font);
	text_layer_set_text_alignment(s_date2_layer, GTextAlignmentCenter);
	
	// Apply font to batterytext and batterypercentage TextLayer
	text_layer_set_font(s_batterytext_layer, s_hack_15_font);
	text_layer_set_font(s_batterypercent_layer, s_hack_15_font);
	
	// Apply font to bluetoothtext TextLayer
	text_layer_set_font(s_bluetoothtext_layer, s_hack_15_font);


	text_layer_set_font(s_schedule1_layer, s_hack_15_font);
	text_layer_set_font(s_schedule2_layer, s_hack_15_font);
 
 	// Add it as a child layer to the Window's root layer
 	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
 	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_timeSeconds_layer));
	// Date Layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	// Date2 Layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date2_layer));
	// BatteryText and BatteryPercent Layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batterytext_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batterypercent_layer));
	// BluetoothText Layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetoothtext_layer));
	// Add to Window
	layer_add_child(window_get_root_layer(window), s_battery_layer);




	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_schedule1_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_schedule2_layer));
	//layer_add_child(window_get_root_layer(window), s_schedule_layer1);
	//layer_add_child(window_get_root_layer(window), s_schedule_layer2);





	// Make sure the time is displayed from the start
	update_time();

	// Call update block at start because why not
	update_block();

	// Ensure battery level is displayed from the start
	battery_callback(battery_state_service_peek());

	// Stuff for the data.txt loading
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_textfile_layer = text_layer_create(GRect(0, 40, bounds.size.w, 50));
	text_layer_set_overflow_mode(s_textfile_layer, GTextOverflowModeWordWrap);
	text_layer_set_text_alignment(s_textfile_layer, GTextAlignmentCenter);

	// layer_add_child(window_layer, text_layer_get_layer(s_textfile_layer));
	
	// Show the correct state of the BT connection fron the start
	bluetooth_callback(bluetooth_connection_service_peek());
}

static void main_window_unload(Window *window) {
	// Destroy TextLayer
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_timeSeconds_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_date2_layer);
	text_layer_destroy(s_batterytext_layer);
	text_layer_destroy(s_bluetoothtext_layer);
	text_layer_destroy(s_batterypercent_layer);
	text_layer_destroy(s_schedule1_layer);
	text_layer_destroy(s_schedule2_layer);

	// Unload GFont
	fonts_unload_custom_font(s_hack_35_font);
	fonts_unload_custom_font(s_hack_20_font);
	fonts_unload_custom_font(s_hack_25_font);
	fonts_unload_custom_font(s_hack_15_font);
	fonts_unload_custom_font(s_hack_10_font);
	fonts_unload_custom_font(s_hack_18_font);
	

	// Destroy GBitmap
	gbitmap_destroy(s_background_bitmap);
	
	// Detroy BitmapLayer
	bitmap_layer_destroy(s_background_layer);
	
	// Destroy Battery Layer
	layer_destroy(s_battery_layer);

	// Detroy data.txt layer
	text_layer_destroy(s_textfile_layer);

	// Unload Bluetooth crap
	gbitmap_destroy(s_bt_active_bitmap);
	gbitmap_destroy(s_bt_inactive_bitmap);
	bitmap_layer_destroy(s_bt_active_layer);
	bitmap_layer_destroy(s_bt_inactive_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	update_block();
}
static void load_resource(){
	// Get resource and size
	ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_DATA_TXT);
	size_t res_size = resource_size(handle);

	// Copy to buffer
	s_buffer = (char*)malloc(res_size);
	resource_load(handle, (uint8_t*)s_buffer, res_size);

	//Show in TextLayer
	text_layer_set_text(s_textfile_layer, s_buffer);

}


static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	
	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

	// Register for battery level updates
	battery_state_service_subscribe(battery_callback);

	// Register for Bluetooth connection updates
	bluetooth_connection_service_subscribe(bluetooth_callback);

	load_resource();
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);

	free(s_buffer);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
