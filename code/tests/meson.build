if get_option('with_test').enabled()
    subdir('data')

    run_command(['python3', 'tools' / 'generate-runner.py'], check: true)

    test_src = ['unit_runner.c']
    test_cubes = [
        'operations'
    ]

    foreach cube : test_cubes
        test_src += ['test_' + cube + '.c']
    endforeach

    pizza = executable('runner', test_src,
        include_directories: dir,
        dependencies: [
            dependency('fossil-test'),
            dependency('fossil-mock'),
            fossil_crabdb_dep])

    test('xunit_tests', pizza)  # Renamed the test target for clarity
endif
