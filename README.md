# ***Blue CrabDB Database System by Fossil Logic***

Blue CrabDB is a comprehensive, platform-independent database system designed to streamline the management and storage of key-value pairs within namespaces. Written in C and C++, Blue CrabDB is built to offer high performance and reliability across various operating systems. The C and C++ components are built using the Meson build system, ensuring efficient development and deployment processes.

## Key Features

1. **Cross-Platform Compatibility**: Ensures consistent performance and functionality on Windows, macOS, Linux, and other platforms.
2. **Modular Design**: Offers a collection of modular components that can be easily integrated and customized to meet specific project requirements.
3. **Efficient Build Systems**: Utilizes Meson for C and C++ components, providing a modern build system known for its fast build times and user-friendly syntax.
4. **High Performance**: Written in C and C++ to provide optimal performance and resource efficiency, making it suitable for desktop and embedded applications.
5. **Extensive Documentation and Examples**: Comes with comprehensive documentation and sample code to aid developers in getting started quickly and effectively.

## Prerequisites

Before getting started, make sure you have the following installed:

- **Meson Build System**: This project relies on Meson. If you don't have Meson installed, visit the official [Meson website](https://mesonbuild.com/Getting-meson.html) for installation instructions.

## Adding Dependency

1. **Install Meson Build System**: Before integrating the dependency, ensure you have Meson `1.3` or newer installed on your host system. You can install it with this command:

   ```sh
   python -m pip install meson           # to install Meson
   python -m pip install --upgrade meson # to upgrade Meson
   ```

2. **Adding Wrap File**: You can add a `.wrap`, first go into the `subprojects` directory and create `fossil-crabdb.wrap` then copy the definition into the file:

   ```ini
   # ======================
   # Git Wrap package definition
   # ======================
   [wrap-git]
   url = https://github.com/fossillogic/fossil-crabdb.git
   revision = v0.1.3

   [provide]
   fossil-crabdb = fossil_crabdb_dep
   ```

3. **Integrate the New Dependency**: After creating the dependency `.wrap` file, you need to integrate it into your Meson project. This typically involves adding the dependency to your `meson.build` file. Here's an example of how you might do that:

   ```ini
   dep = dependency('fossil-crabdb') # includes everything
   ```

   This line retrieves the `fossil-crabdb` dependency, allowing you to use it in your project.

## Configure Options

You have options when configuring the build, each serving a different purpose:

- **Running Tests**: To enable running tests, use `-Dwith_test=enabled` when configuring the build.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

## Usage Examples

### CrabQL SQL-like Commands

CrabQL provides SQL-like commands to interact with the Blue CrabDB database. Here are some examples of how to use these commands:

```sql
-- Insert a new key-value pair
INSERT INTO table_name (key, value) VALUES ('myKey', 'myValue');

-- Update an existing key-value pair
UPDATE table_name SET value = 'newValue' WHERE key = 'myKey';

-- Select a value by key
SELECT value FROM table_name WHERE key = 'myKey';

-- Delete a key-value pair
DELETE FROM table_name WHERE key = 'myKey';

-- List all key-value pairs
LIST;

-- Clear all key-value pairs
CLEAR;

-- Show the contents of the database
SHOW;

-- Drop the entire database
DROP;

-- Check if a key exists
EXIST ('myKey');

-- Search by key
SEARCH_KEY ('myKey');

-- Search by value
SEARCH_VALUE ('myValue');

-- Sort by key
SORT_BY_KEY;

-- Sort by value
SORT_BY_VALUE;
```

### CrabQL Script Syntax

CrabQL scripts are similar to Meson build files, using `#` for comments and single quotes for strings. Here’s an example script:

```ini
# This is a CrabQL script

# Insert commands
insert ('myKey1', 'myValue1');
insert ('myKey2', 'myValue2');

# Update command
update ('myKey1', 'updatedValue1');

# Select command
select ('myKey2');

# Delete command
delete ('myKey1');

# List all entries
list();

# Clear the database
clear();

# Show the current state of the database
show();

# Drop the database
drop();

# Check if a key exists
exist ('myKey2');

# Search for a value
search_value ('updatedValue1');

# Sort by key
sort_by_key();

# Sort by value
sort_by_value();
```

## Contributing and Support

If you're interested in contributing to this project, encounter any issues, have questions, or would like to provide feedback, don't hesitate to open an issue or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information.
