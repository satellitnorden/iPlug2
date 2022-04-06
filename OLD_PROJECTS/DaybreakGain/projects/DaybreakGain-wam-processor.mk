include ../config/DaybreakGain-web.mk

TARGET = ../build-web/scripts/DaybreakGain-wam.js

SRC += $(WAM_SRC)
CFLAGS += $(WAM_CFLAGS)
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += $(WAM_LDFLAGS) \
-s EXPORTED_FUNCTIONS=$(WAM_EXPORTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)
