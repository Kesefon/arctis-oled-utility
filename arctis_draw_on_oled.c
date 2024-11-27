#ifdef WIN32
#include <windows.h>
#endif
#include "hidapi/hidapi.h"
#include <stdio.h>
#include <stdlib.h>

void drawpx(int x, int y, int on, unsigned char *buffer) {
  if (on)
    buffer[(x - 1) + (y / 8) * 128] |= (1 << (y & 7));
  else
    buffer[(x - 1) + (y / 8) * 128] &= ~(1 << (y & 7));
}

int draw_bitmap(char image_data[]) {
  int res;
  hid_device *handle;

  // Initialize the hidapi library
  res = hid_init();

  // Open the device using the VID & PID, no serial number
  handle = hid_open(0x1038, 0x1290, NULL);

  hid_set_nonblocking(handle, 0);

  char data[1025] = {0};
  data[0] = 0x00; // Report type = 0;
  data[1] = 0xD2; // "Draw on screen" function

  for (int i = 0; i <= 128 * 64; i++) {
    drawpx(i % 128, i / 128, ~image_data[(i / 8)] >> (7 - (i % 8)) & 1,
           &data[2]);
  }

  res = hid_send_feature_report(handle, data, 1025);

  // Close the device
  hid_close(handle);

  // Finalize the hidapi library
  res = hid_exit();

  return 0;
}

int main(int argc, char *argv[]) {
  FILE *image_file = fopen(argv[1], "rb");
  if (image_file == NULL)
    exit(2);
  char image_data[1024] = {0};
  const long image_size = fread(image_data, 1, 1024, image_file);
  if (image_size != 1024 || image_data[0] != 'P' || image_data[1] != '4' ||
      image_data[2] != 0x0A || image_data[3] != '1' || image_data[4] != '2' ||
      image_data[5] != '8' || image_data[6] != ' ' || image_data[7] != '6' ||
      image_data[8] != '4' || image_data[9] != 0x0A) {
    wprintf(
        L"invalid file!\nUse a 128x64 sized Binary Portable BitMap (PBM) \n");
    exit(3);
  }
  return draw_bitmap(image_data + 10);
}
