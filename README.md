# ***Blue CrabDB Database System by Fossil Logic***

Blue CrabDB is a comprehensive, platform-independent database system designed to streamline the management and storage of key-value pairs within namespaces. Written in C and C++, Blue CrabDB is built to offer high performance and reliability across various operating systems. The C and C++ components are built using the Meson build system, ensuring efficient development and deployment processes.

## Key Features

1. **Cross-Platform Compatibility**: Ensures consistent performance and functionality on Windows, macOS, Linux, and other platforms.
2. **Modular Design**: Offers a collection of modular components that can be easily integrated and customized to meet specific project requirements.
3. **Efficient Build Systems**: Utilizes Meson for C and C++ components, providing a modern build system known for its fast build times and user-friendly syntax.
4. **High Performance**: Written in C and C++ to provide optimal performance and resource efficiency, making it suitable for desktop and embedded applications.
5. **Extensive Documentation and Examples**: Comes with comprehensive documentation and sample code to aid developers in getting started quickly and effectively.

## Blue CrabDB Query Language Examples

### Sample 1: CrabQL Commands (SQL-like Syntax)

This example demonstrates how to use CrabQL commands to interact with the Blue CrabDB database. The commands are similar to SQL and perform basic operations such as creating namespaces, setting key-value pairs, and retrieving values.

```sql
-- Create a namespace called "users"
CREATE NAMESPACE "users";

-- Set a key-value pair in the "users" namespace
SET "users" key1 "value1";

-- Update a key-value pair in the "users" namespace
UPDATE "users" key1 "new_value1";

-- Get the value of a key from the "users" namespace
GET "users" key1;

-- Delete a key-value pair from the "users" namespace
DELETE "users" key1;

-- Delete the "users" namespace
DELETE NAMESPACE "users";

-- Print a message
PRINT "This is a test message.";

-- Export the database to a file
EXPORT "backup.crabdb";

-- Import data from a file
IMPORT "restore.crabdb";

```

### Sample 2: CrabQL Script (Meson-like Syntax)

This example shows how to use CrabQL scripts with a syntax resembling Meson build scripts. It includes comments, conditional logic, and operations within a script file.

```ini
# This script creates a namespace, sets key-value pairs, and performs conditional operations

# Create a new namespace called 'default_ns'
create_namespace(name=default_ns)

# Set key-value pairs in the 'default_ns' namespace
set(namespace=default_ns, key=my_key, value=my_value)
set(namespace=default_ns, key=another_key, value=another_value)

# Retrieve and check the value of 'my_key'
if (get(namespace=default_ns, key=my_key) == my_value)
    # Condition met, set another key-value pair
    set(namespace=default_ns, key=third_key, value=third_value)
else
    # Condition not met, delete the key 'my_key'
    delete_key(namespace=default_ns, key=my_key)
end

# Loop through a list of keys and print their values
foreach key in [my_key, another_key]
    # Retrieve and print the value for each key
    value = get(namespace=default_ns, key=key)
    print(value)
end
```

### Explanation

- **Commands (SQL-like)**: Use commands similar to SQL statements to perform operations on the database. Each command ends with a semicolon, and operations are straightforward and resemble traditional SQL syntax.
  
- **Scripts (Meson-like)**: Utilize a script-like syntax with comments, conditional statements, and loops. The syntax is designed to be more flexible and readable, resembling Meson build system scripts. Comments start with `#`, and control structures such as `if`, `else`, and `foreach` allow for more dynamic interactions.

These samples illustrate the flexibility of the CrabQL language, providing both SQL-like commands for simple queries and Meson-like scripts for more complex workflows.

### CrabDB presistent data

Got it. It seems like you need a clear format for `.crabdb` files that includes various data types with examples. Here's how you might structure a `.crabdb` file to accommodate the different types:

### Example `.crabdb` File

```ini
[namespace1]
key1=u8:42
key2=i16:-123
key3=h32:0x1A2B3C
key4=f64:3.14159
key5=cstr:Hello, World!
key6=bool:true
key7=char:A
key8=o32:012345
key9=u64:18446744073709551615

[namespace2]
another_key=i32:123456789
some_hex=h64:0x1234567890ABCDEF
float_key=f32:2.71828
```

### Data Type Definitions

1. **Unsigned Integers:**
   - `u8`: 8-bit unsigned integer (0 to 255)
   - `u16`: 16-bit unsigned integer (0 to 65,535)
   - `u32`: 32-bit unsigned integer (0 to 4,294,967,295)
   - `u64`: 64-bit unsigned integer (0 to 18,446,744,073,709,551,615)

2. **Signed Integers:**
   - `i8`: 8-bit signed integer (-128 to 127)
   - `i16`: 16-bit signed integer (-32,768 to 32,767)
   - `i32`: 32-bit signed integer (-2,147,483,648 to 2,147,483,647)
   - `i64`: 64-bit signed integer (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)

3. **Hexadecimal Values:**
   - `h8`: 8-bit hexadecimal number
   - `h16`: 16-bit hexadecimal number
   - `h32`: 32-bit hexadecimal number
   - `h64`: 64-bit hexadecimal number

4. **Octal Values:**
   - `o8`: 8-bit octal number
   - `o16`: 16-bit octal number
   - `o32`: 32-bit octal number
   - `o64`: 64-bit octal number

5. **Floating-Point Numbers:**
   - `f32`: 32-bit floating-point number
   - `f64`: 64-bit floating-point number

6. **String Values:**
   - `cstr`: C-style string

7. **Boolean Values:**
   - `bool`: Boolean (`true` or `false`)

8. **Character Values:**
   - `char`: Single character

### File Extension

- `.crabdb` is used for database files that store key-value pairs in this format.

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
   revision = v0.1.0

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

## Contributing and Support

If you're interested in contributing to this project, encounter any issues, have questions, or would like to provide feedback, don't hesitate to open an issue or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information.
