v1 and v2 have a design error. IO2 must be pulled low during flash, but gets pulled high by the pullup.
Easy fix: Cut the pin connectiong to IO2, and bridge IO2 and IO0 using a solder bridge.
