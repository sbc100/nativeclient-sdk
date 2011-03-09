/*
 * Copyright 2008 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 *
 * This loader can only process NaCl object files as produced using
 * the NaCl toolchain.  Other ELF files will be rejected.
 *
 * The primary function, NaClAppLoadFile, parses an ELF file,
 * allocates memory, loads the relocatable image from the ELF file
 * into memory, and performs relocation.  NaClAppRun runs the
 * resultant program.
 *
 * This loader is written in C so that it can be used by C-only as
 * well as C++ applications.  Other languages should also be able to
 * use their foreign-function interfaces to invoke C code.
 *
 * This loader must be part of the NaCl TCB, since it directly handles
 * externally supplied input (the ELF file).  Any security
 * vulnerabilities in handling the ELF image, e.g., buffer or integer
 * overflows, can put the application at risk.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_LDR_H_
#define NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_LDR_H_ 1

#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/include/elf.h"

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_threads.h"

#include "native_client/src/trusted/service_runtime/dyn_array.h"
#include "native_client/src/trusted/service_runtime/nacl_config_dangerous.h"
#include "native_client/src/trusted/service_runtime/nacl_error_code.h"
#include "native_client/src/trusted/service_runtime/nacl_sync_queue.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_mem.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"


EXTERN_C_BEGIN

#define NACL_SERVICE_PORT_DESCRIPTOR    3
#define NACL_SERVICE_ADDRESS_DESCRIPTOR 4

#define NACL_DEFAULT_ALLOC_MAX  (32 << 20)  /* total brk and mmap allocs */
#define NACL_DEFAULT_STACK_MAX  (16 << 20)  /* main thread stack */

#define NACL_SANDBOX_CHROOT_FD  "SBX_D"

/* NMM */
#if __IGNORE_NACL_WINDOWS
#define WINDOWS_EXCEPTION_TRY do { __try {
#define WINDOWS_EXCEPTION_CATCH } __except(EXCEPTION_EXECUTE_HANDLER) { \
                                  NaClLog(LOG_ERROR, \
                                      "Unhandled Windows exception\n"); \
                                  exit(1); \
                                } \
                              } while (0)
#else
#define WINDOWS_EXCEPTION_TRY do {
#define WINDOWS_EXCEPTION_CATCH } while (0)
#endif

struct NaClAppThread;
struct NaClDesc;  /* see native_client/src/trusted/desc/nacl_desc_base.h */

struct NaClApp {
  /*
   * public, user settable.
   */
  uint8_t                   addr_bits;
  uintptr_t                 max_data_alloc;
  uintptr_t                 stack_size;
  /*
   * max_data_alloc controls how much total data memory can be
   * allocated to the NaCl process; this is initialized data,
   * uninitialized data, and heap and affects the brk system call.
   * the text size and rodata size are not included, even though in
   * NaCl the text and rodata pages are also backed by the pager
   * since due to relocation the text pages and rodata contents
   * cannot simply be memory mapped from the executable.
   *
   * stack_size is the maximum size of the (main) stack.  The stack
   * memory is eager allocated (mapped in w/o MAP_NORESERVE) so
   * there must be enough swap space; page table entries are not
   * populated (no MAP_POPULATE), so actual accesses will likely
   * incur page faults.
   */

  /*
   * Determined at load time; OS-determined.
   * Read-only after load, so accesses do not require locking.
   */
  uintptr_t                 mem_start;
  /*
   * Flag to remember whether guard pages may be present.  If true, then
   * on tear down, NaClTeardownMprotectGuards should be invoked.
   *
   * This will probably go away when we remove the NaClAppDtor.
   */
  int                       guard_pages_initialized;

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 32 && __PIC__
  uintptr_t                 pcrel_thunk;
#endif

  /* only used for ET_EXEC:  for CS restriction */
  uintptr_t                 static_text_end;  /* relative to mem_start */
  /* ro after app starts. memsz from phdr */

