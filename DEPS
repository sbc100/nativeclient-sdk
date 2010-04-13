vars = {
  "compiler_version": "latest",
  "chromium_trunk": "http://src.chromium.org/svn/trunk/",
  # Note: make sure this is the same rev as the one used in native_client.
  "chromium_version": "42751",
}

deps = {
  # Pull in the trusted plugin headers from Chromium.
  # TODO(dspringer): remove these once we no longer need to build trusted
  # plugins for debugging.
  "src/third_party/npapi/bindings": Var("chromium_trunk") +
      "src/third_party/npapi/bindings@" + Var("chromium_version"),
  "src/third_party/include/GLES2": Var("chromium_trunk") +
      "src/gpu/GLES2@" + Var("chromium_version"),
  "src/third_party/include/KHR": Var("chromium_trunk") +
      "src/gpu/KHR@" + Var("chromium_version"),
  "src/third_party/include/pgl": Var("chromium_trunk") +
      "src/gpu/pgl@" + Var("chromium_version"),
  # The trusted plugins have to build a GPU client for rendering.
  # TODO(dspringer): remove these once we can debug .nexes directly.
  "src/third_party/include/base": Var("chromium_trunk") +
      "src/base@" + Var("chromium_version"),
  "src/third_party/include/build": Var("chromium_trunk") +
      "src/build@" + Var("chromium_version"),
  "src/third_party/gpu/command_buffer/common": Var("chromium_trunk") +
      "src/gpu/command_buffer/common@" + Var("chromium_version"),
  "src/third_party/gpu/command_buffer/client": Var("chromium_trunk") +
      "src/gpu/command_buffer/client@" + Var("chromium_version"),
  "src/third_party/gpu/pgl": Var("chromium_trunk") +
      "src/gpu/pgl@" + Var("chromium_version"),
}

hooks = [
  {
    "pattern": ".",
    "action": ["python", "src/build_tools/download_compilers.py",
               "-v", Var("compiler_version")],
  }
]

