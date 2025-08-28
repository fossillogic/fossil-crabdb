# ***Blue Crab Database by Fossil Logic***

CrabDB is a lightweight, portable key-value database designed to offer fast and efficient data storage in a user-friendly format. Available in two distinct interfaces—**MyShell** and **NoShell**—CrabDB tailors its experience to fit various use cases, from those who prefer a SQL-like syntax to those who prefer raw, key-based access. Its book-inspired structure, where each entry is akin to a page in a book, makes managing key-value pairs intuitive and easy to understand, while ensuring high performance across both small and large datasets.

## Key Features

| **Feature**                     | **MyShell**                                                                                                                                   | **NoShell**                                                                                                                                |
|----------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| **Portability & Cross-Platform** | Works across different operating systems, offering seamless performance with minimal dependencies.                                             | Works across various platforms without external dependencies, ensuring smooth performance.                                               |
| **Interface**                    | SQL-like command interface, ideal for users familiar with relational databases.                                                               | Direct key-value CRUD operations with a minimalistic approach, no query language required.                                                |
| **CRUD Operations**              | Insert, find, update, and delete key-value pairs with SQL-like commands.                                                                       | Perform CRUD operations using direct key-value access.                                                                                   |
| **Backup and Restore**           | Supports backup and restore features using MyShell's command-line interface.                                                                   | Allows for easy backup and restore of database files with no query overhead.                                                              |
| **API**                          | Intuitive API designed for SQL-like interaction.                                                                                              | Simple API for minimalistic key-value operations.                                                                                        |
| **Memory Management**            | Optimized for low memory usage and fast access, even with large datasets.                                                                     | Ensures efficient memory usage and performance, even with direct key-value access.                                                        |
| **Database Management**          | Full support for creating, opening, closing, and deleting databases, as well as managing backups.                                               | Supports similar database management operations with a direct approach, including backups and restores.                                   |

## Prerequisites

To get started with Fossil CrabDB, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-crabdb')` in `meson.build` so Meson can fetch and build it automatically.

#### Adding via Conan GitHub repository

 packages directly from a GitHub repository if it contains a valid `conanfile.py`.

```bash
conan install git+https://github.com/fossillogic/fossil-crabdb.git#v0.2.4 --name fossil_crabdb --build=missing
```

#### Integrate the Dependency:

Add the `fossil-crabdb.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-crabdb.git
revision = v0.2.4

[provide]
dependency_names = fossil-crabdb
```

**Note**: For the best experience, always use the latest releases. Visit the [releases](https://github.com/fossillogic/fossil-crabdb/releases) page for the latest versions.

## Configure Options

You have options when configuring the build, each serving a different purpose:

- **Running Tests**: To enable running tests, use `-Dwith_test=enabled` when configuring the build.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

### Tests Double as Samples

The project is designed so that **test cases serve two purposes**:

- ✅ **Unit Tests** – validate the framework’s correctness.  
- 📖 **Usage Samples** – demonstrate how to use these libraries through test cases.  

This approach keeps the codebase compact and avoids redundant “hello world” style examples.  
Instead, the same code that proves correctness also teaches usage.  

This mirrors the **Meson build system** itself, which tests its own functionality by using Meson to test Meson.  
In the same way, Fossil Logic validates itself by demonstrating real-world usage in its own tests via Fossil Test.  

```bash
meson test -C builddir -v
```

Running the test suite gives you both verification and practical examples you can learn from.

## Contributing and Support

If you're interested in contributing to this project, encounter any issues, have questions, or would like to provide feedback, don't hesitate to open an issue or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information.
