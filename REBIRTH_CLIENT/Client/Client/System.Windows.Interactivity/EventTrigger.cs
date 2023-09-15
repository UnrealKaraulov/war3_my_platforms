// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.EventTrigger
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

namespace System.Windows.Interactivity
{
  public class EventTrigger : EventTriggerBase<object>
  {
    public static readonly DependencyProperty EventNameProperty = DependencyProperty.Register(nameof (EventName), typeof (string), typeof (EventTrigger), (PropertyMetadata) new FrameworkPropertyMetadata((object) "Loaded", new PropertyChangedCallback(EventTrigger.OnEventNameChanged)));

    public EventTrigger()
    {
    }

    public EventTrigger(string eventName)
    {
      this.EventName = eventName;
    }

    public string EventName
    {
      get
      {
        return (string) this.GetValue(EventTrigger.EventNameProperty);
      }
      set
      {
        this.SetValue(EventTrigger.EventNameProperty, (object) value);
      }
    }

    protected override string GetEventName()
    {
      return this.EventName;
    }

    private static void OnEventNameChanged(object sender, DependencyPropertyChangedEventArgs args)
    {
      ((EventTriggerBase) sender).OnEventNameChanged((string) args.OldValue, (string) args.NewValue);
    }
  }
}
