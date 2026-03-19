project('tinywl', 'c', 
  default_options: [
    'c_std=c11',
    'pkg_config_path=/usr/local/lib/pkgconfig'
  ],
  meson_version: '>=1.8.0'
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
wl_mod = import('wayland')

# In 2026, wlroots 0.20 is the current stable/RC series
wlroots = dependency('wlroots-0.20')
wayland_client = dependency('wayland-client')
wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')

# --- 3. Protocol Generation (The "Meson Way") ---
protocols_dir = wayland_protos.get_variable('pkgdatadir')

# Instead of custom_target, use wl_mod.scan_xml for EVERYTHING. 
# It handles the scanner_bin and dependency tracking internally.
xdg_shell_xml = protocols_dir / 'stable/xdg-shell/xdg-shell.xml'
xdg_shell_protos = wl_mod.scan_xml(xdg_shell_xml, server: true, client: false)

custom_protocol_xml = 'my-protocol.xml'
custom_protos = wl_mod.scan_xml(custom_protocol_xml, server: true, client: true)

cc = meson.get_compiler('c')
rt_dep = cc.find_library('rt', required: false)

# --- 4. The Executable ---
executable(
    'tinywl',
    [
        'tinywl.c', 
        xdg_shell_protos,
		custom_protos
    ],
    dependencies: [
        wlroots, 
        wayland_client,
		wayland_server,
		xkbcommon,
		rt_dep,
    ],
    install: true
)