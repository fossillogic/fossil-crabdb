# ***Blue Crab by Fossil Logic***

Blue Crab is a lightweight, portable key-value database built for speed, efficiency, and traceable data integrity.   It offers multiple interfaces to fit different workflows: **MyShell** for SQL-like commands with structured FSON queries, **NoShell** for direct key-value operations backed by a git-chain commit model, and **CacheShell** for in-memory caching with TTL and optional FSON serialization. Powered by the **git-chain experiment**, every change in Blue Crab can be versioned, branched, and merged like source code —  
enabling transparent history, rollback, and auditability. The integrated **FSON type system** provides self-describing, schema-aware data structures for both simple and complex records.  

## Key Features

| **Feature**                     | **MyShell**                                                                                                                                         | **NoShell**                                                                                                                                                     | **CacheShell**                                                                                                                                    |
|---------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| **Portability & Cross-Platform** | Fully cross-platform, designed for integration across Linux, macOS, and Windows with minimal dependencies.                                          | Portable and dependency-light, works consistently across OSes and embedded systems.                                                                              | Lightweight and portable; in-memory cache functions identically across supported platforms.                                                       |
| **Interface**                    | SQL-like command interface with support for structured FSON queries and schema reflection.                                                          | Minimalist key-value interface now extended with FSON-based structured records and git-chain commit tracking.                                                    | Key-value and TTL-based interface, FSON-encoded for cross-shell interoperability.                                                                 |
| **CRUD Operations**              | Insert, find, update, and delete data using SQL-like commands, with git-chain-backed transactional history.                                          | Perform direct CRUD operations with git-chain commit tracking and FSON record serialization for full auditability.                                               | Insert, get, update, and remove cache entries; optionally commit snapshots into git-chain for audit persistence.                                  |
| **Backup and Restore**           | Backup and restore now handled via git-chain commits, enabling branchable database states and rollbacks.                                            | Supports backup and restore through git-chain synchronization; each commit stores FSON diffs for efficient version tracking.                                     | Optional persistence through git-chain snapshots; supports branch-based cache state restoration.                                                  |
| **API**                          | Intuitive C API supporting FSON object handling, commit/branch operations, and query execution layers.                                              | Simple C API for direct CRUD and commit operations; includes git-chain and FSON helper utilities for structured data encoding/decoding.                          | Lightweight API for in-memory caching and TTL; includes optional FSON serialization hooks for structured cache objects.                           |
| **Memory Management**             | Optimized for large datasets with internal FSON caching and incremental git-chain commit diffing to reduce memory footprint.                        | Memory-efficient design; uses incremental FSON serialization and diff-based commit tracking to minimize data duplication.                                         | Efficient in-memory storage with periodic cleanup; supports TTL-based reclamation and optional FSON compression for cache entries.                |
| **Database Management**          | Full support for create, open, close, branch, merge, and delete operations via git-chain; each state stored as a structured FSON snapshot.            | Supports open, close, sync, and merge of git-chain-backed databases; provides FSON-based schema introspection and consistency checks.                            | Simple cache lifecycle management with optional git-chain commit/merge support for replicable cache state management.                             |

## ***Prerequisites***

To get started, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson `1.8.0` or newer installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-crabdb')` in `meson.build` so Meson can fetch and build it automatically.

#### Integrate the Dependency:

Add the `fossil-crabdb.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-crabdb.git
revision = v0.2.5

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