  uintptr_t                 dynamic_text_start;
  uintptr_t                 dynamic_text_end;

  uintptr_t                 rodata_start;  /* initialized data, ro */
  uintptr_t                 data_start;    /* initialized data/bss, rw */
  /*
   * Various region sizes must be a multiple of NACL_MAP_PAGESIZE
   * before the NaCl app can run.  The sizes from the ELF file
   * (p_filesz field) might not be -- that would waste space for
   * padding -- and while we could use p_memsz to specify padding, but
   * we will record the virtual addresses of the start of the segments
   * and figure out the gap between the p_vaddr + p_filesz of one
   * segment and p_vaddr of the next to determine padding.
   */

  uintptr_t                 data_end;
  /* see break_addr below */

  uintptr_t                 entry_pt;

  /*
   * bundle_size is the bundle alignment boundary for validation (16
   * or 32), so int is okay.  This value must be a power of 2.
   */
  int                       bundle_size;

  /* common to both ELF executables and relocatable load images */

  uintptr_t                 springboard_addr;  /* relative to mem_start */
  /*
   * springboard code addr for context switching into app sandbox, relative
   * to code sandbox CS
   */

  /*
   * The socket at which the app should be accepting connections.  The
   * corresponding socket address are made available by the JavaScript
   * bridge to other NaCl modules.
   */
  struct NaClDesc           *service_port;
  struct NaClDesc           *service_address;

  struct NaClDesc           *secure_channel;
  struct NaClThread         secure_channel_thr;  /* valid iff secure_channel */

  struct NaClMutex          mu;
  struct NaClCondVar        cv;
  char                      *origin;
  NaClErrorCode             module_load_status;
  int                       module_may_start;

  /*
   * runtime info below, thread state, etc; initialized only when app
   * is run.  Mutex mu protects access to mem_map and other member
   * variables while the application is running and may be
   * multithreaded; thread, desc members have their own locks.  At
   * other times it is assumed that only one thread is
   * constructing/loading the NaClApp and that no mutual exclusion is
   * needed.
   */

  /*
   * memory map is in user addresses.
   */
  struct NaClVmmap          mem_map;

  /*
   * may reject nexes that are incompatible w/ dynamic-text in the near future
   */
  int                       use_shm_for_dynamic_text;
  struct NaClDesc           *text_shm;
  struct NaClMutex          dynamic_load_mutex;

  int                       running;
  int                       exit_status;

  /*
   * enforce that some "special" syscalls may only be made from the
   * main/privileged thread
   */
  int                       restrict_to_main_thread;
  /* all threads enqueue the "special" syscalls to the work queue */
  struct NaClSyncQueue      work_queue;

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 32
  uint16_t                  code_seg_sel;
  uint16_t                  data_seg_sel;
#endif

  uintptr_t                 break_addr;   /* user addr */
  /* data_end <= break_addr is an invariant */

  int                       freeze_thread_ops;
  /* used when process is killed, or when address space move is needed */

  /*
   * Thread table lock threads_mu is higher in the locking order than
   * the thread locks, i.e., threads_mu must be acqured w/o holding
   * any per-thread lock (natp->mu).
   */
  struct NaClMutex          threads_mu;
  struct NaClCondVar        threads_cv;
  struct DynArray           threads;   /* NaClAppThread pointers */
  int                       num_threads;  /* number actually running */

  struct NaClMutex          desc_mu;
  struct DynArray           desc_tbl;  /* NaClDesc pointers */
};



void  NaClAppIncrVerbosity(void);

int   NaClAppCtor(struct NaClApp  *nap) NACL_WUR;

/* DEPRECATED */
void NaClAppDtor(struct NaClApp  *nap);
/*
 * TODO(bsy): remove NaClAppDtor.  This teardown/cleanup activity
 * is better left to the OS.
 *
 * We expect *one* NaClApp per process, and having multiple NaClApp
 * constructed / used is not supported (though some tests do this).
 */

void  NaClAppFreeAllMemory(struct NaClApp *nap);

