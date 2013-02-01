Open Instrument Control
=======================

The goal of this project is to develop an embedded-side software stack for
instrument control.  The initial target will be the Arduino platform, however
the goal is to eventually provide support for at least 8/16/32-bit PIC, AVR,
and ARM.

Initial development is for PC, and not much effort has been spent on
efficiency.  Data structures are all dynamically-allocated linked lists,
unsuitable for platforms with constrained resources.

## Version 1 (In development) ##

The first version of OIC will provide a basic platform upon which further
development may begin.  This release shall provide a virtual instrument
platform for the Arduino Uno.

Communication shall be via the virtual serial port provided by the device,
which shall provide a fully conformant implementation of SCPI.

In addition to the basic platform, this release will include two virtual
instruments:

* An implementation of the Digital Meter instrument class using the
	on-board ADC.
* An implementation of the RF and Microwave Source class using the Sparkfun
	AD9835 breakout board.

### 0.1 (In development) ###

Release 0.1 is a prototype of the SCPI implementation, and will implement
contain a fully conforming implementation of SCPI-99 on the PC.

### 0.2 (Planned) ###

Release 0.2 contains an embedded version of the SCPI implementation.  This
is to demonstrated with a simple implementation of the digital meter
instrument and an accompanying Python script to perform the read operation.

### 0.3 (Planned) ###

The 0.3 release will provide complete implementations of the Digital Meter
and RF Source classes as described by the SCPI implementation, along with
the host-side software necessary to operate them.

TODO: Do we neeed a GUI?

### 1.0 (Planned) ###

The 1.0 release will provide multi-instrument support, allowing both the
voltage measurement and signal generator instruments to be accessed with
a single communications channel.

## Version 2 (Planned) ##

The second version will provide support for PIC32 using the Microchip
USB to Serial driver and USBTMC.