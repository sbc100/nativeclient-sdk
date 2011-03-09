using System.Diagnostics;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public class MemoryBytes : IDebugMemoryBytes2
  {
    // Keep track of the top and bottom of the address space, in
    // untrusted terms.
    public static readonly ulong kMinAddr = 0;
    public static readonly ulong kMaxAddr = 0xffffffff;

    public MemoryBytes(ISimpleDebugger dbg, ulong address, uint size) {
      dbg_ = dbg;
      address_ = address;
      size_ = size;
    }

    public ISimpleDebugger Dbg {
      get { return dbg_; }
    }

    public ulong Address {
      get { return address_; }
    }

    public ulong Size {
      get { return size_; }
    }

    #region Implementation of IDebugMemoryBytes2

    public int ReadAt(IDebugMemoryContext2 pStartContext, uint dwCount, byte[] rgbMemory, out uint pdwRead, ref uint pdwUnreadable) {
      Debug.Assert(pStartContext is MemoryContext);
      Debug.Assert(rgbMemory.Length >= dwCount);

      var context = (MemoryContext) pStartContext;
      Debug.WriteLine(string.Format("MemoryBytes.ReadAt({0})", context.Address));

      pdwRead = dwCount;
      pdwUnreadable = 0;

      if (context.Address + dwCount > kMaxAddr) {
        pdwRead = (uint)(context.Address > kMaxAddr ? 0 : kMaxAddr - context.Address);
        pdwUnreadable = dwCount - pdwRead;
      }

      dbg_.GetMemory(context.Address, rgbMemory, pdwRead);

      return VSConstants.S_OK;
    }

    public int WriteAt(IDebugMemoryContext2 pStartContext, uint dwCount, byte[] rgbMemory) {
      Debug.Assert(pStartContext is MemoryContext);
      Debug.Assert(rgbMemory.Length == dwCount);

      var context = (MemoryContext)pStartContext;
      Debug.WriteLine(string.Format("MemoryBytes.WriteAt({0})", context.Address));

      dbg_.SetMemory(context.Address, rgbMemory, dwCount);

      return VSConstants.S_OK;
    }

    public int GetSize(out ulong pqwSize) {
      // TODO(ilewis): replace this bogus impl with a real one
      pqwSize = 4096;
      return VSConstants.S_OK;
    }

    #endregion

    private ISimpleDebugger dbg_;
    private ulong address_;
    private uint size_;
  }
}
