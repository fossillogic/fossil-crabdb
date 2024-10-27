dir = include_directories('.')

dep = [dependency('threads')]

fossil_crabdb_lib = library('fossil-crabdb',
    files('database.c', 'query.c', 'logging.c', 'ai.c', 'rest.c', 'sync.c', 'search.c'),
    install: true,
    dependencies: dep,
    include_directories: dir)

fossil_crabdb_dep = declare_dependency(
    link_with: [fossil_crabdb_lib],
    dependencies: dep,
    include_directories: dir)
