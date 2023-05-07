// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.TargetedTriggerAction
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

using System.ComponentModel;
using System.Globalization;

namespace System.Windows.Interactivity
{
  public abstract class TargetedTriggerAction : TriggerAction
  {
    public static readonly DependencyProperty TargetObjectProperty = DependencyProperty.Register(nameof (TargetObject), typeof (object), typeof (TargetedTriggerAction), (PropertyMetadata) new FrameworkPropertyMetadata(new PropertyChangedCallback(TargetedTriggerAction.OnTargetObjectChanged)));
    public static readonly DependencyProperty TargetNameProperty = DependencyProperty.Register(nameof (TargetName), typeof (string), typeof (TargetedTriggerAction), (PropertyMetadata) new FrameworkPropertyMetadata(new PropertyChangedCallback(TargetedTriggerAction.OnTargetNameChanged)));
    private Type targetTypeConstraint;
    private bool isTargetChangedRegistered;
    private NameResolver targetResolver;

    public object TargetObject
    {
      get
      {
        return this.GetValue(TargetedTriggerAction.TargetObjectProperty);
      }
      set
      {
        this.SetValue(TargetedTriggerAction.TargetObjectProperty, value);
      }
    }

    public string TargetName
    {
      get
      {
        return (string) this.GetValue(TargetedTriggerAction.TargetNameProperty);
      }
      set
      {
        this.SetValue(TargetedTriggerAction.TargetNameProperty, (object) value);
      }
    }

    protected object Target
    {
      get
      {
        object obj = (object) this.AssociatedObject;
        if (this.TargetObject != null)
          obj = this.TargetObject;
        else if (this.IsTargetNameSet)
          obj = (object) this.TargetResolver.Object;
        if (obj != null && !this.TargetTypeConstraint.IsAssignableFrom(obj.GetType()))
          throw new InvalidOperationException(string.Format((IFormatProvider) CultureInfo.CurrentCulture, ExceptionStringTable.RetargetedTypeConstraintViolatedExceptionMessage, (object) this.GetType().Name, (object) obj.GetType(), (object) this.TargetTypeConstraint, (object) nameof (Target)));
        return obj;
      }
    }

    protected override sealed Type AssociatedObjectTypeConstraint
    {
      get
      {
        return TypeDescriptor.GetAttributes(this.GetType())[typeof (TypeConstraintAttribute)] is TypeConstraintAttribute attribute ? attribute.Constraint : typeof (DependencyObject);
      }
    }

    protected Type TargetTypeConstraint
    {
      get
      {
        this.ReadPreamble();
        return this.targetTypeConstraint;
      }
    }

    private bool IsTargetNameSet
    {
      get
      {
        return !string.IsNullOrEmpty(this.TargetName) || this.ReadLocalValue(TargetedTriggerAction.TargetNameProperty) != DependencyProperty.UnsetValue;
      }
    }

    private NameResolver TargetResolver
    {
      get
      {
        return this.targetResolver;
      }
    }

    private bool IsTargetChangedRegistered
    {
      get
      {
        return this.isTargetChangedRegistered;
      }
      set
      {
        this.isTargetChangedRegistered = value;
      }
    }

    internal TargetedTriggerAction(Type targetTypeConstraint)
      : base(typeof (DependencyObject))
    {
      this.targetTypeConstraint = targetTypeConstraint;
      this.targetResolver = new NameResolver();
      this.RegisterTargetChanged();
    }

    internal virtual void OnTargetChangedImpl(object oldTarget, object newTarget)
    {
    }

    protected override void OnAttached()
    {
      base.OnAttached();
      DependencyObject associatedObject = this.AssociatedObject;
      Behavior behavior = associatedObject as Behavior;
      this.RegisterTargetChanged();
      if (behavior != null)
      {
        associatedObject = ((IAttachedObject) behavior).AssociatedObject;
        behavior.AssociatedObjectChanged += new EventHandler(this.OnBehaviorHostChanged);
      }
      this.TargetResolver.NameScopeReferenceElement = associatedObject as FrameworkElement;
    }

    protected override void OnDetaching()
    {
      Behavior associatedObject = this.AssociatedObject as Behavior;
      base.OnDetaching();
      this.OnTargetChangedImpl((object) this.TargetResolver.Object, (object) null);
      this.UnregisterTargetChanged();
      if (associatedObject != null)
        associatedObject.AssociatedObjectChanged -= new EventHandler(this.OnBehaviorHostChanged);
      this.TargetResolver.NameScopeReferenceElement = (FrameworkElement) null;
    }

    private void OnBehaviorHostChanged(object sender, EventArgs e)
    {
      this.TargetResolver.NameScopeReferenceElement = ((IAttachedObject) sender).AssociatedObject as FrameworkElement;
    }

    private void RegisterTargetChanged()
    {
      if (this.IsTargetChangedRegistered)
        return;
      this.TargetResolver.ResolvedElementChanged += new EventHandler<NameResolvedEventArgs>(this.OnTargetChanged);
      this.IsTargetChangedRegistered = true;
    }

    private void UnregisterTargetChanged()
    {
      if (!this.IsTargetChangedRegistered)
        return;
      this.TargetResolver.ResolvedElementChanged -= new EventHandler<NameResolvedEventArgs>(this.OnTargetChanged);
      this.IsTargetChangedRegistered = false;
    }

    private static void OnTargetObjectChanged(
      DependencyObject obj,
      DependencyPropertyChangedEventArgs args)
    {
      ((TargetedTriggerAction) obj).OnTargetChanged((object) obj, new NameResolvedEventArgs(args.OldValue, args.NewValue));
    }

    private static void OnTargetNameChanged(
      DependencyObject obj,
      DependencyPropertyChangedEventArgs args)
    {
      ((TargetedTriggerAction) obj).TargetResolver.Name = (string) args.NewValue;
    }

    private void OnTargetChanged(object sender, NameResolvedEventArgs e)
    {
      if (this.AssociatedObject == null)
        return;
      this.OnTargetChangedImpl(e.OldObject, e.NewObject);
    }
  }
}
