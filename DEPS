vars = {
  "compiler_version": "3431",
  "valgrind_path": "http://nativeclient.googlecode.com/svn/trunk/src/native_client/src/third_party/valgrind",
  "valgrind_version": "3087",
}

deps = {
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
    # Install GMock and GTest.
    # Make sure this runs after download_compilers.py.
    "pattern": ".",
    "action": ["python", "src/build_tools/install_gtest.py"],
  }
]

