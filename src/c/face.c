#include "face.h"

typedef enum {
  INFILL_LINES = 0,
  INFILL_HONEYCOMB,
  INFILL_GYROID,
  INFILL_TRIANGLES,
  INFILL_COUNT
} InfillKind;

// Digit cell grid: '#' wall, '+' interior, ' ' outside.
#define DIGIT_COLS 7
#define DIGIT_ROWS 11

static Window *s_window;
static Layer *s_layer;
static int s_hour;
static int s_minute;
static int s_second;

static const char *const DIGIT_GLYPHS[10][DIGIT_ROWS] = {
  { // 0
    " ##### ",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    " ##### ",
  },
  { // 1
    "  ###  ",
    " ####  ",
    " +###  ",
    "  ###  ",
    "  ###  ",
    "  ###  ",
    "  ###  ",
    "  ###  ",
    "  ###  ",
    "  ###  ",
    " ##### ",
  },
  { // 2
    " ##### ",
    "##+++##",
    "   ++##",
    "    ###",
    "   ### ",
    "  ###  ",
    " ###   ",
    "###    ",
    "##++   ",
    "##+++++",
    "#######",
  },
  { // 3
    " ##### ",
    "##+++##",
    "    ++#",
    "    ###",
    "  #### ",
    "  +### ",
    "    ###",
    "    ++#",
    "    ++#",
    "##+++##",
    " ##### ",
  },
  { // 4
    "##   ##",
    "##   ##",
    "##+++##",
    "##+++##",
    "##+++##",
    "#######",
    "   ### ",
    "   ### ",
    "   ### ",
    "   ### ",
    "  #####",
  },
  { // 5
    "#######",
    "##+++++",
    "##     ",
    "###### ",
    "##+++##",
    "    ++#",
    "    ++#",
    "    ++#",
    "    ++#",
    "##+++##",
    " ##### ",
  },
  { // 6
    " ##### ",
    "##+++##",
    "##++   ",
    "##     ",
    "###### ",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    " ##### ",
  },
  { // 7
    "#######",
    "+++++##",
    "    ## ",
    "   ##  ",
    "  ##   ",
    " ###   ",
    " ###   ",
    " ###   ",
    " ###   ",
    " ###   ",
    " ###   ",
  },
  { // 8
    " ##### ",
    "##+++##",
    "##+++##",
    "##+++##",
    " ##### ",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    " ##### ",
  },
  { // 9
    " ##### ",
    "##+++##",
    "##+++##",
    "##+++##",
    "##+++##",
    " ######",
    "    ++#",
    "    ++#",
    "   ++##",
    "##+++##",
    " ##### ",
  },
};

static GColor color_bg(void) {
#if defined(PBL_COLOR)
  return GColorBlack;
#else
  return GColorBlack;
#endif
}

static GColor color_wall(void) {
#if defined(PBL_COLOR)
  return GColorWhite;
#else
  return GColorWhite;
#endif
}

static GColor color_caption(void) {
#if defined(PBL_COLOR)
  return GColorLightGray;
#else
  return GColorWhite;
#endif
}

