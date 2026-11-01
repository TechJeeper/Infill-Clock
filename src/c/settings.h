#pragma once
#include <pebble.h>

typedef enum {
  TIME_FMT_SYSTEM = 0,
  TIME_FMT_12H = 1,
  TIME_FMT_24H = 2,
} TimeFormat;

typedef enum {
  DATE_FMT_DOW_DAY_MON = 0,
  DATE_FMT_MON_DAY = 1,
  DATE_FMT_MD = 2,
  DATE_FMT_DM = 3,
} DateFormat;

typedef enum {
  PATTERN_AUTO = 0,
  PATTERN_LINES = 1,
  PATTERN_HONEYCOMB = 2,
  PATTERN_GYROID = 3,
  PATTERN_TRIANGLES = 4,
} PatternMode;

typedef enum {
  FILL_COLOR_AUTO = 0,
  FILL_COLOR_CUSTOM = 1,
} FillColorMode;

typedef struct {
  TimeFormat time_format;
  bool leading_zero;
  bool show_date;
  DateFormat date_format;
  bool show_caption;
  PatternMode pattern_mode;
  int bg_hex;
  int wall_hex;
  int caption_hex;
  FillColorMode fill_mode;
  int fill_hex;
} FaceSettings;

void settings_init(void);
void settings_deinit(void);
const FaceSettings *settings_get(void);

GColor settings_bg(void);
GColor settings_wall(void);
GColor settings_caption(void);
GColor settings_fill(int hour);
