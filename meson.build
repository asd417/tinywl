project('tinywl', 'c', 
  default_options: [
    'c_std=c11',
    'pkg_config_path=/usr/local/lib/pkgconfig'
  ]
)

# --- 1. Project Arguments & Endianness ---
# We use .to_int() to convert boolean to 0 or 1 for C macros
is_little = target_machine.endian() == 'little'
is_big = target_machine.endian() == 'big'

add_project_arguments([
    '-D_POSIX_C_SOURCE=200809L',
    '-DWLR_USE_UNSTABLE',
    '-DWLR_LITTLE_ENDIAN=@0@'.format(is_little.to_int()),
    '-DWLR_BIG_ENDIAN=@0@'.format(is_big.to_int()),
], language: 'c')

# --- 2. Dependencies ---
wlroots = dependency('wlroots-0.20')
wayland_client = dependency('wayland-client')
wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
# native: true ensures we get the scanner for the build machine (to run it)
wayland_scanner = dependency('wayland-scanner', native: true)

# --- 3. Protocol Generation ---
# Get the scanner executable and the protocols directory
scanner_bin = find_program(wayland_scanner.get_variable('wayland_scanner'))
protocols_dir = wayland_protos.get_variable('pkgdatadir')
xdg_shell_xml = protocols_dir / 'stable/xdg-shell/xdg-shell.xml'

# We need TWO targets: one for the header (.h) and one for the code (.c)
xdg_shell_header = custom_target(
    'xdg-shell-header',
    output: 'xdg-shell-protocol.h',
    input: xdg_shell_xml,
    command: [scanner_bin, 'server-header', '@INPUT@', '@OUTPUT@'],
)

xdg_shell_code = custom_target(
    'xdg-shell-code',
    output: 'xdg-shell-protocol.c',
    input: xdg_shell_xml,
    command: [scanner_bin, 'private-code', '@INPUT@', '@OUTPUT@'],
)

# Cross-platform/Old Glibc fix for clocks
cc = meson.get_compiler('c')
rt_dep = cc.find_library('rt', required: false)

# --- 4. The Executable ---
executable(
    'tinywl',
    [
        'tinywl.c', 
        xdg_shell_header, 
        xdg_shell_code
    ],
    dependencies: [
        wlroots, 
        wayland_client,
		wayland_server,
		xkbcommon,
		rt_dep
    ],
    install: true
)