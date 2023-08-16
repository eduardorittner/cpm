#include "core.h"
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CPM_VERSION "0.1"

#define CTRL_KEY(k) ((k)&0x1f)

void die(const char *s) {
  write(STDOUT_FILENO, "x1b[2J", 4);
  write(STDOUT_FILENO, "x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.base_termios) == -1) {
    die("disable raw mode");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &config.base_termios) == -1)
    die("tcgetattr for enabling raw mode");
  atexit(disableRawMode);

  struct termios raw = config.base_termios;

  // IXON - Disables C-s and C-q
  // ICRNL - Disables C-m
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
  // Disables these flags
  // ECHO - prints all input keys
  // ICANON - reads input line by line instead of byte by byte
  // ISIG - Disables C-c and C-z
  // IEXTEN - Disables C-v
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  // OPOST - Disables carriage return \r before \n
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr for enabling raw mode");
}

char read_key() {
  int read_output;
  char c;
  while ((read_output = read(STDIN_FILENO, &c, 1)) != 1) {
    if (read_output == -1 && errno != EAGAIN) {
      die("read");
    }
  }
  return c;
}

void buf_append(ap_buf *buf, const char *str, int len) {

  char *new = realloc(buf->str, buf->len + len);

  if (new == NULL) {
    return;
  }
  memcpy(&new[buf->len], str, len);
  buf->str = new;
  buf->len += len;
}

void buf_free(ap_buf *buf) { free(buf->str); }

int get_cursor_pos(uint8_t *rows, uint8_t *cols) {

  char buffer[32];
  uint8_t i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
    return -1;
  }

  while (i < sizeof(buffer) - 1) {
    if (read(STDIN_FILENO, &buffer[i], 1) != 1) {
      break;
    }
    if (buffer[i] == 'R') {
      break;
    }
    i++;
  }

  buffer[i] = '\0';

  if (buffer[0] != '\x1b' || buffer[1] != '[') {
    return -1;
  }

  if (sscanf(&buffer[2], "%c;%c", rows, cols) != 2) {
    return -1;
  }

  return 0;
}

int window_size(uint8_t *rows, uint8_t *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void draw_rows(ap_buf *buf) {
  for (int i = 0; i < config.screen_rows - 1; i++) {
    if (i == config.screen_cols / 3) {
      char welcome[80];
      int len = snprintf(welcome, sizeof(welcome), "cpm editor -- version %s",
                         CPM_VERSION);
      buf_append(buf, "~ blablablabla\r\n", 17);
      if (len > config.screen_cols) {
        len = config.screen_cols;
      }
      int padding = (config.screen_cols - len) / 2;
      if (padding) {
        buf_append(buf, "~", 1);
      }
      while (padding--) {
        buf_append(buf, welcome, len);
      }
    } else {
      buf_append(buf, "~", 1);
    }
    buf_append(buf, "\x1b[K", 3);
    buf_append(buf, "\r\n", 2);
  }
  buf_append(buf, "~", 1);
}

void refresh_screen() {
  ap_buf buf = ABUF_INIT;

  buf_append(&buf, "\x1b[?25l", 6);
  buf_append(&buf, "\x1b[K", 4);
  buf_append(&buf, "\x1b[H", 3);

  draw_rows(&buf);

  buf_append(&buf, "\x1b[H", 3);
  buf_append(&buf, "\x1b[?25h", 6);

  write(STDOUT_FILENO, buf.str, buf.len);
  buf_free(&buf);
}

void process_keypress() {
  char c = read_key();
  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}

void init_editor() {
  if (window_size(&config.screen_rows, &config.screen_cols) == -1)
    die("get windows size");
}
