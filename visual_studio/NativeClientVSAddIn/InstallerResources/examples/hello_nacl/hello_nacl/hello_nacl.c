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

//#define STEP2
// What Changed: The platform launches Chrome, which will then load a Native
// Client Module. STEP2 encloses the Native Client module APIs needed to link
// any app to the browser. The module does nothing except report
// starting/ending the function Instance_DidCreate. The Windows app does not
// run because it is not being called.

//#define STEP3
// What changed: Replace WinMain with WndProc, and call it from
// Instance_DidCreate, launching HelloNaCl in its own window.
// Since WndProc spins in its message loop, the call to Instance_DidCreate
// never returns.
// Close the HelloNaCl window and the module initialization will finish.

//#define STEP4
// What changed: In WndProc replace the message loop with a callback function.
// Now the app window and the Native Client module are running concurrently.

//#define STEP5
// What changed: Instance_DidCreate calls initInstanceInBrowserWindow rather
// than initInstanceInPCWindow.
// The initInstanceInBrowserWindow uses postMessage to place text (now "Hello,
// Native Client") in the web page instead of opening and writing to a window.

//#define STEP6
// What changed: All the Windows code is def'd out, to prove we are
// PPAPI-compliant.  The functional code that is running is the same as STEP5.

// *** SELECT THE NACL64 PLATFORM AND RUN ***

// What changed: The code is the same as STEP6, but you are using the SDK
// toolchain to compile it into a nexe.  The module is now running as a real
// Native Client executable in a NaCl sandbox, with nacl-gdb attached.

// *** RUN YOUR MODULE IN THE WILD ***
// You can run your nexe outside of Visual Studio, directly from Chrome by
// following these steps:
// - Build STEP6 and verify the file <project
//   directory>/newlib/HelloNaCl/HelloNaCl.nexe exists
// - Copy the folder <project directory>/HelloNaCl into your NaCl SDK's example
//   directory.
// - Go to the NaCl SDK directory and launch the httpd.py server.
// - Launch Chrome, go to about:flags and enable the Native Client flag and
//   relaunch Chrome
// - Point Chrome at localhost:5103/HelloNaCl


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
// includes for PPAPI
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include <string.h>

// Native Client APIs
static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
PP_Instance myInstance;

int initInstanceInPCWindow();
void initInstanceInBrowserWindow();

#endif


#ifdef STEP4
// Implements message handling in a callback function.
void HelloWorldCallbackFun(void* user_data, int32_t result);
struct PP_CompletionCallback HelloWorldCallback = { HelloWorldCallbackFun, NULL };

void HelloWorldCallbackFun(void* user_data, int32_t result) {
  MSG uMsg;
  if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&uMsg);
    DispatchMessage(&uMsg);
  }
  ppb_core_interface->CallOnMainThread(100, HelloWorldCallback, 0);
}

#endif

#ifdef STEP2
// The basic framework needed for all Native Client Modules.  Handles creation
// of the module instance and initial handshake with the browser.
/**
 * Creates new string PP_Var from C string. Useful utility for
 * message-handling.
 */
static struct PP_Var CStrToVar(const char* str) { if (ppb_var_interface !=
NULL) { return ppb_var_interface->VarFromUtf8(str, strlen(str)); } return
PP_MakeUndefined(); }


