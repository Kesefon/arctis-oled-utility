#include <string.h>
#include <wchar.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "hidapi/hidapi.h"
#include <stdio.h>
#include <stdlib.h>
#include "font.h"

hid_device *device_handle;

void drawpx(int x, int y, int on, unsigned char *buffer) {
  if (on)
    buffer[(x - 1) + (y / 8) * 128] |= (1 << (y & 7));
  else
    buffer[(x - 1) + (y / 8) * 128] &= ~(1 << (y & 7));
}

void open_device() {
  // Initialize the hidapi library
  hid_init();

  // Open the device using the VID & PID, no serial number
  device_handle = hid_open(0x1038, 0x1290, NULL);

  hid_set_nonblocking(device_handle, 0);
}

int draw_bitmap(char bitmap[]) {
  int res;

  char data[1025] = {0};          // 1 Byte too small for full 128x64 resolution, but when this is increased, the base station resets.
  data[0] = 0x00;                 // Report type = 0;
  data[1] = 0xD2;                 // "Draw on screen" function
  memcpy(&data[4], bitmap, 1021); // WTF, why [4]? I assumed this to be 2, but that shifts all pixel. Should be re-investigated.

  res = hid_send_feature_report(device_handle, data, 1025);

  return 0;
}

void close_device() {
  // Close the device
  hid_close(device_handle);

  // Finalize the hidapi library
  hid_exit();
}

void printUsage(char *bin) {
  wprintf(L"Usage:\n"
          L"Display text:\t%s t <text> \n"
          L"Display image:\t%s i <img.pbm>\n"
          L"Display clear:\t%s c\n"
          L"Display video:\tffmpeg -i <video.mp4> -s 128x64 -r 15 -f image2pipe -c:v pbm - | %s v\n",
          bin, bin, bin, bin);
}

int text2bitmap(char *text, char bitmap[1024]) {
  int cur_pos = 0;
  int line_num = 0;
  int i = 0;
  char letter;
  while (((letter = text[i++]) != 0) & (line_num < 10)) {
    for (int j = 0; j < 5 * 5; j++) {
      drawpx((cur_pos * 5) + (j % 5), (line_num * 6) + (j / 5), font[letter][j], bitmap);
    }
    cur_pos++;
    if (cur_pos >= 25) {
      line_num++;
      cur_pos = 0;
    }
  }
  return 0;
}

int pbm2bitmap(char image_data[1034], char bitmap[1024]) {
  if (image_data[0] != 'P' || image_data[1] != '4' ||
      image_data[2] != 0x0A || image_data[3] != '1' || image_data[4] != '2' ||
      image_data[5] != '8' || image_data[6] != ' ' || image_data[7] != '6' ||
      image_data[8] != '4' || image_data[9] != 0x0A) {
    return 1;
  }
  for (int i = 0; i <= 128 * 64; i++) {
    drawpx(i % 128, i / 128, ~image_data[10 + (i / 8)] >> (7 - (i % 8)) & 1, bitmap);
  }
  return 0;
}

int main(int argc, char *argv[]) {

  open_device();
  atexit(close_device);

  if (argc == 3 && *argv[1] == 'i') {
    FILE *image_file;
    if (*argv[2] == '-') {
      image_file = stdin;
    } else {
      image_file = fopen(argv[2], "rb");
        if (image_file == NULL) {
        wprintf(L"cannot open file!\n");
        return 2;
      }
    }
    char image_data[1034] = {0};
    const long image_size = fread(image_data, 1, 1034, image_file);
    if (image_size != 1034 ) {
      wprintf(L"Incorrect file size: %d expected: %d", image_size, 1034);
      return 3;
    }
    char bitmap[1024] = {0};
    int ret = pbm2bitmap(image_data, bitmap);
    if (ret) {
      wprintf(L"Error while parsing pbm!\n Use a 128x64 sized Binary Portable BitMap (PBM)");
      return 4;
    }
    return draw_bitmap(bitmap);
  } else if (argc == 2 && *argv[1] == 'v') {
    char image_data[1034] = {0};
    char bitmap[1024] = {0};
    while (fread(image_data, 1, 1034, stdin) == 1034) {
        int ret = pbm2bitmap(image_data, bitmap);
      if (ret) {
        wprintf(L"Error while parsing pbm!\n Use a 128x64 sized Binary Portable BitMap (PBM)");
        return 4;
      }
      draw_bitmap(bitmap);
      usleep(60000); // if frames go brrrrrrrr basestation go ⣗⣒⣿⣾⣿⡹⣇⣿⣏⣚
                     // this is round about 15fps
                     // 30fps is almost stable
    }
    return 0;
  } else if (argc == 3 && *argv[1] == 't') {
    char bitmap[1024] = {0};
    text2bitmap(argv[2], bitmap);
    return draw_bitmap(bitmap);
  } else if (argc == 2 && *argv[1] == 'c') {
    char bitmap[1024] = {0};
    return draw_bitmap(bitmap);
  } else {
    printUsage(argv[0]);
    return 1;
  }
}
