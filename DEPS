vars = {
  "compiler_version": "2010_03_03",
}

deps = {
}

hooks = [
  {
    "pattern": "DEPS$",
    "action": ["python", "src/scripts/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

