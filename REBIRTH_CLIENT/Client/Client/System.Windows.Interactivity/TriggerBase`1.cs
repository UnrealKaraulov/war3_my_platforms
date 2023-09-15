// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.TriggerBase`1
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public abstract class TriggerBase<T> : TriggerBase where T : DependencyObject
  {
    protected TriggerBase()
      : base(typeof (T))
    {
    }

    protected T AssociatedObject
    {
      get
      {
        return (T) base.AssociatedObject;
      }
    }

    protected override sealed Type AssociatedObjectTypeConstraint
    {
      get
      {
        return base.AssociatedObjectTypeConstraint;
      }
    }
  }
}
