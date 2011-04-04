# Note: NACL_REVISION is used by build_tools/generate_installers.py
NACL_REVISION = "4528"

vars = {
  "native_client_trunk": "http://src.chromium.org/native_client/trunk",
  "native_client_version": NACL_REVISION,
  "x86_toolchain_version": NACL_REVISION,
  "valgrind_path": "http://src.chromium.org/native_client/trunk/"
    "src/native_client/src/third_party/valgrind",
  "valgrind_version": NACL_REVISION,
  "pymox": "http://pymox.googlecode.com/svn/trunk",
  "pymox_version": "61",
  "chromium_trunk": "http://src.chromium.org/svn/trunk",
  "chromium_version": "78322",
}
# When we have ARM and PNaCl support, we'll need to add toolchain support
# for those too.

deps = {
  "src/third_party/scons":
    Var("native_client_trunk") + "/src/third_party/scons@"
    + Var("native_client_version"),
  "src/third_party/swtoolkit":
    "http://swtoolkit.googlecode.com/svn/trunk@66",
  "src/third_party/valgrind": Var("valgrind_path") + "@" +
    Var("valgrind_version"),
  "src/third_party/pymox": Var("pymox") + "@" + Var("pymox_version"), 
  "src/experimental/visual_studio_plugin/third_party/nacl_dir/native_client/"
    "src/shared/gio":
    Var("native_client_trunk") + "/src/native_client/src/shared/gio@"
    + Var("native_client_version"),          
  "src/experimental/visual_studio_plugin/third_party/nacl_dir/native_client/"
    "src/shared/platform":
    Var("native_client_trunk") + "/src/native_client/src/shared/platform@"
    + Var("native_client_version"),        
  "src/experimental/visual_studio_plugin/third_party/nacl_dir/native_client/"
    "src/trusted/debug_stub":
    Var("native_client_trunk") + "/src/native_client/src/trusted/debug_stub@"
    + Var("native_client_version"),      
  "src/experimental/visual_studio_plugin/third_party/nacl_dir/native_client/"
    "src/trusted/service_runtime/include":
    Var("native_client_trunk")
    + "/src/native_client/src/trusted/service_runtime/include@"
    + Var("native_client_version"),    
  "src/experimental/visual_studio_plugin/third_party/nacl_dir/native_client/"
    "src/include":
    Var("native_client_trunk") + "/src/native_client/src/include@"
    + Var("native_client_version"),  
  "src/third_party/ppapi": Var("chromium_trunk") + "/src/ppapi" +
    "@" + Var("chromium_version"),
  "src/experimental/visual_studio_plugin/third_party/nacl_sub/native_client/"
    "src/trusted/port":
    Var("native_client_trunk") + "/src/native_client/src/trusted/port@"
    + Var("native_client_version"),
  "src/experimental/visual_studio_plugin/third_party/nacl_sub/native_client/"
    "src/trusted/gdb_rsp":
    Var("native_client_trunk") + "/src/native_client/src/trusted/gdb_rsp@"
    + Var("native_client_version")
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

