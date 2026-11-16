APP_ID      ?= com.plexamp.Plexamp
INSTALL_DIR ?= $(HOME)/.var/app/$(APP_ID)/tweaks
LIB         := plexamp-tweaks.so

# Runtime settings written into the flatpak override. Leave one empty to skip it.
SMOOTH_SCROLLING ?= 0
ZOOM             ?= 1.25
HW_ACCEL         ?=

CC     ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

ENV_VARS := LD_PRELOAD PLEXAMP_SMOOTH_SCROLLING PLEXAMP_ZOOM PLEXAMP_HW_ACCEL

.PHONY: all install override show unoverride uninstall clean

all: $(LIB)

$(LIB): plexamp-tweaks.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -ldl

install: $(LIB)
	install -Dm755 $(LIB) $(INSTALL_DIR)/$(LIB)

override: install
	flatpak override --user $(APP_ID) \
		--env=LD_PRELOAD=$(INSTALL_DIR)/$(LIB) \
		$(if $(SMOOTH_SCROLLING),--env=PLEXAMP_SMOOTH_SCROLLING=$(SMOOTH_SCROLLING),--unset-env=PLEXAMP_SMOOTH_SCROLLING) \
		$(if $(ZOOM),--env=PLEXAMP_ZOOM=$(ZOOM),--unset-env=PLEXAMP_ZOOM) \
		$(if $(HW_ACCEL),--env=PLEXAMP_HW_ACCEL=$(HW_ACCEL),--unset-env=PLEXAMP_HW_ACCEL)
	@$(MAKE) --no-print-directory show

show:
	flatpak override --user --show $(APP_ID)

# Removes only the variables this project sets; other overrides are left alone.
unoverride:
	flatpak override --user $(APP_ID) $(foreach v,$(ENV_VARS),--unset-env=$(v))

uninstall: unoverride
	rm -f $(INSTALL_DIR)/$(LIB)
	-rmdir $(INSTALL_DIR) 2>/dev/null

clean:
	rm -f $(LIB)
