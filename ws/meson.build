project(
  'ws',
  'c',
  version: '0.0.1',
  license: 'BSD',
  default_options: ['c_std=c99'],
  meson_version: '>=1.1.0',
)

cc = meson.get_compiler('c')
wayland_protos = dependency('wayland-protocols', version: '>=1.13')
wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')
wayland_scanner = find_program('wayland-scanner')
wayland_client = dependency('wayland-client')
rt = cc.find_library('rt')
pixman = dependency('pixman-1')
fcft = dependency('fcft')
epoll = dependency('epoll-shim')

wayland_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
  wayland_scanner,
  output: '@BASENAME@-client-protocol.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

client_protocols = [
  [wl_protocol_dir + '/stable/xdg-shell', 'xdg-shell.xml'],
  [meson.project_source_root() + '/protocols', 'wlr-layer-shell-unstable-v1.xml'],]

foreach p : client_protocols
  xml = join_paths(p)
  src = wayland_scanner_code.process(xml)
  header = wayland_scanner_client.process(xml)

  name = p[1].split('.')[0].underscorify()

  lib = static_library(
    name,
    [src, header],
    dependencies: [wayland_client],
  )

  dep = declare_dependency(
    link_with: lib,
    sources: header,
  )

  set_variable(name, dep)
endforeach

ws_sources = ['src/image.c', 'src/main.c']
ws_dependencies = [wayland_client, wlr_layer_shell_unstable_v1, xdg_shell,
  pixman, fcft, epoll]

executable(
  'ws',
  ws_sources,
  dependencies: ws_dependencies,
  install: true
)
