// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
namespace Google.MsAd7.BaseImpl.DebugProperties {
  public class RegisterSetSchema {
    
    public string Name;
    public RegisterDef[] Registers;

    public static RegisterSetSchema DwarfAmd64Integer = new RegisterSetSchema {
        Name = "CPU",
        Registers = new RegisterDef[] {
            new RegisterDef("CFA", -1, 8, true),
            new RegisterDef("RAX", 0, 8),
            new RegisterDef("RDX", 1, 8),
            new RegisterDef("RCX", 2, 8),
            new RegisterDef("RBX", 3, 8),
            new RegisterDef("RSI", 4, 8),
            new RegisterDef("RDI", 5, 8),
            new RegisterDef("RBP", 6, 8),
            new RegisterDef("RSP", 7, 8),
            new RegisterDef("R8", 8, 8),
            new RegisterDef("R9", 9, 8),
            new RegisterDef("R10", 10, 8),
            new RegisterDef("R11", 11, 8),
            new RegisterDef("R12", 12, 8),
            new RegisterDef("R13", 13, 8),
            new RegisterDef("R14", 14, 8),
            new RegisterDef("R15", 15, 8),
            new RegisterDef("RIP", 16, 8),
            new RegisterDef("FLAGS", 49, 4), 
        }
    };

    public static RegisterSetSchema DwarfAmd64Segment = new RegisterSetSchema
    {
      Name = "Segment",
      Registers = new RegisterDef[] {
          new RegisterDef( "ES", 50, 8),
          new RegisterDef( "CS", 51, 8),
          new RegisterDef( "SS", 52, 8),
          new RegisterDef( "DS", 53, 8),
          new RegisterDef( "FS", 54, 8),
          new RegisterDef( "GS", 55, 8),
        }
    };

    #region Nested type: RegisterDef

    public class RegisterDef {
      public RegisterDef(string name, int index, uint size, bool pseudo) {
        Name = name;
        Index = index;
        Size = size;
        Pseudo = pseudo;
      }

      public RegisterDef(string name, int index, uint size) 
        : this(name, index, size, false) {
      }

      public string Name { get; private set; }
      public int Index { get; private set; }
      public uint Size { get; private set; }
      public bool Pseudo { get; private set; }
    }

    #endregion

  
  }
}
