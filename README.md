# ***Blue Crab by Fossil Logic***

Blue Crab is a lightweight, portable key-value database built for speed, efficiency, and ease of use. It offers multiple interfaces to fit different workflows: **MyShell** for SQL-like commands, **NoShell** for direct key-value operations, **TimeShell** for timestamped time-series data, **CacheShell** for in-memory caching with TTL support, and **FileShell** for straightforward file management. Its book-inspired structure, where each entry resembles a page, makes organizing and accessing data intuitive, while maintaining high performance across both small and large datasets.

## Key Features

| **Feature**                     | **MyShell**                                                                                                                                   | **NoShell**                                                                                                                                | **TimeShell**                                                                                                                           | **CacheShell**                                                                                                                        | **FileShell**                                                                                                                      |
|---------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| **Portability & Cross-Platform** | Works across different operating systems, offering seamless performance with minimal dependencies.                                             | Works across various platforms without external dependencies, ensuring smooth performance.                                               | Lightweight and portable, optimized for time-series storage with minimal dependencies.                                                 | Highly portable in-memory caching, works on multiple platforms with minimal overhead.                                              | Portable file-based storage, works across OSes without external dependencies.                                                    |
| **Interface**                    | SQL-like command interface, ideal for users familiar with relational databases.                                                               | Direct key-value CRUD operations with a minimalistic approach, no query language required.                                                | Time-series oriented interface, supports timestamped insertions and queries.                                                           | Key-value interface optimized for fast retrieval and TTL management.                                                               | Simple file read/write/append/delete interface for persistent storage.                                                           |
| **CRUD Operations**              | Insert, find, update, and delete key-value pairs with SQL-like commands.                                                                       | Perform CRUD operations using direct key-value access.                                                                                   | Insert, query, and delete timestamped records efficiently.                                                                            | Insert, get, update, and remove cache entries with optional TTL.                                                                  | Write, append, read, and delete files directly.                                                                                  |
| **Backup and Restore**           | Supports backup and restore features using MyShell's command-line interface.                                                                   | Allows for easy backup and restore of database files with no query overhead.                                                              | Supports backup and restore of time-series data files.                                                                                | Optional persistence to disk can be implemented; otherwise in-memory only.                                                        | Files are directly persistent; backup and restore can be done using file copy operations.                                        |
| **API**                          | Intuitive API designed for SQL-like interaction.                                                                                              | Simple API for minimalistic key-value operations.                                                                                        | API provides timestamp-aware insertions, queries, and iteration.                                                                      | Simple, fast API for cache access and TTL operations.                                                                             | Straightforward API for basic file manipulation.                                                                                 |
| **Memory Management**             | Optimized for low memory usage and fast access, even with large datasets.                                                                     | Ensures efficient memory usage and performance, even with direct key-value access.                                                        | Optimized for sequential time-series data; minimal memory overhead per record.                                                         | In-memory store, optimized for fast access and efficient memory usage; supports TTL cleanup.                                       | Efficient for file-based operations; memory usage depends on file buffering strategy.                                            |
| **Database Management**          | Full support for creating, opening, closing, and deleting databases, as well as managing backups.                                               | Supports similar database management operations with a direct approach, including backups and restores.                                   | Manage time-series databases including open/close and backup/restore of timestamped datasets.                                         | Simple in-memory key-value store management; clear, count, and expire operations available.                                        | Provides basic file management operations: check existence, size, list, and delete files.                                       |

## ***Prerequisites***

To get started, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson `1.8.0` or newer installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).
- **Conan Package Manager**: If you prefer using Conan, ensure it is installed by following the instructions on the official [Conan website](https://docs.conan.io/en/latest/installation.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-crabdb')` in `meson.build` so Meson can fetch and build it automatically.

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
