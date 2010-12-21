vars = {
  "native_client_trunk": "http://src.chromium.org/native_client/trunk",
  "native_client_version": "3992",
# NOTE: The version number for build_tools/generate_installers.py needs to have
# |NACL_REVISION| set to match |native_client_version|
  "x86_toolchain_version": "3992",
  "valgrind_path": "svn://svn.chromium.org/native_client/trunk/src/native_client/src/third_party/valgrind",
  "valgrind_version": "3860",
}
# When we have ARM and PNaCl support, we'll need to add toolchain support
# for those too.

deps = {
  "src/third_party/scons":
    Var("native_client_trunk") + "/src/third_party/scons@" + Var("native_client_version"),
  "src/third_party/swtoolkit":
    "http://swtoolkit.googlecode.com/svn/trunk@66",
  "src/third_party/valgrind": Var("valgrind_path") + "@" +
       Var("valgrind_version"),
}

hooks = [
  {
    # Grab the NaCl toolchain.  This action has to happen before the testing
    # libraries are installed, and before boost gets installed.
    "pattern": ".",
    "action": ["python", "src/build_tools/download_compilers.py",
               "-v", Var("x86_toolchain_version")],
  },
  {
    # Install and build all the third-party libs & tools.
    # Make sure this runs after download_compilers.py.
    "pattern": ".",
    "action": ["python", "src/build_tools/install_third_party.py"],
  }
]

