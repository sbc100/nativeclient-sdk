vars = {
  "compiler_version": "latest",
  "chromium_trunk": "http://src.chromium.org/svn/trunk/",
  "hammer_trunk": "http://swtoolkit.googlecode.com/svn/trunk/",
  "nixysa_trunk": "http://nixysa.googlecode.com/svn/trunk/",
}

deps = {
  # Get the hammer tools.
  "src/tools/hammer": Var("hammer_trunk"),
  "src/tools/nixysa": Var("nixysa_trunk"),
  # Pull in the base headers from Chromium, this is for smart pointers.
  "src/third_party/include/base": Var("chromium_trunk") + "src/base",
}

hooks = [
  {
    "pattern": "DEPS$",
    "action": ["python", "src/scripts/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

