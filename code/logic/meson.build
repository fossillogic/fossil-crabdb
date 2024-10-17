dir = include_directories('.')

fossil_crabdb_lib = library('fossil-crabdb',
    files('database.c', 'internal.c'),
    install: true,
    include_directories: dir)

fossil_crabdb_dep = declare_dependency(
    link_with: [fossil_crabdb_lib],
    include_directories: dir)
