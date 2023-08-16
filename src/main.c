#include "core.h"
#include <stdlib.h>

editor_config config;

int main() {
  enableRawMode();
  init_editor();
  get_cursor_pos(&config.screen_rows, &config.screen_cols);

  while (1) {
    refresh_screen();
    process_keypress();
  }

  return 0;
}
