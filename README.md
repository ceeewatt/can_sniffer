# CAN Sniffer

This a (WIP) CAN/J1939 bus monitoring tool using, built using the Qt CAN Bus API and my custom [J1939](https://github.com/ceeewatt/mini_j1939) implementation. If you're using a CAN device supported by one of [Qt's CAN Bus Plugins](https://doc.qt.io/qt-6/qtcanbus-backends.html#can-bus-plugins), this tool can be used to monitor CAN bus traffic, and if a DBC file is provided, the relevant signals will be decoded and displayed. Presently, only the Linux SocketCAN plugin is supported.

Sample DBC files can be found from: https://github.com/nberlette/canbus

# Build Instructions

This project uses some experimental Qt APIs for CAN DBC parsing, which requires Qt 6.5+. At the time of writing, I'm on Qt 6.9.

Once the proper version of Qt is installed and this repo is cloned, you can configure and build the CMake project as usual. Just be sure to somehow set the `CMAKE_PREFIX_PATH` variable to the Qt installation directory - this is necessary for CMake to find the Qt packages. A typical installation path on Linux would be something like: `~/Qt/6.9.1/gcc_64`.

For example:

```sh
git clone --recurse-submodules https://github.com/ceeewatt/can_sniffer.git
cd can_sniffer
mkdir build
cmake -S . -B build -DCMAKE_PREFIX_PATH="~/Qt/6.9.1/gcc_64"
cmake --build build
```

To demo the project using SocketCAN, you can execute the provided shell script (`./virtual_can.sh`) and generate CAN frames using a utility like `cansend` or `cangen`.
