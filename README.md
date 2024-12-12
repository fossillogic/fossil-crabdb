# ***Blue Crab Database by Fossil Logic***

CrabDB is a lightweight, portable key-value database designed to provide fast and efficient data storage in a simple, easy-to-use format. Inspired by the metaphor of a book, CrabDB organizes data as pages in a book, where each page represents an entry in the database. This structure makes it intuitive to understand and interact with, while maintaining high performance for both small and large datasets.

## Key Features:
- **Portable**: Designed to be platform-independent, CrabDB works seamlessly across different operating systems, allowing you to store and manage data with minimal dependencies.
- **Book-Like Structure**: The database's organization is inspired by a book, where each entry is like a page in a book. This simple, yet powerful structure makes it easy to conceptualize and manage key-value pairs.
- **Search and Lookup**: Quickly search and retrieve values by key with efficient lookup operations, making it ideal for use in a wide range of applications, from small utilities to larger systems.
- **Simple API**: The database API is designed to be straightforward and easy to use, with a focus on simplicity and readability. Whether you're storing data for a small project or a larger system, CrabDB's API is intuitive enough for anyone to get started quickly.
- **Efficient Memory Management**: With a focus on performance, CrabDB minimizes memory usage while maintaining fast access times and stable behavior, even with a growing dataset.

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
   revision = v0.2.0

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
