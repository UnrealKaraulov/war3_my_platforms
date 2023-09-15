// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.EventObserver
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

using System.Reflection;

namespace System.Windows.Interactivity
{
  public sealed class EventObserver : IDisposable
  {
    private EventInfo eventInfo;
    private object target;
    private Delegate handler;

    public EventObserver(EventInfo eventInfo, object target, Delegate handler)
    {
      if (eventInfo == (EventInfo) null)
        throw new ArgumentNullException(nameof (eventInfo));
      if ((object) handler == null)
        throw new ArgumentNullException(nameof (handler));
      this.eventInfo = eventInfo;
      this.target = target;
      this.handler = handler;
      this.eventInfo.AddEventHandler(this.target, handler);
    }

    public void Dispose()
    {
      this.eventInfo.RemoveEventHandler(this.target, this.handler);
    }
  }
}
