#
# Makefile
#

CIRCLEHOME = circle

OBJS	= main.o

LIBS	= $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

OPTIMIZE += -Ofast -funsafe-math-optimizations
include circle/Rules.mk
