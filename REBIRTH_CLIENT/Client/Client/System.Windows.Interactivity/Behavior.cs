// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.Behavior
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

using System.Globalization;
using System.Windows.Media.Animation;

namespace System.Windows.Interactivity
{
  public abstract class Behavior : Animatable, IAttachedObject
  {
    private Type associatedType;
    private DependencyObject associatedObject;

    internal event EventHandler AssociatedObjectChanged;

    protected Type AssociatedType
    {
      get
      {
        this.ReadPreamble();
        return this.associatedType;
      }
    }

    protected DependencyObject AssociatedObject
    {
      get
      {
        this.ReadPreamble();
        return this.associatedObject;
      }
    }

    internal Behavior(Type associatedType)
    {
      this.associatedType = associatedType;
    }

    protected virtual void OnAttached()
    {
    }

    protected virtual void OnDetaching()
    {
    }

    protected override Freezable CreateInstanceCore()
    {
      return (Freezable) Activator.CreateInstance(this.GetType());
    }

    private void OnAssociatedObjectChanged()
    {
      if (this.AssociatedObjectChanged == null)
        return;
      this.AssociatedObjectChanged((object) this, new EventArgs());
    }

    DependencyObject IAttachedObject.AssociatedObject
    {
      get
      {
        return this.AssociatedObject;
      }
    }

    public void Attach(DependencyObject dependencyObject)
    {
      if (dependencyObject == this.AssociatedObject)
        return;
      if (this.AssociatedObject != null)
        throw new InvalidOperationException(ExceptionStringTable.CannotHostBehaviorMultipleTimesExceptionMessage);
      if (dependencyObject != null && !this.AssociatedType.IsAssignableFrom(dependencyObject.GetType()))
        throw new InvalidOperationException(string.Format((IFormatProvider) CultureInfo.CurrentCulture, ExceptionStringTable.TypeConstraintViolatedExceptionMessage, (object) this.GetType().Name, (object) dependencyObject.GetType().Name, (object) this.AssociatedType.Name));
      this.WritePreamble();
      this.associatedObject = dependencyObject;
      this.WritePostscript();
      this.OnAssociatedObjectChanged();
      this.OnAttached();
    }

    public void Detach()
    {
      this.OnDetaching();
      this.WritePreamble();
      this.associatedObject = (DependencyObject) null;
      this.WritePostscript();
      this.OnAssociatedObjectChanged();
    }
  }
}
