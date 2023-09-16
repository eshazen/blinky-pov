This repository contains the documentation and firmware for the "Blinky Color POV"
gadget, which displays text in color when waved back and forth in the air.

Downloading is accomplished using a variation of the "Blinky POV" optical download
technique by Wayne&Layne.  See: http://www.wayneandlayne.com/projects/blinky/

Hardware  	       KiCAD PCB layout and gerber files
		       Hardware/gerber/blinky_color2.zip is latest fab package

Firmware/ColorText     Firmware for final version
		       Edit makefile as required for download cable, then
		       'make fuse' and 'make flash'

Programming	       Javascript download tool
		       use 'colortext.html', with on-screen documentation

myFont2c	       Contains 'fontExtract' tool which converts an X11 bitmap
		       font to javascript data