static GColor color_infill(int hour) {
#if defined(PBL_COLOR)
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

static InfillKind infill_for_hour(int hour) {
  return (InfillKind)(((hour % 24) + 24) % INFILL_COUNT);
}

static const char *infill_name(InfillKind kind) {
  static const char *names[INFILL_COUNT] = {
    "lines", "honeycomb", "gyroid", "triangles",
  };
  return names[((int)kind % INFILL_COUNT + INFILL_COUNT) % INFILL_COUNT];
}

// Progress through the hour: 0..1000 (allows a thin first layer at :00).
static int fill_progress_permille(void) {
  int total = s_minute * 60 + s_second;
  if (total <= 0) {
    return 40;  // first "layer"
  }
  if (total >= 3599) {
    return 1000;
  }
  return 40 + (total * 960) / 3599;
}

// Integer sin in roughly -amp..amp from pebble angle units via lookup.
static int isin_amp(int angle_units, int amp) {
  return (sin_lookup(angle_units) * amp) / TRIG_MAX_RATIO;
}

static bool pattern_hit(InfillKind kind, int x, int y) {
  switch (kind) {
    case INFILL_LINES:
      return (y & 3) < 2;

    case INFILL_HONEYCOMB: {
      // Staggered hex cells; draw walls on cell borders.
      const int hw = 6;
      const int hh = 5;
      int row = y / hh;
      int shift = (row & 1) * (hw / 2);
      int lx = ((x + shift) % hw + hw) % hw;
      int ly = ((y % hh) + hh) % hh;
      if (lx == 0) {
        return true;
      }
      if (ly == 0 && lx <= hw / 2) {
        return true;
      }
      if (ly == hh / 2 && lx >= hw / 2) {
        return true;
      }
      if (ly == 0 && lx == hw - 1) {
        return true;
      }
      return false;
    }

    case INFILL_GYROID: {
      // Wavy vertical strands — a flat gyroid stand-in.
      int wave = isin_amp((y * TRIG_MAX_ANGLE) / 18, 3);
      int t = x + wave;
      int m = t % 5;
      if (m < 0) {
        m += 5;
      }
      return m < 2;
    }

    case INFILL_TRIANGLES: {
      const int s = 6;
      int a = ((x + y) % s + s) % s;
      int b = ((x - y) % s + s) % s;
      int c = ((y % s) + s) % s;
      return a == 0 || b == 0 || c == 0;
    }

    default:
      return false;
  }
}

static char glyph_at(int digit, int col, int row) {
  if (digit < 0 || digit > 9 || col < 0 || col >= DIGIT_COLS || row < 0 || row >= DIGIT_ROWS) {
    return ' ';
  }
  return DIGIT_GLYPHS[digit][row][col];
}

static void draw_digit(GContext *ctx, int digit, GRect box, InfillKind kind,
                       GColor wall, GColor fill, int progress_pm) {
  if (box.size.w <= 0 || box.size.h <= 0) {
    return;
  }

  const int cell_w = box.size.w / DIGIT_COLS;
  const int cell_h = box.size.h / DIGIT_ROWS;
  if (cell_w <= 0 || cell_h <= 0) {
    return;
  }

  const int used_w = cell_w * DIGIT_COLS;
  const int used_h = cell_h * DIGIT_ROWS;
  const int ox = box.origin.x + (box.size.w - used_w) / 2;
  const int oy = box.origin.y + (box.size.h - used_h) / 2;

  // Fill rises from the bottom of the glyph.
  const int fill_top = oy + used_h - (used_h * progress_pm) / 1000;

  for (int row = 0; row < DIGIT_ROWS; row++) {
    for (int col = 0; col < DIGIT_COLS; col++) {
      const char ch = glyph_at(digit, col, row);
      if (ch == ' ') {
        continue;
      }
      const int px = ox + col * cell_w;
      const int py = oy + row * cell_h;

      if (ch == '#') {
        graphics_context_set_fill_color(ctx, wall);
        graphics_fill_rect(ctx, GRect(px, py, cell_w, cell_h), 0, GCornerNone);
        continue;
      }

      // Interior: pattern only below the rising fill line.
      if (ch == '+') {
        for (int dy = 0; dy < cell_h; dy++) {
          const int sy = py + dy;
          if (sy < fill_top) {
            continue;
          }
          for (int dx = 0; dx < cell_w; dx++) {
            const int sx = px + dx;
            if (pattern_hit(kind, sx, sy)) {
              graphics_context_set_stroke_color(ctx, fill);
              graphics_draw_pixel(ctx, GPoint(sx, sy));
            }
          }
        }
      }
    }
  }
}

static void draw_colon(GContext *ctx, GRect box, InfillKind kind, GColor wall,
                       GColor fill, int progress_pm) {
  int dot = box.size.w;
  if (dot > box.size.h / 4) {
    dot = box.size.h / 4;
  }
  if (dot < 4) {
    dot = 4;
  }
  int cx = box.origin.x + box.size.w / 2;
  int gap = box.size.h / 6;
  int y0 = box.origin.y + box.size.h / 2 - gap - dot;
  int y1 = box.origin.y + box.size.h / 2 + gap;

  GRect dots[2] = {
    GRect(cx - dot / 2, y0, dot, dot),
    GRect(cx - dot / 2, y1, dot, dot),
  };

  int fill_top = box.origin.y + box.size.h - (box.size.h * progress_pm) / 1000;

  for (int i = 0; i < 2; i++) {
    GRect d = dots[i];
    // Outer wall
    graphics_context_set_fill_color(ctx, wall);
    graphics_fill_rect(ctx, d, 0, GCornerNone);
    // Hollow interior
    int inset = dot > 5 ? 2 : 1;
    GRect inner = GRect(d.origin.x + inset, d.origin.y + inset,
                        d.size.w - inset * 2, d.size.h - inset * 2);
    if (inner.size.w > 0 && inner.size.h > 0) {
      graphics_context_set_fill_color(ctx, color_bg());
      graphics_fill_rect(ctx, inner, 0, GCornerNone);
      for (int y = inner.origin.y; y < inner.origin.y + inner.size.h; y++) {
        if (y < fill_top) {
          continue;
        }
        for (int x = inner.origin.x; x < inner.origin.x + inner.size.w; x++) {
          if (pattern_hit(kind, x, y)) {
            graphics_context_set_stroke_color(ctx, fill);
            graphics_draw_pixel(ctx, GPoint(x, y));
          }
        }
      }
    }
  }
}

static void draw_frame(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, color_bg());
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  InfillKind kind = infill_for_hour(s_hour);
  GColor wall = color_wall();
  GColor fill = color_infill(s_hour);
  int progress = fill_progress_permille();

  int hour = s_hour;
  if (!clock_is_24h_style()) {
    hour = hour % 12;
    if (hour == 0) {
      hour = 12;
    }
  }

  int d0 = (hour / 10) % 10;
  int d1 = hour % 10;
  int d2 = (s_minute / 10) % 10;
  int d3 = s_minute % 10;

  const int caption_h = 28;
  const int top_pad = 6;
  const int side_pad = 6;
  int area_h = bounds.size.h - caption_h - top_pad;
  int area_w = bounds.size.w - side_pad * 2;
  int area_y = top_pad;

  // Layout: [H][H][:][M][M] with colon narrower.
  int colon_w = area_w / 10;
  if (colon_w < 8) {
    colon_w = 8;
  }
  if (colon_w > 16) {
    colon_w = 16;
  }
  int digit_w = (area_w - colon_w) / 4;
  int digit_h = area_h;
  // Keep digit aspect near glyph 7:11
  if (digit_h > (digit_w * DIGIT_ROWS) / DIGIT_COLS) {
    digit_h = (digit_w * DIGIT_ROWS) / DIGIT_COLS;
  }
  if (digit_w > (digit_h * DIGIT_COLS) / DIGIT_ROWS) {
    digit_w = (digit_h * DIGIT_COLS) / DIGIT_ROWS;
  }

  int total_w = digit_w * 4 + colon_w;
  int start_x = (bounds.size.w - total_w) / 2;
  int start_y = area_y + (area_h - digit_h) / 2;

  GRect r0 = GRect(start_x, start_y, digit_w, digit_h);
  GRect r1 = GRect(start_x + digit_w, start_y, digit_w, digit_h);
  GRect rc = GRect(start_x + digit_w * 2, start_y, colon_w, digit_h);
  GRect r2 = GRect(start_x + digit_w * 2 + colon_w, start_y, digit_w, digit_h);
  GRect r3 = GRect(start_x + digit_w * 3 + colon_w, start_y, digit_w, digit_h);

  draw_digit(ctx, d0, r0, kind, wall, fill, progress);
  draw_digit(ctx, d1, r1, kind, wall, fill, progress);
  draw_colon(ctx, rc, kind, wall, fill, progress);
  draw_digit(ctx, d2, r2, kind, wall, fill, progress);
  draw_digit(ctx, d3, r3, kind, wall, fill, progress);

  // Rising "print" surface line across the digit band.
  int fill_y = start_y + digit_h - (digit_h * progress) / 1000;
  graphics_context_set_stroke_color(ctx, fill);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(start_x, fill_y), GPoint(start_x + total_w - 1, fill_y));

  graphics_context_set_text_color(ctx, color_caption());
  char line[36];
  snprintf(line, sizeof(line), "%s  ·  %d/59", infill_name(kind), s_minute);
  GRect cap = GRect(0, bounds.size.h - 22, bounds.size.w, 18);
  graphics_draw_text(ctx, line, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     cap, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void face_tick(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_second = t->tm_sec;
  layer_mark_dirty(s_layer);
}

void face_init(Window *window) {
  s_window = window;
  window_set_background_color(window, color_bg());

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_second = t->tm_sec;

  s_layer = layer_create(layer_get_bounds(window_get_root_layer(window)));
  layer_set_update_proc(s_layer, draw_frame);
  layer_add_child(window_get_root_layer(window), s_layer);
}

void face_deinit(void) {
  if (s_layer) {
    layer_destroy(s_layer);
    s_layer = NULL;
  }
}
