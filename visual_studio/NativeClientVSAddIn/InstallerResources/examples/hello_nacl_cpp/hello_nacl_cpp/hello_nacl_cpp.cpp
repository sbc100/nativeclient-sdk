// This project demonstrates how to migrate a Windows desktop app to Native
// Client, running first as a Win32 application (define STEP1), then as a PPAPI
// plugin (define STEP2 through STEP6), and finally as a Native Client module.

// Start with STEP1 defined and the defines for STEP2 through STEP6 commented
// out.  For each step in the process, un-comment the next #define, leaving the
// previous ones on.  Ready, set, port!

// *** SELECT THE WIN32 PLATFORM AND RUN WITH #define STEP1 ONLY ***

#define STEP1
// Launches the original Windows desktop application, Hello World, which runs
// as WinMain.  STEP1 encloses Windows-specific functions that are used with
// the WIN32 and PPAPI platforms.  These will be removed when the full PPAPI
// port is finished (STEP6)

// *** SELECT THE PPAPI PLATFORM ***

#define STEP2

// Client Module.  STEP2 encloses the Native Client module APIs needed to link
// any app to the browser.  The module does nothing except report
// starting/ending the function Instance_DidCreate.  The Windows app does not
// run because it is not being called.

#define STEP3
// What changed: Replace WinMain with WndProc, and call it from
// Instance_DidCreate, launching hello_nacl_plus in its own window. Since
// WndProc spins in its message loop, the call to Instance_DidCreate never
// returns. Close the hello_nacl_plus window and the module initialization will
// finish.

#define STEP4
// What changed: In WndProc replace the message loop with a callback function.
// Now the app window and the Native Client module are running concurrently.

#define STEP5
// What changed: Instance_DidCreate calls initInstanceInBrowserWindow rather
// than initInstanceInPCWindow. The initInstanceInBrowserWindow uses
// postMessage to place text (now "Hello, Native Client") in the web page
// instead of opening and writing to a window.

#define STEP6
// What changed: All the Windows code is def'd out, to prove we are
// PPAPI-compliant.  The functional code that is running is the same as STEP5.

// *** SELECT THE NACL64 PLATFORM AND RUN ***

// What changed: The code is the same as STEP6, but you are using the SDK
// toolchain to compile it into a nexe.  The module is now running as a real
// Native Client executable in a NaCl sandbox, with nacl-gdb attached.

// *** RUN YOUR MODULE IN THE WILD ***
// You can run your nexe outside of Visual Studio, directly from Chrome by
// following these steps:
// - Build STEP6 and verify the file
//   <project directory>/newlib/hello_nacl_plus/hello_nacl_plus.nexe exists
// - Copy the folder <project directory>/hello_nacl_plus into your NaCl SDK's
//   example directory.
// - Go to the NaCl SDK directory and launch the httpd.py server.
// - Launch Chrome, go to about:flags and enable the Native Client flag and
//   relaunch Chrome
// - Point Chrome at localhost:5103/hello_nacl_plus

#ifdef STEP6
// remove Windows-dependent code.
#undef STEP1
#undef STEP3
#undef STEP4
#define NULL 0
#else
// includes for Windows APIs.
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#endif

#ifdef STEP2
// includes for PPAPI C++
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/core.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

// This is a known PPAPI platform problem (Issue 81375).
// When WinUser.h is included it defines PostMessage, so we undef it here.
// There is a work-around:
#ifdef PostMessage
#undef PostMessage
#endif

class NaClProjectInstance *myInstance;
int initInstanceInPCWindow();
void initInstanceInBrowserWindow();

#endif

#ifdef STEP2

// **** Native Client Framework ****

// The Instance class.
class NaClProjectInstance : public pp::Instance {
public:
  pp::CompletionCallbackFactory<NaClProjectInstance> factory_;

  explicit NaClProjectInstance(PP_Instance instance)
    : pp::Instance(instance),
    // Use this macro to eliminate compiler warning.
    PP_ALLOW_THIS_IN_INITIALIZER_LIST(factory_(this)) {
      myInstance = this;
  }


  virtual ~NaClProjectInstance() {
  }

