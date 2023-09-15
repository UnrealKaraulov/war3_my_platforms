// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.TargetedTriggerAction`1
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public abstract class TargetedTriggerAction<T> : TargetedTriggerAction where T : class
  {
    protected TargetedTriggerAction()
      : base(typeof (T))
    {
    }

    protected T Target
    {
      get
      {
        return (T) base.Target;
      }
    }

    internal override sealed void OnTargetChangedImpl(object oldTarget, object newTarget)
    {
      base.OnTargetChangedImpl(oldTarget, newTarget);
      this.OnTargetChanged(oldTarget as T, newTarget as T);
    }

    protected virtual void OnTargetChanged(T oldTarget, T newTarget)
    {
    }
  }
}
