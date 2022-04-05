WL_SCANNER	= /usr/local/bin/wayland-scanner
WLR_LAYER_SHELL = protocols/wlr-layer-shell-unstable-v1.xml
XDG_SHELL = /usr/local/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

CFLAGS +=	-I/usr/local/include/ -I/usr/local/include/pixman-1/ -I.
CFLAGS +=	-DWLR_USE_UNSTABLE

LDFLAGS =	-L/usr/local/lib -lwayland-server -lwlroots -lxkbcommon

HEADERS =	xdg-shell-protocol.h wlr-layer-shell-unstable-v1-protocol.h

all:	${HEADERS}
	cc ${CFLAGS} -DSTAGE_DEV ${LDFLAGS} stage.c -o stage

prod:	${HEADERS}
	cc ${CFLAGS} ${LDFLAGS} stage.c -o stage

xdg-shell-protocol.h:
	${WL_SCANNER} server-header ${XDG_SHELL} $@

wlr-layer-shell-unstable-v1-protocol.h:
	${WL_SCANNER} server-header ${WLR_LAYER_SHELL} $@

run:
	ssh-agent ./stage

clean:
	rm -f stage stage.o ${HEADERS}