/*
 * Loads a NaCl ELF file into memory in preparation for running it.
 *
 * gp is a pointer to a generic I/O object and should be a GioMem with
 * a memory buffer containing the file read entirely into memory if
 * the file system might be subject to race conditions (e.g., another
 * thread / process might modify a downloaded NaCl ELF file while we
 * are loading it here).
 *
 * nap is a pointer to the NaCl object that is being filled in.  it
 * should be properly constructed via NaClAppCtor.
 *
 * return value: one of the LOAD_* values defined in
 * nacl_error_code.h.  TODO: add some error detail string and hang
 * that off the nap object, so that more details are available w/o
 * incrementing verbosity (and polluting stdout).
 *
 * note: it may be necessary to flush the icache if the memory
 * allocated for use had already made it into the icache from another
 * NaCl application instance, and the icache does not detect
 * self-modifying code / data writes and automatically invalidate the
 * cache lines.
 */
enum NaClAbiCheckOption {
  NACL_ABI_CHECK_OPTION_SKIP,
  NACL_ABI_CHECK_OPTION_CHECK
};

NaClErrorCode NaClAppLoadFile(struct Gio      *gp,
                              struct NaClApp  *nap,
                              enum NaClAbiCheckOption check_abi)
  NACL_WUR;

size_t  NaClAlignPad(size_t val,
                     size_t align);

void  NaClAppPrintDetails(struct NaClApp  *nap,
                          struct Gio      *gp);

NaClErrorCode NaClLoadImage(struct Gio            *gp,
                            struct NaClApp        *nap) NACL_WUR;

int NaClValidateCode(struct NaClApp *nap,
                     uintptr_t      guest_addr,
                     uint8_t        *data,
                     size_t         size) NACL_WUR;
void NaClIgnoreValidatorResult();
NaClErrorCode NaClValidateImage(struct NaClApp  *nap) NACL_WUR;


int NaClAddrIsValidEntryPt(struct NaClApp *nap,
                           uintptr_t      addr);

/*
 * Takes ownership of descriptor, i.e., when NaCl app closes, it's gone.
 */
void NaClAddHostDescriptor(struct NaClApp *nap,
                           int            host_os_desc,
                           int            mode,
                           int            nacl_desc);

/*
 * Takes ownership of handle.
 */
void NaClAddImcHandle(struct NaClApp  *nap,
                      NaClHandle      h,
                      int             nacl_desc);

void NaClAddImcAddr(struct NaClApp                  *nap,
                    struct NaClSocketAddress const  *addr,
                    int                             nacl_desc);

/*
 * Used to launch the main thread.  NB: calling thread may in the
 * future become the main NaCl app thread, and this function will
 * return only after the NaCl app main thread exits.  In such an
 * alternative design, NaClWaitForMainThreadToExit will become a
 * no-op.
 */
int NaClCreateMainThread(struct NaClApp     *nap,
                         int                argc,
                         char               **argv,
                         char const *const  *envp) NACL_WUR;

int NaClWaitForMainThreadToExit(struct NaClApp  *nap);

/*
 * Used by syscall code.
 */
int32_t NaClCreateAdditionalThread(struct NaClApp *nap,
                                   uintptr_t      prog_ctr,
                                   uintptr_t      stack_ptr,
                                   uintptr_t      sys_tdb,
                                   size_t         tdb_size) NACL_WUR;

void NaClLoadTrampoline(struct NaClApp *nap);

void NaClLoadSpringboard(struct NaClApp  *nap);

static const uintptr_t kNaClBadAddress = (uintptr_t) -1;

#ifndef NACL_NO_INLINE
#include "native_client/src/trusted/service_runtime/sel_ldr-inl.h"
#endif

/*
 * Looks up a descriptor in the open-file table.  An additional
 * reference is taken on the returned NaClDesc object (if non-NULL).
 * The caller is responsible for invoking NaClDescUnref() on it when
 * done.
 */
