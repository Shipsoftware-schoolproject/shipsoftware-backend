# ShipSoftware-backend

ShipSoftware-backend was a school project which was worked on in couple
different course in VAMK on 2017-2018.

Initial version of this project was made with C# but was then re-written in C to
get a grasp of cross-platform development. In re-write a full documentation
with Doxygen was added.

This program is just a dummy "middleman" between aprs.fi API and database and it
does not provide any other functionality than automatically getting data from
the API and inserting it to database. This program just gives a purpose for
the [shipsoftware-chsarp](https://github.com/Shipsoftware-schoolproject/shipsoftware-csharp)
and a [shipsoftware-php](https://github.com/Shipsoftware-schoolproject/shipsoftware-php),
by providing new data to the database; otherwise those projects would not
have much data to display.

Keep in mind that this and the other projects in [Shipsoftware-schoolproject](https://github.com/Shipsoftware-schoolproject)
group are just a school projects. They may not suit for production use and the
use cases of these programs might be very marginal; you would get more
information about the ships by just using the [aprs.fi](https://aprs.fi/) site
itself or any other similar site.

## Requirements

 * Valid API key from [aprs.fi](https://aprs.fi).
 * MySQL database
  * Use MySQL clauses from [shipsoftware-sql](https://github.com/Shipsoftware-schoolproject/shipsoftware-sql/tree/master/MySQL)
  repository to setup the database correctly.

API key is needed to fetch information of ships and database obliviously is
for storing the data.

## Configuration

Configuration is stored in `configuration.json` in JSON format. It will be
loaded from the same directory as the executable is started from.

See `configuration.json.example` for example configuration. Or if you're reading
Doxygen documentation, check [Configuration file](@ref ConfigPage) page.

`log_size` option is not necessary, it is used only by the GUI and it will
default to `20` if it is not provided.

## Documentation

Documentation can be generated with Doxygen. `doxygen.conf` which comes with
the project has been configured to generate documentation only in HTML format.

To generate documentation along with the build, see `CMake flags` section
for the flag to trigger generation of documentation.

## Contributing

See [Contributing](CONTRIBUTING.md).

## Dependencies

### Run time
 * libmariadb2
 * libjson-glib-1.0
 * gtk3 (required only for GUI build)

### Build
 * gcc
 * make
 * cmake
 * libjson-glib-dev
 * libmariadbclient-dev
 * libcurl-dev
 * libgtk-3-dev (required only for GUI build)
 * doxygen (optional)
#### For cross-compiling for Windows
 * python3
 * binutils (objdump)

## Building

#### CMake flags

 * `BUILD_DOC` set to `1` to build documentation (default 0).
 * `WITH_GUI` set to `0` to build without GTK (default 1).

### Linux

Compiling is simple as:
 * `mkdir build`
 * `cd build`
 * `cmake ..`
 * `make`

### MinGW cross-compile for Windows

Should be simple as:
 * `mkdir build`
 * `cd build`
 * `x86_64-w64-mingw32-cmake ..`
 * `make`

`*-cmake` might vary between distros.

All required DLL's are copied automatically next to .exe file with [mingw-bundledlls](https://github.com/mpreisler/mingw-bundledlls)
script when you execute `make`. You might need to adjust the paths in
`mingw-bundledlls` script regarding to your distro.

`mingw-gtktheme` script is also called by `make` to set default theme so the
program looks more native in Windows.

### Windows
 * Never tried on Windows..
