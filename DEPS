vars = {
  "compiler_version": "2939",
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
    "action": ["python", "src/build_tools/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

