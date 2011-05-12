# Note: NACL_REVISION is used by build_tools/generate_installers.py
NACL_REVISION = "5201"
TOOLCHAIN_REVISION = "5188"

vars = {
  "native_client_trunk": "http://src.chromium.org/native_client/trunk",
  "native_client_version": NACL_REVISION,
  "x86_toolchain_version": TOOLCHAIN_REVISION,
  "valgrind_version": NACL_REVISION,
  "pymox": "http://pymox.googlecode.com/svn/trunk",
  "pymox_version": "61",
  "chromium_trunk": "http://src.chromium.org/svn/trunk",
  "chromium_version": "79704",
  # Grab the SCons build stuff from the NaCl repo.
  "scons_trunk": "http://src.chromium.org/native_client/trunk/",
  "scons_version": NACL_REVISION,
  "gtest_rev": "267",
}
# When we have ARM and PNaCl support, we'll need to add toolchain support
# for those too.

deps = {
  "src/third_party/scons-2.0.1":
    Var("scons_trunk") + "src/third_party/scons-2.0.1@" + Var("scons_version"),
  "src/site_scons":
    Var("scons_trunk") + "src/native_client/site_scons@" + Var("scons_version"),
  "src/third_party/valgrind": Var("native_client_trunk") +
    "/src/native_client/src/third_party/valgrind" + "@" +
    Var("valgrind_version"),
  "src/third_party/gtest":
    "http://googletest.googlecode.com/svn/trunk@" + Var("gtest_rev"),
  "src/third_party/valgrind/bin": Var("native_client_trunk") +
    "/src/third_party/valgrind/bin" + "@" + Var("valgrind_version"),
  "src/third_party/pymox": Var("pymox") + "@" + Var("pymox_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/shared/gio":
    Var("native_client_trunk") + "/src/native_client/src/shared/gio@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/shared/platform":
    Var("native_client_trunk") + "/src/native_client/src/shared/platform@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/trusted/debug_stub":
    Var("native_client_trunk") + "/src/native_client/src/trusted/debug_stub@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/trusted/service_runtime/include":
    Var("native_client_trunk")
    + "/src/native_client/src/trusted/service_runtime/include@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/include":
    Var("native_client_trunk") + "/src/native_client/src/include@"
    + Var("native_client_version"),
  "src/third_party/ppapi": Var("chromium_trunk") + "/src/ppapi" +
    "@" + Var("chromium_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/trusted/port":
    Var("native_client_trunk") + "/src/native_client/src/trusted/port@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/native_client/"
    "src/trusted/gdb_rsp":
    Var("native_client_trunk") + "/src/native_client/src/trusted/gdb_rsp@"
    + Var("native_client_version"),
}

hooks = [
  {
    # Grab the NaCl toolchain.  This action has to happen before the testing
    # libraries are installed, and before boost gets installed.
    "pattern": ".",
    "action": ["python", "src/build_tools/download_compilers.py",
               "-v", Var("x86_toolchain_version")],
  },
]

