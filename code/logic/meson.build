dir = include_directories('.')

code_deps = [
    dependency('threads')
]

fossil_crabdb_lib = library('fossil-crabdb',
    files('database.c', 'internal.c'),
    install: true,
    dependencies: code_deps,
    include_directories: dir)

fossil_crabdb_dep = declare_dependency(
    link_with: [fossil_crabdb_lib],
    dependencies: code_deps,
    include_directories: dir)
