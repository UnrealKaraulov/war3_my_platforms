// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.EventTriggerBase`1
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public abstract class EventTriggerBase<T> : EventTriggerBase where T : class
  {
    protected EventTriggerBase()
      : base(typeof (T))
    {
    }

    public T Source
    {
      get
      {
        return (T) base.Source;
      }
    }

    internal override sealed void OnSourceChangedImpl(object oldSource, object newSource)
    {
      base.OnSourceChangedImpl(oldSource, newSource);
      this.OnSourceChanged(oldSource as T, newSource as T);
    }

    protected virtual void OnSourceChanged(T oldSource, T newSource)
    {
    }
  }
}
