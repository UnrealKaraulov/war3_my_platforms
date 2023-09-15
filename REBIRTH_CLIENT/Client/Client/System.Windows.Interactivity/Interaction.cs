// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.Interaction
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public static class Interaction
  {
    private static readonly DependencyProperty TriggersProperty = DependencyProperty.RegisterAttached("ShadowTriggers", typeof (TriggerCollection), typeof (Interaction), (PropertyMetadata) new FrameworkPropertyMetadata(new PropertyChangedCallback(Interaction.OnTriggersChanged)));
    private static readonly DependencyProperty BehaviorsProperty = DependencyProperty.RegisterAttached("ShadowBehaviors", typeof (BehaviorCollection), typeof (Interaction), (PropertyMetadata) new FrameworkPropertyMetadata(new PropertyChangedCallback(Interaction.OnBehaviorsChanged)));

    internal static bool ShouldRunInDesignMode { get; set; }

    public static TriggerCollection GetTriggers(DependencyObject obj)
    {
      TriggerCollection triggerCollection = (TriggerCollection) obj.GetValue(Interaction.TriggersProperty);
      if (triggerCollection == null)
      {
        triggerCollection = new TriggerCollection();
        obj.SetValue(Interaction.TriggersProperty, (object) triggerCollection);
      }
      return triggerCollection;
    }

    public static BehaviorCollection GetBehaviors(DependencyObject obj)
    {
      BehaviorCollection behaviorCollection = (BehaviorCollection) obj.GetValue(Interaction.BehaviorsProperty);
      if (behaviorCollection == null)
      {
        behaviorCollection = new BehaviorCollection();
        obj.SetValue(Interaction.BehaviorsProperty, (object) behaviorCollection);
      }
      return behaviorCollection;
    }

    private static void OnBehaviorsChanged(
      DependencyObject obj,
      DependencyPropertyChangedEventArgs args)
    {
      BehaviorCollection oldValue = (BehaviorCollection) args.OldValue;
      BehaviorCollection newValue = (BehaviorCollection) args.NewValue;
      if (oldValue == newValue)
        return;
      if (oldValue != null && ((IAttachedObject) oldValue).AssociatedObject != null)
        oldValue.Detach();
      if (newValue == null || obj == null)
        return;
      if (((IAttachedObject) newValue).AssociatedObject != null)
        throw new InvalidOperationException(ExceptionStringTable.CannotHostBehaviorCollectionMultipleTimesExceptionMessage);
      newValue.Attach(obj);
    }

    private static void OnTriggersChanged(
      DependencyObject obj,
      DependencyPropertyChangedEventArgs args)
    {
      TriggerCollection oldValue = args.OldValue as TriggerCollection;
      TriggerCollection newValue = args.NewValue as TriggerCollection;
      if (oldValue == newValue)
        return;
      if (oldValue != null && ((IAttachedObject) oldValue).AssociatedObject != null)
        oldValue.Detach();
      if (newValue == null || obj == null)
        return;
      if (((IAttachedObject) newValue).AssociatedObject != null)
        throw new InvalidOperationException(ExceptionStringTable.CannotHostTriggerCollectionMultipleTimesExceptionMessage);
      newValue.Attach(obj);
    }

    internal static bool IsElementLoaded(FrameworkElement element)
    {
      return element.IsLoaded;
    }
  }
}
