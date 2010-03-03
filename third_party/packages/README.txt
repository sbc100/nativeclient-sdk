External Native Client SDK Packages

The scripts directory contains bash scripts and patch files to build
common packages for Native Client. The bash scripts will download, patch
build and copy the binary library and developer header files into your
Native Client SDK.

The nacl-install-all.sh script will iterate over all scripts.  This
will download, patch, build and install all the libraries supported by the
SDK.  It is recommended to install all the SDK libraries once using this
higher level script, instead of the individual scripts.

Headers and libraries are installed where nacl-gcc and nacl-g++ will
be able to automatically find them without having to add extra -I or -L
options.  (Currently, these scripts will generate a gcc "specs" file
to add the required extra paths.)

The source code & build out for each package is placed in:

  native_client_sdk/packages/repository

NOTE:  These external libraries each have their own licenses for use.
Please read and understand these licenses before using these packages
in your projects.

NOTE to Windows users:  These scripts are written in bash and must be
launched from a Cygwin shell.
