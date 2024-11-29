# Arctis Pro OLED utility thingy

Display whatever you want on your SteelSeries Arctis Pro's base station display.  
This is based on a [PoC by Spirit532](https://gist.github.com/Spirit532/3390d754f450b090783d074c1e7bab56).

## Usage

Display text:   `./arctis-oled-utility t <text>`  
Display image:  `./arctis-oled-utility i <img.pbm>`  
Display clear:  `./arctis-oled-utility c`  
Display video[^1]:  `ffmpeg -i <video.mp4> -s 128x64 -r 15 -f image2pipe -c:v pbm - | ./arctis-oled-utility v`  

[^1]: This example assumes that ffmpeg is installed.

## Building

### Linux

This software depends on `hidapi` so install it with your favourite package manager. Then run:  
`gcc arctis_draw_on_oled.c -lhidapi-hidraw -o arctis-oled-utility`

### Windows/MacOS/FreeBSD

I only ever tested this on Linux, but I don't see any reason why it shouldn't work.  
Feel free to try and add your results here.

## Disclaimer

This is all unofficial.  
It might break your device.  
Or not.  
I don't know.  
I don't care.  
