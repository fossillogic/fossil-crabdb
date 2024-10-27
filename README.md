# ***Blue Crab Database System by Fossil Logic***

Blue CrabDB is a comprehensive, platform-independent data ecosystem crafted to streamline the management, storage, and intelligent processing of key-value pairs within namespaces. Developed in C and C++, it provides high performance and reliability across various operating systems. At its core is **CrabDB**, a powerful database engine, enhanced by **CrabQL**, a SQL-like query language designed for precise data retrieval and manipulation. The suite also includes **CrabSearch** for fast, relevant full-text search, and **CrabAI** for AI-driven analytics that yield predictive insights and smarter data handling. Additionally, **CrabLogs** offers robust logging for secure transaction tracking, while **CrabSync** ensures reliable synchronization across multiple instances, maintaining data consistency across environments. Altogether, Blue CrabDB’s modules deliver a unified, advanced solution, making it ideal for applications requiring speed, reliability, and sophisticated data capabilities.

# Key Features

1. **Cross-Platform Compatibility**: Ensures consistent performance and functionality across Windows, macOS, Linux, and other major platforms, making it versatile for a wide range of applications.
2. **Modular Design**: Features a collection of powerful components, including **CrabDB**, **CrabQL**, **CrabSearch**, **CrabAI**, **CrabLogs**, **CrabSync**, and more. Each module can be easily integrated and customized to meet specific project needs, offering flexibility for different use cases.
3. **Efficient Build Systems**: Utilizes the Meson build system for C and C++ components, enabling fast build times and a user-friendly syntax that enhances the developer experience.
4. **High Performance**: Written in C and C++ for optimized performance and resource efficiency, making Blue CrabDB ideal for high-demand desktop and embedded applications.
5. **Extensive Documentation and Examples**: Accompanied by comprehensive documentation and sample code for each module, simplifying the development process and accelerating the onboarding of new users.

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
