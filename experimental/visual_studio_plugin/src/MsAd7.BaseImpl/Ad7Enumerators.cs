// Copyright 2009 The Native Client Authors. All rights reserved.
// 
// Use of this source code is governed by a BSD-style license that can
// 
// be found in the LICENSE file.

using System.Collections.Generic;
using System.Reflection;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using System.Linq;

namespace Google.MsAd7.BaseImpl {
  namespace Ad7Enumerators {

    #region Base

    public class EnumeratorBase<TElement, TBase, TDerived>
        where TDerived : EnumeratorBase<TElement, TBase, TDerived>, TBase
        where TBase : class {
      public EnumeratorBase(ICollection<TElement> collection) {
        collection_ = collection;
        enumerator_ = collection.GetEnumerator();
      }

      public ICollection<TElement> Collection {
        get { return collection_; }
      }

      #region Implementation of IEnumDebugTElements2

      /// <summary>
      ///   Returns the next set of elements from the enumeration.
      /// </summary>
      /// <param name = "celt">
      ///   [in] The number of elements to retrieve.
      ///   Also specifies the maximum size of the <paramref name = "rgelt" />
      ///   array.
      /// </param>
      /// <param name = "rgelt">
      ///   [in, out] Array of TInterface elements to be filled in.
      /// </param>
      /// <param name = "pceltFetched">
      ///   [out] Returns the number of elements actually returned in rgelt.
      /// </param>
      /// <returns>
      ///   If successful, returns S_OK. Returns S_FALSE if fewer than the
      ///   requested number of elements could be returned; otherwise, returns
      ///   an error code
      /// </returns>
      public int Next(uint celt, TElement[] rgelt, ref uint pceltFetched) {
        pceltFetched = 0;
        for (uint i = 0;
             i < celt && enumerator_.MoveNext();
             ++i) {
          rgelt[i] = enumerator_.Current;
          ++pceltFetched;
        }
        return (celt == pceltFetched) ? VSConstants.S_OK : VSConstants.S_FALSE;
      }


      /// <summary>
      ///   Skips over the specified number of elements.
      /// </summary>
      /// <param name = "celt">[in] Number of elements to skip.</param>
      /// <returns>
      ///   If successful, returns S_OK. Returns S_FALSE if celt is greater than
      ///   the number of remaining elements; otherwise, returns an error code.
      /// </returns>
      public int Skip(uint celt) {
        uint i;
        for (i = 0;
             i < celt && enumerator_.MoveNext();
             ++i) {}
        return (i == celt) ? VSConstants.S_OK : VSConstants.S_FALSE;
      }

      /// <summary>
      ///   Resets the enumeration to the first element.
      /// </summary>
      /// <returns>
      ///   If successful, returns S_OK; otherwise, returns an error code.
      /// </returns>
      public int Reset() {
        enumerator_ = collection_.GetEnumerator();
        return VSConstants.S_OK;
      }

      /// <summary>
      ///   Returns a copy of the current enumeration as a separate object.
      /// </summary>
      /// <param name = "ppEnum">
      ///   [out] Returns a copy of this enumeration as a separate object.
      /// </param>
      /// <returns>
      ///   If successful, returns S_OK; otherwise, returns an error code.
      /// </returns>
      public int Clone(out TBase ppEnum) {
        ConstructorInfo cons =
            typeof (TDerived).GetConstructor(new[] {collection_.GetType()});
        ppEnum = cons.Invoke(new object[] {collection_}) as TBase;

        return VSConstants.S_OK;
      }

      /// <summary>
      /// </summary>
      /// <param name = "pcelt">
      ///   [out] Returns the number of elements in the enumeration.
      /// </param>
      /// <returns>
      ///   If successful, returns S_OK; otherwise, returns an error code. 
      /// </returns>
      public int GetCount(out uint pcelt) {
        pcelt = (uint) collection_.Count;
        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      readonly ICollection<TElement> collection_;
      IEnumerator<TElement> enumerator_;

      #endregion
        }

    #endregion Base

    public class ErrorBreakpointEnumerator
      : EnumeratorBase<IDebugErrorBreakpoint2, 
                       IEnumDebugErrorBreakpoints2, 
                       ErrorBreakpointEnumerator>,
        IEnumDebugErrorBreakpoints2 {
      public ErrorBreakpointEnumerator()
        : base(new List<IDebugErrorBreakpoint2>()) {}
      public ErrorBreakpointEnumerator(ICollection<IDebugErrorBreakpoint2> collection)
        :base(collection) {}

      /// <summary>
      /// This function exists so the PendingBreakpoint can add itself to the errors in this
      /// enumeration after BreakpointInfo generates them.
      /// </summary>
      public int PopulateBreakpoint(PendingBreakpoint breakpoint) {
        int populated = VSConstants.S_FALSE;
        if (Collection != null) {
          foreach (
              ErrorBreakpoint assignable in
                  Collection.OfType<ErrorBreakpoint>()) {
            assignable.PendingBreakpoint = breakpoint;
            populated = VSConstants.S_OK;
          }
        }
        return populated;
      }

      public void Insert(IDebugErrorBreakpoint2 point) {
        Collection.Add(point);
        // Reset makes the underlying class get a new enumerator, which is necessary because the
        // old one was just invalidated.
        Reset();
      }
    }

    public class ProgramEnumerator
        : EnumeratorBase<IDebugProgram2, IEnumDebugPrograms2, ProgramEnumerator>
          ,
          IEnumDebugPrograms2 {
      public ProgramEnumerator(ICollection<IDebugProgram2> collection)
          : base(collection) {}
          }

    public class ModuleEnumerator
        : EnumeratorBase<IDebugModule2, IEnumDebugModules2, ModuleEnumerator>,
          IEnumDebugModules2 {
      public ModuleEnumerator(ICollection<IDebugModule2> collection)
          : base(collection) {}
          }

    public class FrameInfoEnumerator
        : EnumeratorBase<FRAMEINFO, IEnumDebugFrameInfo2, FrameInfoEnumerator>,
          IEnumDebugFrameInfo2 {
      public FrameInfoEnumerator(ICollection<FRAMEINFO> collection)
          : base(collection) {}
          }

    public class ThreadEnumerator
        : EnumeratorBase<IDebugThread2, IEnumDebugThreads2, ThreadEnumerator>,
          IEnumDebugThreads2 {
      public ThreadEnumerator(ICollection<IDebugThread2> collection)
          : base(collection) {}
          }

    public class PropertyEnumerator
        : EnumeratorBase
              <DEBUG_PROPERTY_INFO, IEnumDebugPropertyInfo2, PropertyEnumerator>,
          IEnumDebugPropertyInfo2 {
      public PropertyEnumerator(ICollection<DEBUG_PROPERTY_INFO> collection)
          : base(collection) {}

      // The PIA definition of this interface has a typo, and Next is defined with
      // an out param instead of a ref param. This little shim fixes that.

      #region IEnumDebugPropertyInfo2 Members

      public int Next(uint celt,
                      DEBUG_PROPERTY_INFO[] rgelt,
                      out uint pceltFetched) {
        uint n = 0;
        int result = base.Next(celt, rgelt, ref n);
        pceltFetched = n;
        return result;
      }

      #endregion
          }

    public class CodeContextEnumerator
        : EnumeratorBase
              <IDebugCodeContext2, IEnumDebugCodeContexts2,
              CodeContextEnumerator>,
          IEnumDebugCodeContexts2 {
      public CodeContextEnumerator(ICollection<IDebugCodeContext2> collection)
          : base(collection) {}
    }

    public class PortEnumerator
        : EnumeratorBase<IDebugPort2, IEnumDebugPorts2, PortEnumerator>,
          IEnumDebugPorts2 {
      public PortEnumerator(ICollection<IDebugPort2> collection)
        : base(collection) { }
    }

    public class ProcessEnumerator
        : EnumeratorBase
              <IDebugProcess2, IEnumDebugProcesses2, ProcessEnumerator>,
          IEnumDebugProcesses2 {
      public ProcessEnumerator(ICollection<IDebugProcess2> collection)
          : base(collection) {}
    }
  }
}