struct NaClDesc *NaClGetDesc(struct NaClApp *nap,
                             int            d);

/*
 * Takes ownership of ndp.
 */
void NaClSetDesc(struct NaClApp   *nap,
                 int              d,
                 struct NaClDesc  *ndp);


int32_t NaClSetAvail(struct NaClApp   *nap,
                     struct NaClDesc  *ndp);

/*
 * Versions that are called while already holding the desc_mu lock
 */
struct NaClDesc *NaClGetDescMu(struct NaClApp *nap,
                               int            d);

void NaClSetDescMu(struct NaClApp   *nap,
                   int              d,
                   struct NaClDesc  *ndp);

int32_t NaClSetAvailMu(struct NaClApp   *nap,
                       struct NaClDesc  *ndp);


int NaClAddThread(struct NaClApp        *nap,
                  struct NaClAppThread  *natp);

int NaClAddThreadMu(struct NaClApp        *nap,
                    struct NaClAppThread  *natp);

void NaClRemoveThread(struct NaClApp  *nap,
                      int             thread_num);

void NaClRemoveThreadMu(struct NaClApp  *nap,
                        int             thread_num);

struct NaClAppThread *NaClGetThreadMu(struct NaClApp  *nap,
                                      int             thread_num);

void NaClAppVmmapUpdate(struct NaClApp    *nap,
                        uintptr_t         page_num,
                        size_t            npages,
                        int               prot,
                        struct NaClMemObj *nmop,
                        int               remove);

uintptr_t NaClAppVmmapFindSpace(struct NaClApp  *nap,
                                int             num_pages);

uintptr_t NaClAppVmmapFindMapSpace(struct NaClApp *nap,
                                   int            num_pages);

void NaClCreateServiceSocket(struct NaClApp *nap);

void NaClSendServiceAddressTo(struct NaClApp  *nap,
                              int             desc);

void NaClSecureCommandChannel(struct NaClApp  *nap);

void NaClDumpServiceAddressTo(struct NaClApp  *nap,
                              int             desc);

void NaClWaitForModuleStartStatusCall(struct NaClApp *nap);

void NaClFillMemoryRegionWithHalt(void *start, size_t size);

void NaClFillTrampolineRegion(struct NaClApp *nap);

void NaClFillEndOfTextRegion(struct NaClApp *nap);

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 32 && __PIC__

int NaClMakePcrelThunk(struct NaClApp *nap);

#endif

void NaClPatchOneTrampoline(struct NaClApp *nap,
                            uintptr_t target_addr);
/*
 * target is an absolute address in the source region.  the patch code
 * will figure out the corresponding address in the destination region
 * and modify as appropriate.  this makes it easier to specify, since
 * the target is typically the address of some symbol from the source
 * template.
 */
struct NaClPatch {
  uintptr_t           target;
  uint64_t            value;
};

struct NaClPatchInfo {
  uintptr_t           dst;
  uintptr_t           src;
  size_t              nbytes;

  struct NaClPatch    *abs16;
  size_t              num_abs16;

  struct NaClPatch    *abs32;
  size_t              num_abs32;

  struct NaClPatch    *abs64;
  size_t              num_abs64;

#if NACL_TARGET_SUBARCH == 32
  uintptr_t           *rel32;
  size_t              num_rel32;
#endif

  uintptr_t           *rel64;
  size_t              num_rel64;
};

struct NaClPatchInfo *NaClPatchInfoCtor(struct NaClPatchInfo *self);

void NaClApplyPatchToMemory(struct NaClPatchInfo *patch);

int NaClThreadContextCtor(struct NaClThreadContext  *ntcp,
                          struct NaClApp            *nap,
                          nacl_reg_t                prog_ctr,
                          nacl_reg_t                stack_ptr,
                          uint32_t                  tls_info);

void NaClThreadContextDtor(struct NaClThreadContext *ntcp);

void NaClGdbHook(struct NaClApp const *nap);

EXTERN_C_END

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_LDR_H_ */
