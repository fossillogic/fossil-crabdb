# ***Blue CrabDB Database System by Fossil Logic***

Blue CrabDB is a comprehensive, platform-independent database system designed to streamline the management and storage of key-value pairs within namespaces. Written in C, C++, and Kotlin, Blue CrabDB is built to offer high performance and reliability across various operating systems. The C and C++ components are built using the Meson build system, while the Kotlin components use Gradle, ensuring efficient development and deployment processes.

## Key Features

1. **Cross-Platform Compatibility**: Ensures consistent performance and functionality on Windows, macOS, Linux, and other platforms.
2. **Modular Design**: Offers a collection of modular components that can be easily integrated and customized to meet specific project requirements.
3. **Efficient Build Systems**: Utilizes Meson for C and C++ components and Gradle for Kotlin, providing modern build systems known for their fast build times and user-friendly syntax.
4. **High Performance**: Written in C, C++, and Kotlin to provide optimal performance and resource efficiency, making it suitable for desktop, Android, and embedded applications.
5. **Extensive Documentation and Examples**: Comes with comprehensive documentation and sample code to aid developers in getting started quickly and effectively.

## Prerequisites

Before getting started, make sure you have the following installed:

### For C and C++ Components:

- **Meson Build System**: This project relies on Meson. If you don't have Meson installed, visit the official [Meson website](https://mesonbuild.com/Getting-meson.html) for installation instructions.

### For Kotlin Components:

- **Gradle Build System**: Ensure you have Gradle installed. Visit the official [Gradle website](https://gradle.org/install/) for installation instructions.

## Adding Dependency

### For C and C++ Components

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

### For Kotlin Components

1. **Add the Dependency**: Open your `build.gradle.kts` file and add the dependency for Blue CrabDB:

   ```kotlin
   dependencies {
       implementation("com.fossillogic:fossil-crabdb:0.1.0")
   }
   ```

## Configure Options

### For C and C++ Components

You have options when configuring the build, each serving a different purpose:

- **Running Tests**: To enable running tests, use `-Dwith_test=enabled` when configuring the build.

Example:

```sh
meson setup builddir -Dwith_test=enabled
```

### For Kotlin Components

To run tests in your Kotlin project, ensure you have the following in your `build.gradle.kts` file:

```kotlin
tasks.test {
    useJUnitPlatform()
}
```

Run tests using:

```sh
gradle test
```

## Contributing and Support

If you're interested in contributing to this project, encounter any issues, have questions, or would like to provide feedback, don't hesitate to open an issue or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information.