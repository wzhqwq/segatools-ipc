project('segatools', 'c', version: '0.1.0')

add_project_arguments(
    '-DCOBJMACROS',
    '-DDIRECTINPUT_VERSION=0x0800',
    '-DWIN32_LEAN_AND_MEAN',
    '-D_WIN32_WINNT=_WIN32_WINNT_WIN7',
    '-DMINGW_HAS_SECURE_API=1',
    language: 'c',
)

# Use get_argument_syntax() instead once Meson 0.49.0 releases
if meson.get_compiler('c').get_id() != 'msvc'
    add_project_arguments(
        '-ffunction-sections',
        '-fdata-sections',
        language: 'c',
    )

    add_project_link_arguments(
        '-Wl,--enable-stdcall-fixup',
        '-Wl,--exclude-all-symbols',
        '-Wl,--gc-sections',
        '-static-libgcc',
        language: 'c',
    )
endif

cc = meson.get_compiler('c')
shlwapi_lib = cc.find_library('shlwapi')
dinput8_lib = cc.find_library('dinput8')
dxguid_lib = cc.find_library('dxguid')
xinput_lib = cc.find_library('xinput')

inc = include_directories('.')
capnhook = subproject('capnhook')

subdir('amex')
subdir('iccard')
subdir('board')
subdir('hooklib')
subdir('jvs')
subdir('platform')
subdir('util')

subdir('aimeio')
# subdir('chuniio')
# subdir('divaio')
# subdir('idzio')
# subdir('mu3io')
subdir('mai2io')
# subdir('fgoio')
# subdir('carolio')

# subdir('chunihook')
# subdir('divahook')
# subdir('idzhook')
# subdir('minihook')
# subdir('mu3hook')
subdir('mai2hook')
# subdir('fgohook')
# subdir('carolhook')
subdir('sinmaihook')