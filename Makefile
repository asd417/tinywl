WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WLR_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wlr-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

# 2. A "Target" that depends on the XML file
# This tells Make: "If the XML changes, redo the header."
my-protocol-protocol.h: my-protocol.xml
	wayland-scanner server-header < my-protocol.xml > my-protocol-protocol.h

# 3. Add the generated .c file to your build list
# This ensures the protocol logic is linked into your final binary
tinywl: tinywl.c my-protocol-protocol.c
	$(CC) $(CFLAGS) -o tinywl tinywl.c my-protocol-protocol.c $(LIBS)