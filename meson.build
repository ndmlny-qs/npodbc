project('npodbc', 'cpp')

py = import('python').find_installation(pure: false)
py_dep = py.dependency()

compiler = meson.get_compiler('cpp')
odbc_dep = compiler.find_library('odbc', required : false)

py.extension_module(
    'npodbc',
    'npodbc.cpp',
    dependencies: [py_dep, odbc_dep],
    install: true,
)
