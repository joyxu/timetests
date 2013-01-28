CFLAGS += -O3 -Wl,-no-as-needed
LDFLAGS += -lrt -lpthread
bins = inconsistency-check threadtest consistent-offsets leap-a-day leapcrash raw_skew change_skew set-2038 nanosleep clocksource-switch


all: ${bins}

clean:
	rm -f ${bins}
