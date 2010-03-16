vars = {
  "compiler_version": "2010_03_16",
  "chromium_trunk": "http://src.chromium.org/svn/trunk/",
  "hammer_trunk": "http://swtoolkit.googlecode.com/svn/trunk/",
  "nixysa_trunk": "http://nixysa.googlecode.com/svn/trunk/",
}

deps = {
  # Get the hammer, SCons and nixysa tools.
  "src/tools/hammer": Var("hammer_trunk"),
  "src/tools/scons": Var("chromium_trunk") + "src/third_party/scons",
  "src/tools/nixysa": Var("nixysa_trunk"),
  # Pull in the trusted plugin headers from Chromium.
  # TODO(dspringer): remove these once we no longer need to build trusted
  # plugins for debugging.
  "src/third_party/npapi/bindings": Var("chromium_trunk") +
      "src/third_party/npapi/bindings",
  "src/third_party/include/GLES2": Var("chromium_trunk") + "src/gpu/GLES2",
  "src/third_party/include/KHR": Var("chromium_trunk") + "src/gpu/KHR",
  "src/third_party/include/pgl": Var("chromium_trunk") + "src/gpu/pgl",
  # The trusted plugins have to build a GPU client for rendering.
  # TODO(dspringer): remove these once we can debug .nexes directly.
  "src/third_party/include/base": Var("chromium_trunk") + "src/base",
  "src/third_party/include/build": Var("chromium_trunk") + "src/build",
  "src/third_party/gpu/command_buffer/common": Var("chromium_trunk") +
      "src/gpu/command_buffer/common",
  "src/third_party/gpu/command_buffer/client": Var("chromium_trunk") +
      "src/gpu/command_buffer/client",
  "src/third_party/gpu/pgl": Var("chromium_trunk") + "src/gpu/pgl",
}

hooks = [
  {
    "pattern": "DEPS$",
    "action": ["python", "src/scripts/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

