#include "settings.h"

enum {
  PERSIST_TIME_FORMAT = 1,
  PERSIST_LEADING_ZERO = 2,
  PERSIST_SHOW_DATE = 3,
  PERSIST_DATE_FORMAT = 4,
  PERSIST_SHOW_CAPTION = 5,
  PERSIST_PATTERN_MODE = 6,
  PERSIST_BG_COLOR = 7,
  PERSIST_WALL_COLOR = 8,
  PERSIST_CAPTION_COLOR = 9,
  PERSIST_FILL_MODE = 10,
  PERSIST_FILL_COLOR = 11,
};

static FaceSettings s_settings;
static bool s_inbox_open;

static int clamp_int(int v, int lo, int hi) {
  if (v < lo) {
    return lo;
  }
  if (v > hi) {
    return hi;
  }
  return v;
}

static void settings_set_defaults(void) {
  s_settings.time_format = TIME_FMT_SYSTEM;
  s_settings.leading_zero = true;
  s_settings.show_date = false;
  s_settings.date_format = DATE_FMT_DOW_DAY_MON;
  s_settings.show_caption = true;
  s_settings.pattern_mode = PATTERN_AUTO;
  s_settings.bg_hex = 0x000000;
  s_settings.wall_hex = 0xFFFFFF;
  s_settings.caption_hex = 0xAAAAAA;
  s_settings.fill_mode = FILL_COLOR_AUTO;
  s_settings.fill_hex = 0xAA00FF;
}

static void settings_load(void) {
  settings_set_defaults();

  if (persist_exists(PERSIST_TIME_FORMAT)) {
    s_settings.time_format = (TimeFormat)clamp_int(
        persist_read_int(PERSIST_TIME_FORMAT), TIME_FMT_SYSTEM, TIME_FMT_24H);
  }
  if (persist_exists(PERSIST_LEADING_ZERO)) {
    s_settings.leading_zero = persist_read_bool(PERSIST_LEADING_ZERO);
  }
  if (persist_exists(PERSIST_SHOW_DATE)) {
    s_settings.show_date = persist_read_bool(PERSIST_SHOW_DATE);
  }
  if (persist_exists(PERSIST_DATE_FORMAT)) {
    s_settings.date_format = (DateFormat)clamp_int(
        persist_read_int(PERSIST_DATE_FORMAT), DATE_FMT_DOW_DAY_MON, DATE_FMT_DM);
  }
  if (persist_exists(PERSIST_SHOW_CAPTION)) {
    s_settings.show_caption = persist_read_bool(PERSIST_SHOW_CAPTION);
  }
  if (persist_exists(PERSIST_PATTERN_MODE)) {
    s_settings.pattern_mode = (PatternMode)clamp_int(
        persist_read_int(PERSIST_PATTERN_MODE), PATTERN_AUTO, PATTERN_TRIANGLES);
  }
  if (persist_exists(PERSIST_BG_COLOR)) {
    s_settings.bg_hex = persist_read_int(PERSIST_BG_COLOR);
  }
  if (persist_exists(PERSIST_WALL_COLOR)) {
    s_settings.wall_hex = persist_read_int(PERSIST_WALL_COLOR);
  }
  if (persist_exists(PERSIST_CAPTION_COLOR)) {
    s_settings.caption_hex = persist_read_int(PERSIST_CAPTION_COLOR);
  }
  if (persist_exists(PERSIST_FILL_MODE)) {
    s_settings.fill_mode = (FillColorMode)clamp_int(
        persist_read_int(PERSIST_FILL_MODE), FILL_COLOR_AUTO, FILL_COLOR_CUSTOM);
  }
  if (persist_exists(PERSIST_FILL_COLOR)) {
    s_settings.fill_hex = persist_read_int(PERSIST_FILL_COLOR);
  }
}

