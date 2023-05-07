// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.BehaviorCollection
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public sealed class BehaviorCollection : AttachableCollection<Behavior>
  {
    internal BehaviorCollection()
    {
    }

    protected override void OnAttached()
    {
      foreach (Behavior behavior in (FreezableCollection<Behavior>) this)
        behavior.Attach(this.AssociatedObject);
    }

    protected override void OnDetaching()
    {
      foreach (Behavior behavior in (FreezableCollection<Behavior>) this)
        behavior.Detach();
    }

    internal override void ItemAdded(Behavior item)
    {
      if (this.AssociatedObject == null)
        return;
      item.Attach(this.AssociatedObject);
    }

    internal override void ItemRemoved(Behavior item)
    {
      if (((IAttachedObject) item).AssociatedObject == null)
        return;
      item.Detach();
    }

    protected override Freezable CreateInstanceCore()
    {
      return (Freezable) new BehaviorCollection();
    }
  }
}
