WL_SCANNER	= /usr/local/bin/wayland-scanner
WLR_LAYER_SHELL = protocols/wlr-layer-shell-unstable-v1.xml
XDG_SHELL = /usr/local/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

CFLAGS +=	-I/usr/local/include/pixman-1/ -I.
CFLAGS +=	-I/usr/local/include/wlroots-0.20
CFLAGS +=	-I/usr/local/include/
CFLAGS +=	-DWLR_USE_UNSTABLE

LDFLAGS =	-L/usr/local/lib -lwayland-server -lwlroots-0.20 -lxkbcommon -lm

HEADERS =	xdg-shell-protocol.h wlr-layer-shell-unstable-v1-protocol.h

all:	${HEADERS}
	cc ${CFLAGS} ${LDFLAGS} stage.c -o stage

dev:	${HEADERS}
	cc ${CFLAGS} -DSTAGE_DEV ${LDFLAGS} stage.c -o stage

xdg-shell-protocol.h:
	${WL_SCANNER} server-header ${XDG_SHELL} $@

wlr-layer-shell-unstable-v1-protocol.h:
	${WL_SCANNER} server-header ${WLR_LAYER_SHELL} $@

run:
	mkdir -p /tmp/wl
	sudo chmod 0777 /var/run/seatd.sock
	sudo chmod 0777 /dev/dri/card0 /dev/dri/renderD128

clean:
	rm -f stage stage.o ${HEADERS}
