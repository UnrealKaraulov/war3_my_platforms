// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.TriggerActionCollection
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public class TriggerActionCollection : AttachableCollection<TriggerAction>
  {
    internal TriggerActionCollection()
    {
    }

    protected override void OnAttached()
    {
      foreach (TriggerAction triggerAction in (FreezableCollection<TriggerAction>) this)
        triggerAction.Attach(this.AssociatedObject);
    }

    protected override void OnDetaching()
    {
      foreach (TriggerAction triggerAction in (FreezableCollection<TriggerAction>) this)
        triggerAction.Detach();
    }

    internal override void ItemAdded(TriggerAction item)
    {
      if (item.IsHosted)
        throw new InvalidOperationException(ExceptionStringTable.CannotHostTriggerActionMultipleTimesExceptionMessage);
      if (this.AssociatedObject != null)
        item.Attach(this.AssociatedObject);
      item.IsHosted = true;
    }

    internal override void ItemRemoved(TriggerAction item)
    {
      if (((IAttachedObject) item).AssociatedObject != null)
        item.Detach();
      item.IsHosted = false;
    }

    protected override Freezable CreateInstanceCore()
    {
      return (Freezable) new TriggerActionCollection();
    }
  }
}
