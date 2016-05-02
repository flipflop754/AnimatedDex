#include <pebble.h>

static Window *s_window;
static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static int s_battery_level;
static Layer *s_battery_level_layer;
static GBitmap *s_battery_bitmap;
static BitmapLayer *s_battery_image_layer;
//*****************************************************************************//
//                Change these according to your preferences                   //
// Choose 1, 2, 3, 5, 10, or 30 to update every respective number of  minute(s)
static int refresh_time = 10;
//Change names of resource to the names that you created. KEEP THIS TO ONLY 4 TOTAL APNGS
static uint32_t pokemon_list[4] = {RESOURCE_ID_LUXRAY, RESOURCE_ID_LANTURN, RESOURCE_ID_AZUMARILL, RESOURCE_ID_M_ABSOL};
//***********************************************************//
static bool animate = true;

static void update_time() {
  // Get a tm structure for time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date() {
  // Get a tm structure for time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[20];
  // Change date format depending on configuration
  strftime(s_buffer, sizeof(s_buffer), "%a, %b %d", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_buffer);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_level_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int height = (int)(float)(22.0-(((float)s_battery_level / 100.0F) * 22.0F));

  // Draw the background
  graphics_context_set_fill_color(ctx, PBL_IF_ROUND_ELSE(GColorBlack,GColorRed));
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.origin.y+height, bounds.origin.x+bounds.size.w, bounds.origin.y+bounds.size.h), 0, GCornerNone);
}

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame, and get the delay for this frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    // Set the new frame into the BitmapLayer
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
    if(animate)
    {
      app_timer_register(next_delay, timer_handler, NULL);
    }
  } else {
      // Start again
      gbitmap_sequence_restart(s_sequence);
  }
}

static void stop_animation(){
  animate = false;
}

static void handle_focus(bool in_focus){
  // Animate for 10 seconds on face focus
  animate = true;
  app_timer_register(1, timer_handler, NULL);
  app_timer_register(10000, stop_animation, NULL);
}

static void handle_tap(AccelAxisType axis, int32_t direction){
  //Animate for 10 seconds on tap
  animate = true;
  app_timer_register(1, timer_handler, NULL);
  app_timer_register(10000, stop_animation, NULL);
}

static void update_sequence(){
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  int random_num = rand() % 4;
  s_sequence = gbitmap_sequence_create_with_resource(pokemon_list[random_num]);
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
  uint32_t first_delay_ms = 5;
  app_timer_register(first_delay_ms, timer_handler, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  
  if(tick_time->tm_min % refresh_time == 0){
    APP_LOG(APP_LOG_LEVEL_INFO, "Used update sequence");
    update_sequence();
  }
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_copy = bounds;
  bounds_copy.origin.y = -15;
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(PBL_IF_ROUND_ELSE(RESOURCE_ID_BACKGROUND_ROUND, RESOURCE_ID_BACKGROUND));

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_alignment(s_background_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Create battery meter Layer
  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
  s_battery_image_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(160,127), PBL_IF_ROUND_ELSE(76,2), 15, 30));
  bitmap_layer_set_bitmap(s_battery_image_layer, s_battery_bitmap);
  bitmap_layer_set_compositing_mode(s_battery_image_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_image_layer));
  
  // Create battery meter Layer
  s_battery_level_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(164,131), PBL_IF_ROUND_ELSE(80,6), 7, 22));
  layer_set_update_proc(s_battery_level_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_level_layer);
  
  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON);

  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(GRect(2, PBL_IF_ROUND_ELSE(76,2), 30, 30));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  // Create sequence
  int random_num = rand() % 4;
  s_sequence = gbitmap_sequence_create_with_resource(pokemon_list[random_num]);
  //s_sequence = gbitmap_sequence_create_with_resource(my_pokemon);

  // Create blank GBitmap using APNG frame size
  GSize frame_size = gbitmap_sequence_get_bitmap_size(s_sequence);
  s_bitmap = gbitmap_create_blank(frame_size, GBitmapFormat8Bit);
  s_bitmap_layer = bitmap_layer_create(bounds_copy);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  layer_add_child(window_get_root_layer(s_window), bitmap_layer_get_layer(s_bitmap_layer));
  uint32_t first_delay_ms = 5;
  app_timer_register(first_delay_ms, timer_handler, NULL);
  
  // Create the time Layer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(128, 125), bounds.size.w, 50));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKEMON_35));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the date Layer with specific bounds
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(120, 119), bounds.size.w, 50));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKEMON_16));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void init(void) {
	// Create a window and get information about the window
	s_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = main_window_load
  });

	// Push the window, setting the window animation to 'true'
	window_stack_push(s_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  update_time();
  update_date();
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Animate on app focus and on tap (wrist shake)
  app_focus_service_subscribe(handle_focus);
  app_timer_register(10000, stop_animation, NULL);
  accel_tap_service_subscribe(handle_tap);
}

static void deinit(void) {	
  gbitmap_destroy(s_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_sequence_destroy(s_sequence);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
  gbitmap_destroy(s_battery_bitmap);
  bitmap_layer_destroy(s_battery_image_layer);
  layer_destroy(s_battery_level_layer);
  
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  
	// Destroy the window
	window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
