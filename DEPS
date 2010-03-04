vars = {
  "compiler_version": "2010_03_03",
  "chromium_trunk": "http://src.chromium.org/svn/trunk/",
  "hammer_trunk": "http://swtoolkit.googlecode.com/svn/trunk/",
  "chromium_rev": "39744",
}

deps = {
  # Get the hammer tools.
  "src/tools/hammer": Var("hammer_trunk"),
  # Pull in the GLES2 example code from the Chromium trunk.
  "src/third_party/gles2_book":
    Var("chromium_trunk") + "src/third_party/gles2_book@" + Var("chromium_rev"),
}

hooks = [
  {
    "pattern": "DEPS$",
    "action": ["python", "src/scripts/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