static void settings_save(void) {
  persist_write_int(PERSIST_TIME_FORMAT, (int)s_settings.time_format);
  persist_write_bool(PERSIST_LEADING_ZERO, s_settings.leading_zero);
  persist_write_bool(PERSIST_SHOW_DATE, s_settings.show_date);
  persist_write_int(PERSIST_DATE_FORMAT, (int)s_settings.date_format);
  persist_write_bool(PERSIST_SHOW_CAPTION, s_settings.show_caption);
  persist_write_int(PERSIST_PATTERN_MODE, (int)s_settings.pattern_mode);
  persist_write_int(PERSIST_BG_COLOR, s_settings.bg_hex);
  persist_write_int(PERSIST_WALL_COLOR, s_settings.wall_hex);
  persist_write_int(PERSIST_CAPTION_COLOR, s_settings.caption_hex);
  persist_write_int(PERSIST_FILL_MODE, (int)s_settings.fill_mode);
  persist_write_int(PERSIST_FILL_COLOR, s_settings.fill_hex);
}

GColor settings_bg(void) {
#if defined(PBL_COLOR)
  return GColorFromHEX(s_settings.bg_hex);
#else
  return GColorBlack;
#endif
}

GColor settings_wall(void) {
#if defined(PBL_COLOR)
  return GColorFromHEX(s_settings.wall_hex);
#else
  return GColorWhite;
#endif
}

GColor settings_caption(void) {
#if defined(PBL_COLOR)
  return GColorFromHEX(s_settings.caption_hex);
#else
  return GColorWhite;
#endif
}

GColor settings_fill(int hour) {
#if defined(PBL_COLOR)
  if (s_settings.fill_mode == FILL_COLOR_CUSTOM) {
    return GColorFromHEX(s_settings.fill_hex);
  }
  static const GColor palette[12] = {
    GColorCyan,
    GColorRed,
    GColorOrange,
    GColorChromeYellow,
    GColorYellow,
    GColorGreen,
    GColorBrightGreen,
    GColorBlue,
    GColorPurple,
    GColorMagenta,
    GColorShockingPink,
    GColorPictonBlue,
  };
  return palette[((hour % 24) + 24) % 12];
#else
  (void)hour;
  return GColorWhite;
#endif
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  (void)context;
  Tuple *t;

  t = dict_find(iter, MESSAGE_KEY_TimeFormat);
  if (t) {
    s_settings.time_format = (TimeFormat)clamp_int((int)t->value->int32, TIME_FMT_SYSTEM, TIME_FMT_24H);
  }
  t = dict_find(iter, MESSAGE_KEY_LeadingZero);
  if (t) {
    s_settings.leading_zero = t->value->int32 != 0;
  }
  t = dict_find(iter, MESSAGE_KEY_ShowDate);
  if (t) {
    s_settings.show_date = t->value->int32 != 0;
  }
  t = dict_find(iter, MESSAGE_KEY_DateFormat);
  if (t) {
    s_settings.date_format = (DateFormat)clamp_int((int)t->value->int32, DATE_FMT_DOW_DAY_MON, DATE_FMT_DM);
  }
  t = dict_find(iter, MESSAGE_KEY_ShowCaption);
  if (t) {
    s_settings.show_caption = t->value->int32 != 0;
  }
  t = dict_find(iter, MESSAGE_KEY_PatternMode);
  if (t) {
    s_settings.pattern_mode = (PatternMode)clamp_int((int)t->value->int32, PATTERN_AUTO, PATTERN_TRIANGLES);
  }
  t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if (t) {
    s_settings.bg_hex = (int)t->value->int32;
  }
  t = dict_find(iter, MESSAGE_KEY_WallColor);
  if (t) {
    s_settings.wall_hex = (int)t->value->int32;
  }
  t = dict_find(iter, MESSAGE_KEY_CaptionColor);
  if (t) {
    s_settings.caption_hex = (int)t->value->int32;
  }
  t = dict_find(iter, MESSAGE_KEY_FillMode);
  if (t) {
    s_settings.fill_mode = (FillColorMode)clamp_int((int)t->value->int32, FILL_COLOR_AUTO, FILL_COLOR_CUSTOM);
  }
  t = dict_find(iter, MESSAGE_KEY_FillColor);
  if (t) {
    s_settings.fill_hex = (int)t->value->int32;
  }

  settings_save();

  extern void face_settings_changed(void);
  face_settings_changed();
}

const FaceSettings *settings_get(void) {
  return &s_settings;
}

void settings_init(void) {
  settings_load();
  if (!s_inbox_open) {
    app_message_register_inbox_received(inbox_received);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    s_inbox_open = true;
  }
}

void settings_deinit(void) {
  if (s_inbox_open) {
    app_message_deregister_callbacks();
    s_inbox_open = false;
  }
}
