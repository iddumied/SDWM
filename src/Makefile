# sdwm - stylish dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = sdwm.c
OBJ = ${SRC:.c=.o}

all: options sdwm

options:
	@echo sdwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

sdwm: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f sdwm ${OBJ} sdwm-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sdwm-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk \
		sdwm.1 ${SRC} sdwm-${VERSION}
	@tar -cf sdwm-${VERSION}.tar sdwm-${VERSION}
	@gzip sdwm-${VERSION}.tar
	@rm -rf sdwm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sdwm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sdwm
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < sdwm.1 > ${DESTDIR}${MANPREFIX}/man1/sdwm.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/sdwm.1
	@mkdir -p ${DESTDIR}${LOGPREFIX}
	@touch ${DESTDIR}${LOGPREFIX}/sdwm
	@chmod 777 ${DESTDIR}${LOGPREFIX}/sdwm

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sdwm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/sdwm.1
	@echo removing log file from ${DESTDIR}${LOGPREFIX}
	@rm -f ${DESTDIR}${LOGPREFIX}/sdwm

.PHONY: all options clean dist install uninstall
