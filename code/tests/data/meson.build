fs = import('fs')
data = [
    'delete_command.crabql',
    'insert_commands.crabql',
    'select_command.crabql',
    'sort_key_command.crabql',
    'test_condition_met.crabql',
    'test_condition_not_met.crabql',
    'test_invalid.txt',
    'test_unknown_command.crabql',
    'test_valid.crabql',
    'update_command.crabql'
]

foreach iter : data
    fs.copyfile(iter, iter)
endforeach
