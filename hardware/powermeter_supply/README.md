# powermeter with supply

Here is the simplest standalone powermeter with supply you can build with current and voltage measurement.

![](powermeter_schematic_low.png)

## eagle files

[schematic](powermeter.sch)<br>
[board](powermeter.brd)

## bill of material

1 x 150x100x1mm 35µm FR4 [amazon](https://amzn.eu/d/hufknsN)<br>
esp32 D1 mini [amazon](https://amzn.eu/d/gZaocqF)<br>
ZMPT101 [amazon](https://amzn.eu/d/3shv3Ax)<br>
SCT-013-000 [amazon](https://amzn.eu/d/5wpwDs4)<br>
AC-05-03 ( 110-250V/AC -> 5V/DC ) [amazon](https://amzn.eu/d/ckoURve)<br>
3.5mm jack [amazon](https://amzn.eu/d/a7YUG0G)<br>
MSTBA2 connector [amazon](https://amzn.eu/d/0bCERTn)<br>
2 x 10k resistor<br>
1 x 62R resistor<br>
1 x 1M resistor<br>
1 x 2M resistor<br>
1 x 47µF cap<br>

![](powermeter_top.png)

## gcode

The G-code files are intended for Chilipeppr with a CNC3018 and contain everything. Engraving, drilling and cutting. There are two files. A normal one, where you should have your own gcode for tool change and length measurement and one with the appropriate gcode that also contains the length measurement. The reference point is X=0 and Y=0.

A backlash-free Z-axis is very important for accurate engraving of the tracks and cutting out!

The following tools are required:

0.1mm milling bit<br>
0.6mm drill bit<br>
0.9mm drill bit<br>
1.2mm drill bit<br>
1.3mm drill bit<br>
1.4mm drill bit<br>
2.0mm end mill<br>

Drag and drop the preferred gcode file into Chilipeppr. Autoleveling is a must. And then just press play. For the tool change, the program is paused at the appropriate position (M6).

If everything has gone well, you are rewarded with a finished board without any post-processing and is ready to soldering.

![](powermeter_bottom.png)
![](powermeter_ChiliPeppr_low.png)
![](cnc_1.png)
![](cnc_2.png)

## config

After soldering and software installtion (wifi,mqtt and so on) you can copy the content from config/measure.json to your powermeter. From here you can tweake the ratio settings for precise measuring via web interface under "channel settings".