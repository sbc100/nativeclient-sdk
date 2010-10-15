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
    "pattern": ".",
    # Grab the NaCl toolchain
    "action": ["python", "src/build_tools/download_compilers.py",
               "-v", Var("compiler_version")],
    # Install GMock and GTest.
    "action": ["python", "src/build_tools/install_gtest.py"],
  }
]

