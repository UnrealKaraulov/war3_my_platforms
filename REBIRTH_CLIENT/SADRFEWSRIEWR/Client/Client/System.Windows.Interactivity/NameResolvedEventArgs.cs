// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.NameResolvedEventArgs
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  internal sealed class NameResolvedEventArgs : EventArgs
  {
    private object oldObject;
    private object newObject;

    public object OldObject
    {
      get
      {
        return this.oldObject;
      }
    }

    public object NewObject
    {
      get
      {
        return this.newObject;
      }
    }

    public NameResolvedEventArgs(object oldObject, object newObject)
    {
      this.oldObject = oldObject;
      this.newObject = newObject;
    }
  }
}
