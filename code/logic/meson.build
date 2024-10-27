dir = include_directories('.')

fossil_crabdb_lib = library('fossil-crabdb',
    files('database.c', 'query.c', 'logging.c', 'ai.c', 'rest.c', 'sync.c', 'search.c'),
    install: true,
    dependencies: dependency('threads'),
    include_directories: dir)

fossil_crabdb_dep = declare_dependency(
    link_with: [fossil_crabdb_lib],
    dependencies: dependency('threads'),
    include_directories: dir)
