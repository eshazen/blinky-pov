all:
	mkdir -p zip
	cp blinky_color-B_Cu.gbr zip/blinky_color.GBL
	cp blinky_color-F_Mask.gbr zip/blinky_color.GTS
	cp blinky_color-B_Mask.gbr zip/blinky_color.GBS
	cp blinky_color.drl zip/blinky_color.XLN
	cp blinky_color-F_Cu.gbr zip/blinky_color.GTL
	cp blinky_color-F_SilkS.gbr zip/blinky_color.GTO
	cp blinky_color-B_SilkS.gbr zip/blinky_color.GBO
	cp blinky_color-Edge_Cuts.gbr zip/blinky_color.GKO
	rm -f blinky_color.zip
	zip blinky_color.zip zip/blinky_color.*
