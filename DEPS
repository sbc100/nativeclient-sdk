vars = {
  "chromium_trunk": "http://src.chromium.org/svn/trunk",
  "chromium_version": "61644",
  "compiler_version": "3431",
  "valgrind_path": "http://nativeclient.googlecode.com/svn/trunk/src/native_client/src/third_party/valgrind",
  "valgrind_version": "3087",
}

deps = {
  "src/third_party/scons":
    Var("chromium_trunk") + "/src/third_party/scons@" + Var("chromium_version"),
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
               "-v", Var("compiler_version")],
  },
  {
    # Install and build all the third-party libs & tools.
    # Make sure this runs after download_compilers.py.
    "pattern": ".",
    "action": ["python", "src/build_tools/install_third_party.py"],
  }
]