  virtual bool Init(uint32_t /*argc*/, const char* /*argn*/[],
    const char* /*argv*/[]) {
      PostMessage(pp::Var("Creating Instance Start"));
#ifdef STEP5
      // Will be included in STEP5 and STEP6
      // Uses messaging to relay text to the module's view on the web page
      initInstanceInBrowserWindow();
#else
#ifdef STEP3
      // Will be included in STEP3 and STEP4 only
      // Uses WndProc to place text in a window separate from the browser.
      initInstanceInPCWindow();
#endif
#endif
      PostMessage(pp::Var("Creating Instance End"));
      return true;
  }

#ifdef STEP4
  // Implements Windows window message loop with a callback function.
  void NaClProjectInstance::SendCallback(int result) {
    pp::Core* core = pp::Module::Get()->core();
    CompletionCallback callback = factory_.NewCallback(&HandleWindowMsg);
    core->CallOnMainThread(100, callbacl, result);
  }

  void HandleWindowMsg(int32_t result) {
    MSG uMsg;
    if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&uMsg);
      DispatchMessage(&uMsg);
    }
    SendCallback(0);
  }

#endif
private:
  virtual void HandleMessage(const pp::Var& var_message) {
  }
};

// The Module class.
class NaClProjectModule : public pp::Module {
public:
  NaClProjectModule() : pp::Module() {}
  virtual ~NaClProjectModule() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new NaClProjectInstance(instance);
  }
};

namespace pp {
  Module* CreateModule() {
    return new NaClProjectModule();
  }
}

#endif

// **** Application Code ****

#ifdef STEP1
// Desktop Windows Hello World app. Native Client agnostic.

static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("hello_nacl_plus");
HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// WinMain
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nCmdShow)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style          = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc    = WndProc;
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hInstance      = hInstance;
  wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
  wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName   = NULL;
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm        = LoadIcon(wcex.hInstance,
                                 MAKEINTRESOURCE(IDI_APPLICATION));

  if (!RegisterClassEx(&wcex)) {
    MessageBox(NULL,
      _T("Call to RegisterClassEx failed!"),
      _T("hello_nacl_plus"),
      NULL);

    return 1;
  }

  hInst = hInstance;

  HWND hWnd = CreateWindow(
    szWindowClass,
    szTitle,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    500, 100,
    NULL,
    NULL,
    hInstance,
    NULL);

  if (!hWnd) {
    MessageBox(NULL,
      _T("Call to CreateWindow failed!"),
      _T("hello_nacl_plus"),
      NULL);

    return 1;
  }

  ShowWindow(hWnd,
    nCmdShow);
  UpdateWindow(hWnd);

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int) msg.wParam;
}

//  WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  TCHAR greeting[] = _T("Hello, World!");

  switch (message)
  {
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);

    // Here your application is laid out.
    // For this introduction, we just print out "Hello, World!"
    // in the top left corner.
    TextOut(hdc,
      5, 5,
      greeting, _tcslen(greeting));
    // End application-specific layout section.

    EndPaint(hWnd, &ps);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
    break;
  }

  return 0;
}
#endif

#ifdef STEP3
// Replace WinMain with initInstanceInPCWindow so the NativeClient Module can
// launch the original application.
// Note the inclusion of a message-handling loop. STEP4 will replace the loop
// with a callback.

void shutDown(void);
HINSTANCE g_hInstance = NULL;
HWND g_hWnd = NULL;

int initInstanceInPCWindow()
{
  WNDCLASSEX winClass;
  MSG        uMsg;

  memset(&uMsg,0,sizeof(uMsg));

  winClass.lpszClassName = _T("MY_WINDOWS_CLASS");
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  winClass.lpfnWndProc   = WndProc;
  winClass.hInstance     = g_hInstance;
  winClass.hIcon         = NULL;
  winClass.hIconSm     =  NULL;
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  if (!RegisterClassEx(&winClass))
    return E_FAIL;

  g_hWnd = CreateWindowEx(NULL,_T("MY_WINDOWS_CLASS"),
    _T("hello_nacl_plus"),
    WS_OVERLAPPEDWINDOW,
    0,0, 640, 480, NULL, NULL, g_hInstance, NULL);

  if (g_hWnd == NULL)
    return E_FAIL;

  ShowWindow(g_hWnd, 1);

  UpdateWindow(g_hWnd);

#ifdef STEP4
  // Skip the message loop, schedule a callback instead to periodically check
  // for messages. Here we schedule at 100ms intervals.
  myInstance->SendCallback(0);
  return 0;
#else
  // Main message loop, Windows style.
  while(uMsg.message != WM_QUIT) {
    if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&uMsg);
      DispatchMessage(&uMsg);
    }
  }
  return uMsg.wParam;
#endif

}
#endif

#ifdef STEP5
// Pass the text to the browser page, there is no separate app window anymore.
// The text is added as a new element to the page, it does not appear in the
// module's embed view.
void initInstanceInBrowserWindow() {
  myInstance->PostMessage(pp::Var("Hello, Native Client!"));
}
#endif
