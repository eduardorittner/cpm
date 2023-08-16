#include <stdint.h>
#include <termios.h>

typedef struct {
  uint8_t screen_rows;
  uint8_t screen_cols;

  struct termios base_termios;
} editor_config;

extern editor_config config;

void enableRawMode();

void refresh_screen();

void process_keypress();

void init_editor();

int get_cursor_pos(uint8_t *rows, uint8_t *cols);