void initInstanceInBrowserWindow() {
  // Pass the text to the browser page, there is no separate app window
  // anymore.  The text is added as a new element to the page, it does not
  // appear in the module's embed view.
  ppb_messaging_interface->PostMessage(myInstance, CStrToVar("Hello, Native
  Client!")); }

/**
 * Called when the NaCl module is instantiated on the web page.
 */
static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
  myInstance = instance;
  ppb_messaging_interface->PostMessage(instance,
                                       CStrToVar("Start Instance_DidCreate"));
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

  ppb_messaging_interface->PostMessage(instance,
                                       CStrToVar("End Instance_DidCreate"));
  return PP_TRUE;
}

/**
 * Called when the NaCl module is destroyed.
 */
static void Instance_DidDestroy(PP_Instance instance) {
}

/**
 * Called when the position, the size, or the clip rect of the element in the
 * browser that corresponds to this NaCl module has changed.
  */
static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) {
}

/**
 * Notification that the given NaCl module has gained or lost focus.
  */
static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

/**
 * Handler that gets called after a full-frame module is instantiated based on
 * registered MIME types.  This function is not called on NaCl modules.  This
 * function is essentially a place-holder for the required function pointer in
 * the PPP_Instance structure.
  */
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}



/**
 * Entry points for the module.
 * Initialize needed interfaces: PPB_Core, PPB_Messaging and PPB_Var.
 */
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  ppb_messaging_interface = (PPB_Messaging*)
                            get_browser(PPB_MESSAGING_INTERFACE);
  ppb_var_interface = (PPB_Var*)get_browser(PPB_VAR_INTERFACE);
  ppb_core_interface = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);
  return PP_OK;
}


/**
 * Returns an interface pointer for the interface of the given name, or NULL
 * if the interface is not supported.
  */
PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleDocumentLoad,
    };
    return &instance_interface;
  }
  return NULL;
}


/**
 * Called before the plugin module is unloaded.
 */
PP_EXPORT void PPP_ShutdownModule() {
}
#endif



// **** Application Code ****

#ifdef STEP1
// Desktop Windows Hello World app. Native Client agnostic.

static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("HelloNaCl");
HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// WinMain
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
  WNDCLASSEX wcex;
  HWND hWnd;
  MSG msg;

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
        _T("HelloNaCl"),
        0);

    return 1;
  }

  hInst = hInstance;

  hWnd = CreateWindow(
      szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      500, 100,
      NULL,
      NULL,
      hInstance,
      NULL
      );

  if (!hWnd) {
    MessageBox(NULL,
        _T("Call to CreateWindow failed!"),
        _T("HelloNaCl"),
        0);

    return 1;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  // Main message loop:

  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int) msg.wParam;
}

// WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  TCHAR greeting[] = _T("Hello, World!");

  switch (message) {
    case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
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
// Replace WinMain with initInstanceInPCWindow so the Native Client Module can
// launch the original application.  Note the inclusion of a message-handling
// loop. STEP4 will replace the loop with a callback.
HINSTANCE g_hInstance = NULL; HWND g_hWnd = NULL; int initInstanceInPCWindow()
{ WNDCLASSEX winClass; MSG        uMsg;

  memset(&uMsg,0,sizeof(uMsg));

  winClass.lpszClassName = _T("MY_WINDOWS_CLASS");
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  winClass.lpfnWndProc   = WndProc;
  winClass.hInstance     = g_hInstance;
  winClass.hIcon         = NULL;
  winClass.hIconSm       =  NULL;
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  if (!RegisterClassEx(&winClass))
    return E_FAIL;

  g_hWnd = CreateWindowEx(NULL, _T("MY_WINDOWS_CLASS"),
      _T("HelloNaCl"),
      WS_OVERLAPPEDWINDOW,
      0, 0, 640,480, NULL, NULL, g_hInstance, NULL);

  if (g_hWnd == NULL)
    return E_FAIL;

  ShowWindow(g_hWnd, 1);

  UpdateWindow(g_hWnd);

#ifdef STEP4
  // Skip the message loop, schedule a callback instead to periodically check
  // for messages. Here we schedule at 100ms intervals.
  ppb_core_interface->CallOnMainThread(100, HelloWorldCallback, 0);
  return 0;
#else
  // Main message loop, Windows style.
  while(uMsg.message != WM_QUIT) {
    if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage( &uMsg );
      DispatchMessage( &uMsg );
    }
  }
  return uMsg.wParam;
#endif

}
#endif
