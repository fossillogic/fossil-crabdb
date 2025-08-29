dir = include_directories('.')

dep = [
    dependency('threads'),
    meson.get_compiler('c').find_library('m', required : false)
    ]

fossil_crabdb_lib = library('fossil_crabdb',
    files('myshell.c', 'noshell.c'),
    install: true,
    dependencies: dep,
    include_directories: dir)

fossil_crabdb_dep = declare_dependency(
    link_with: [fossil_crabdb_lib],
    dependencies: dep,
    include_directories: dir)

meson.override_dependency('fossil-crabdb', fossil_crabdb_dep)