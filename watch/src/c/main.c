#include <pebble.h>

#include "holidays.h"
#include "render.h"
#include "sgf_config.h"
#include "timeinfo.h"

static Window *s_window;
static Layer *s_canvas;
static SgfFonts s_fonts;
static SgfModel s_model;

// --- Model refresh ---------------------------------------------------------

static void prv_refresh(struct tm *t) {
  // Zero-padded in both modes. A fixed-width string keeps the centred digits
  // from shifting sideways as the hour rolls over from 9 to 10.
  strftime(s_model.time, sizeof(s_model.time),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", t);

  sgf_format_date(s_model.date, sizeof(s_model.date), t);

  // Cleared first, because sgf_format_holiday() leaves the buffers alone when
  // there is nothing to say -- and an empty `holiday` is what tells the face to
  // draw the rule instead.
  s_model.holiday[0] = '\0';
  s_model.holiday_short[0] = '\0';
  sgf_format_holiday(s_model.holiday, sizeof(s_model.holiday),
                     s_model.holiday_short, sizeof(s_model.holiday_short), t);
}

// --- Service handlers ------------------------------------------------------

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_refresh(tick_time);
  layer_mark_dirty(s_canvas);
}

// --- Window ----------------------------------------------------------------

static void prv_update_proc(Layer *layer, GContext *ctx) {
  sgf_render(ctx, layer_get_bounds(layer), &s_model, &s_fonts);
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);

  // All three faces are system fonts, owned by the firmware. They are taken by
  // key and never unloaded -- fonts_unload_custom_font() is only for fonts the
  // app itself loaded from its own resources, and this one has none.
  s_fonts.time = fonts_get_system_font(SGF_FONT_TIME_KEY);
  s_fonts.date = fonts_get_system_font(SGF_FONT_DATE_KEY);
  s_fonts.holiday = fonts_get_system_font(SGF_FONT_HOLI_KEY);

  s_canvas = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas, prv_update_proc);
  layer_add_child(root, s_canvas);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas);
  s_canvas = NULL;
  // Nothing to unload: all three fonts belong to the firmware.
}

// --- Lifecycle -------------------------------------------------------------

static void prv_init(void) {
  s_window = window_create();
  window_set_background_color(s_window, SGF_COL_FIELD);
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });
  window_stack_push(s_window, true);

  // Populate before the first draw so nothing flashes empty.
  const time_t now = time(NULL);
  prv_refresh(localtime(&now));

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
