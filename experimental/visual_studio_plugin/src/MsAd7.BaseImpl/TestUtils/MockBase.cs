// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;

#endregion

namespace Google.MsAd7.BaseImpl.TestUtils {
  /// <summary>
  ///   This class provides a simple record-replay mock.  It lives in MsAd7
  ///   because this is the lowest-level C# project.
  /// </summary>
  public class MockBase {
    /// <summary>
    ///   Resets the state of the Mock. This exists so that unit testing
    ///   functionality can be factored in such a way that each function that
    ///   has to mock transactions does not have to know about the transactions
    ///   tested by any other functions involved.
    /// </summary>
    public void Reset() {
      expectedCalls_ = new List<MockEntry>();
      index_ = 0;
    }

    /// <summary>
    ///   Records a call with one argument to be expected by the mock.  The
    ///   calls will be expected in the order in which they are recorded.
    /// </summary>
    /// <typeparam name = "TArgType">The type of the call's argument.
    /// </typeparam>
    /// <param name = "functionName">The name of the function to be called.
    /// </param>
    /// <param name = "functionArg">The argument of the function to be called.
    /// </param>
    /// <param name = "returnValue">The value to be returned when this mock-call
    ///   occurs.</param>
    public void RecordCall<TArgType>(string functionName,
                                     TArgType functionArg,
                                     object returnValue) {
      var args = new List<object>();
      args.Add(functionArg);
      AddCall(functionName, args, returnValue);
    }

    /// <summary>
    ///   Records a call with no arguments to be expected by the mock.  The
    ///   calls will be expected in the order in which they are recorded.
    /// </summary>
    /// <param name = "functionName">The name of the function to be called.
    /// </param>
    /// <param name = "returnValue">The value to be returned when this mock-call
    ///   occurs.</param>
    public void RecordCall(string functionName,
                           object returnValue) {
      AddCall(functionName, new List<object>(), returnValue);
    }

    #region Private Implementation

    private List<MockEntry> expectedCalls_ = new List<MockEntry>();
    private int index_;

    #endregion

    #region Private Implementation

    /// <summary>
    ///   Adds a call to the queue.  This function is argument and type agnostic
    ///   so it can be used by all the RecordCall functions.
    /// </summary>
    /// <param name = "functionName">The name of the function to be called.
    /// </param>
    /// <param name = "functionArgs">Any arguments to be expected for the
    ///   function call, as a list of objects.</param>
    /// <param name = "returnValue">The value to be returned when this mock-call
    ///   occurs.</param>
    private void AddCall(string functionName,
                         List<object> functionArgs,
                         object returnValue) {
      var mockEntry = new MockEntry {
          FunctionName = functionName,
          FunctionArgs = functionArgs,
          ReturnValue = returnValue,
      };
      expectedCalls_.Add(mockEntry);
    }

    #endregion

    /// <summary>
    ///   Checks the validity of a call that has been made.  This function is to
    ///   be invoked by the mock when any of its one-argument functions are
    ///   called and need to be verified.
    /// </summary>
    /// <typeparam name = "TArgType">The type of the argument to be checked.
    /// </typeparam>
    /// <typeparam name = "TReturnType">The type to cast the return value to.
    /// </typeparam>
    /// <param name = "aName">The name of the function to be checked.</param>
    /// <param name = "functionArg">The argument with which the function got
    ///   called.</param>
    /// <returns>The stored return value for this function call if all the
    ///   checks pass.</returns>
    protected TReturnType CheckCall<TArgType, TReturnType>(
        string aName, TArgType functionArg) {
      if (index_ >= expectedCalls_.Count) {
        throw new MockException(
            "Got called when no calls were expected.");
      }
      var mockEntry = expectedCalls_[index_];
      if (mockEntry.FunctionName != aName ||
          mockEntry.FunctionArgs.Count != 1) {
        throw new MockException(
            "Mock got called with the wrong function signature.");
      }
      if (!(mockEntry.FunctionArgs[0] is TArgType)) {
        throw new MockException(
            "Mock got called with the wrong argument type.");
      }
      var expectedArg = (TArgType) mockEntry.FunctionArgs[0];
      if (!Equals(expectedArg, functionArg)) {
        throw new MockException(
            "Mock got called with the wrong argument value.");
      }
      if (!(expectedCalls_[index_].ReturnValue is TReturnType)) {
        throw new MockException(
            "Mock got called with the wrong return type.");
      }
      return (TReturnType) expectedCalls_[index_++].ReturnValue;
    }

    /// <summary>
    ///   Checks the validity of a call that has been made.  This function is to
    ///   be invoked by the mock when any of its one-argument functions are
    ///   called and need to be verified.
    /// </summary>
    /// <typeparam name = "TReturnType">The type to cast the return value to.
    /// </typeparam>
    /// <param name = "aName">The name of the function to be checked.</param>
    /// <returns>The stored return value for this function call if all the
    ///   checks pass.</returns>
    protected TReturnType CheckCall<TReturnType>(string aName) {
      if (index_ >= expectedCalls_.Count) {
        throw new MockException(
            "Got called when no calls were expected.");
      }
      var mockEntry = expectedCalls_[index_];
      if (mockEntry.FunctionName != aName ||
          mockEntry.FunctionArgs.Count != 0) {
        throw new MockException(
            "Mock got called with the wrong function signature.");
      }
      if (!(expectedCalls_[index_].ReturnValue is TReturnType)) {
        throw new MockException(
            "Mock got called with the wrong return type.");
      }
      return (TReturnType) expectedCalls_[index_++].ReturnValue;
    }

    #region Nested type: MockEntry

    protected class MockEntry {
      public string FunctionName;
      public List<object> FunctionArgs;
      public object ReturnValue;
    }

    #endregion

    #region Nested type: MockException

    public class MockException : Exception {
      public MockException(string error) : base(error) {}
    }

    #endregion
  }
}
