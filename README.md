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
   revision = v0.2.1

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
