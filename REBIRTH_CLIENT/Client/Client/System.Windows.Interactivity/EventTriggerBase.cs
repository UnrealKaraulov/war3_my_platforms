// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.EventTriggerBase
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

using System.ComponentModel;
using System.Globalization;
using System.Reflection;

namespace System.Windows.Interactivity
{
    public abstract class EventTriggerBase : TriggerBase
    {
        public static readonly DependencyProperty SourceObjectProperty = DependencyProperty.Register(nameof(SourceObject), typeof(object), typeof(EventTriggerBase), new PropertyMetadata(new PropertyChangedCallback(EventTriggerBase.OnSourceObjectChanged)));
        public static readonly DependencyProperty SourceNameProperty = DependencyProperty.Register(nameof(SourceName), typeof(string), typeof(EventTriggerBase), new PropertyMetadata(new PropertyChangedCallback(EventTriggerBase.OnSourceNameChanged)));
        private Type sourceTypeConstraint;
        private bool isSourceChangedRegistered;
        private NameResolver sourceNameResolver;
        private MethodInfo eventHandlerMethodInfo;

        protected override sealed Type AssociatedObjectTypeConstraint
        {
            get
            {
                return TypeDescriptor.GetAttributes(this.GetType())[typeof(TypeConstraintAttribute)] is TypeConstraintAttribute attribute ? attribute.Constraint : typeof(DependencyObject);
            }
        }

        protected Type SourceTypeConstraint
        {
            get
            {
                return this.sourceTypeConstraint;
            }
        }

        public object SourceObject
        {
            get
            {
                return this.GetValue(EventTriggerBase.SourceObjectProperty);
            }
            set
            {
                this.SetValue(EventTriggerBase.SourceObjectProperty, value);
            }
        }

        public string SourceName
        {
            get
            {
                return (string)this.GetValue(EventTriggerBase.SourceNameProperty);
            }
            set
            {
                this.SetValue(EventTriggerBase.SourceNameProperty, (object)value);
            }
        }

        public object Source
        {
            get
            {
                object obj = (object)this.AssociatedObject;
                if (this.SourceObject != null)
                    obj = this.SourceObject;
                else if (this.IsSourceNameSet)
                {
                    obj = (object)this.SourceNameResolver.Object;
                    if (obj != null && !this.SourceTypeConstraint.IsAssignableFrom(obj.GetType()))
                        throw new InvalidOperationException(string.Format((IFormatProvider)CultureInfo.CurrentCulture, ExceptionStringTable.RetargetedTypeConstraintViolatedExceptionMessage, (object)this.GetType().Name, (object)obj.GetType(), (object)this.SourceTypeConstraint, (object)nameof(Source)));
                }
                return obj;
            }
        }

        private NameResolver SourceNameResolver
        {
            get
            {
                return this.sourceNameResolver;
            }
        }

        private bool IsSourceChangedRegistered
        {
            get
            {
                return this.isSourceChangedRegistered;
            }
            set
            {
                this.isSourceChangedRegistered = value;
            }
        }

        private bool IsSourceNameSet
        {
            get
            {
                return !string.IsNullOrEmpty(this.SourceName) || this.ReadLocalValue(EventTriggerBase.SourceNameProperty) != DependencyProperty.UnsetValue;
            }
        }

        private bool IsLoadedRegistered { get; set; }

        internal EventTriggerBase(Type sourceTypeConstraint)
          : base(typeof(DependencyObject))
        {
            this.sourceTypeConstraint = sourceTypeConstraint;
            this.sourceNameResolver = new NameResolver();
            this.RegisterSourceChanged();
        }

        protected abstract string GetEventName();

        protected virtual void OnEvent(EventArgs eventArgs)
        {
            this.InvokeActions((object)eventArgs);
        }

        private void OnSourceChanged(object oldSource, object newSource)
        {
            if (this.AssociatedObject == null)
                return;
            this.OnSourceChangedImpl(oldSource, newSource);
        }

        internal virtual void OnSourceChangedImpl(object oldSource, object newSource)
        {
            if (string.IsNullOrEmpty(this.GetEventName()) || string.Compare(this.GetEventName(), "Loaded", StringComparison.Ordinal) == 0)
                return;
            if (oldSource != null && this.SourceTypeConstraint.IsAssignableFrom(oldSource.GetType()))
                this.UnregisterEvent(oldSource, this.GetEventName());
            if (newSource == null || !this.SourceTypeConstraint.IsAssignableFrom(newSource.GetType()))
                return;
            this.RegisterEvent(newSource, this.GetEventName());
        }

        protected override void OnAttached()
        {
            base.OnAttached();
            DependencyObject associatedObject1 = this.AssociatedObject;
            Behavior behavior = associatedObject1 as Behavior;
            FrameworkElement frameworkElement = associatedObject1 as FrameworkElement;
            this.RegisterSourceChanged();
            if (behavior != null)
            {
                DependencyObject associatedObject2 = ((IAttachedObject)behavior).AssociatedObject;
                behavior.AssociatedObjectChanged += new EventHandler(this.OnBehaviorHostChanged);
            }
            else
            {
                if (this.SourceObject == null)
                {
                    if (frameworkElement != null)
                    {
                        this.SourceNameResolver.NameScopeReferenceElement = frameworkElement;
                        goto label_7;
                    }
                }
                try
                {
                    this.OnSourceChanged((object)null, this.Source);
                }
                catch (InvalidOperationException ex)
                {
                }
            }
            label_7:
            if (string.Compare(this.GetEventName(), "Loaded", StringComparison.Ordinal) != 0 || frameworkElement == null || Interaction.IsElementLoaded(frameworkElement))
                return;
            this.RegisterLoaded(frameworkElement);
        }

        protected override void OnDetaching()
        {
            base.OnDetaching();
            Behavior associatedObject1 = this.AssociatedObject as Behavior;
            FrameworkElement associatedObject2 = this.AssociatedObject as FrameworkElement;
            try
            {
                this.OnSourceChanged(this.Source, (object)null);
            }
            catch (InvalidOperationException ex)
            {
            }
            this.UnregisterSourceChanged();
            if (associatedObject1 != null)
                associatedObject1.AssociatedObjectChanged -= new EventHandler(this.OnBehaviorHostChanged);
            this.SourceNameResolver.NameScopeReferenceElement = (FrameworkElement)null;
            if (string.Compare(this.GetEventName(), "Loaded", StringComparison.Ordinal) != 0 || associatedObject2 == null)
                return;
            this.UnregisterLoaded(associatedObject2);
        }

        private void OnBehaviorHostChanged(object sender, EventArgs e)
        {
            this.SourceNameResolver.NameScopeReferenceElement = ((IAttachedObject)sender).AssociatedObject as FrameworkElement;
        }

        private static void OnSourceObjectChanged(
          DependencyObject obj,
          DependencyPropertyChangedEventArgs args)
        {
            EventTriggerBase eventTriggerBase = (EventTriggerBase)obj;
            object newSource = (object)eventTriggerBase.SourceNameResolver.Object;
            if (args.NewValue == null)
            {
                eventTriggerBase.OnSourceChanged(args.OldValue, newSource);
            }
            else
            {
                if (args.OldValue == null && newSource != null)
                    eventTriggerBase.UnregisterEvent(newSource, eventTriggerBase.GetEventName());
                eventTriggerBase.OnSourceChanged(args.OldValue, args.NewValue);
            }
        }

        private static void OnSourceNameChanged(
          DependencyObject obj,
          DependencyPropertyChangedEventArgs args)
        {
            ((EventTriggerBase)obj).SourceNameResolver.Name = (string)args.NewValue;
        }

        private void RegisterSourceChanged()
        {
            if (this.IsSourceChangedRegistered)
                return;
            this.SourceNameResolver.ResolvedElementChanged += new EventHandler<NameResolvedEventArgs>(this.OnSourceNameResolverElementChanged);
            this.IsSourceChangedRegistered = true;
        }

        private void UnregisterSourceChanged()
        {
            if (!this.IsSourceChangedRegistered)
                return;
            this.SourceNameResolver.ResolvedElementChanged -= new EventHandler<NameResolvedEventArgs>(this.OnSourceNameResolverElementChanged);
            this.IsSourceChangedRegistered = false;
        }

        private void OnSourceNameResolverElementChanged(object sender, NameResolvedEventArgs e)
        {
            if (this.SourceObject != null)
                return;
            this.OnSourceChanged(e.OldObject, e.NewObject);
        }

        private void RegisterLoaded(FrameworkElement associatedElement)
        {
            if (this.IsLoadedRegistered || associatedElement == null)
                return;
            associatedElement.Loaded += new RoutedEventHandler(this.OnEventImpl);
            this.IsLoadedRegistered = true;
        }

        private void UnregisterLoaded(FrameworkElement associatedElement)
        {
            if (!this.IsLoadedRegistered || associatedElement == null)
                return;
            associatedElement.Loaded -= new RoutedEventHandler(this.OnEventImpl);
            this.IsLoadedRegistered = false;
        }

        private void RegisterEvent(object obj, string eventName)
        {
            EventInfo eventInfo = obj.GetType().GetEvent(eventName);
            if (eventInfo == (EventInfo)null)
            {
                if (this.SourceObject != null)
                    throw new ArgumentException(string.Format((IFormatProvider)CultureInfo.CurrentCulture, ExceptionStringTable.EventTriggerCannotFindEventNameExceptionMessage, (object)eventName, (object)obj.GetType().Name));
            }
            else if (!EventTriggerBase.IsValidEvent(eventInfo))
            {
                if (this.SourceObject != null)
                    throw new ArgumentException(string.Format((IFormatProvider)CultureInfo.CurrentCulture, ExceptionStringTable.EventTriggerBaseInvalidEventExceptionMessage, (object)eventName, (object)obj.GetType().Name));
            }
            else
            {
                this.eventHandlerMethodInfo = typeof(EventTriggerBase).GetMethod("OnEventImpl", BindingFlags.Instance | BindingFlags.NonPublic);
                eventInfo.AddEventHandler(obj, Delegate.CreateDelegate(eventInfo.EventHandlerType, (object)this, this.eventHandlerMethodInfo));
            }
        }

        private static bool IsValidEvent(EventInfo eventInfo)
        {
            Type eventHandlerType = eventInfo.EventHandlerType;
            if (!typeof(Delegate).IsAssignableFrom(eventInfo.EventHandlerType))
                return false;
            ParameterInfo[] parameters = eventHandlerType.GetMethod("Invoke").GetParameters();
            return parameters.Length == 2 && typeof(object).IsAssignableFrom(parameters[0].ParameterType) && typeof(EventArgs).IsAssignableFrom(parameters[1].ParameterType);
        }

        private void UnregisterEvent(object obj, string eventName)
        {
            if (string.Compare(eventName, "Loaded", StringComparison.Ordinal) == 0)
            {
                if (!(obj is FrameworkElement associatedElement))
                    return;
                this.UnregisterLoaded(associatedElement);
            }
            else
                this.UnregisterEventImpl(obj, eventName);
        }

        private void UnregisterEventImpl(object obj, string eventName)
        {
            Type type = obj.GetType();
            if (this.eventHandlerMethodInfo == (MethodInfo)null)
                return;
            EventInfo eventInfo = type.GetEvent(eventName);
            eventInfo.RemoveEventHandler(obj, Delegate.CreateDelegate(eventInfo.EventHandlerType, (object)this, this.eventHandlerMethodInfo));
            this.eventHandlerMethodInfo = (MethodInfo)null;
        }

        private void OnEventImpl(object sender, EventArgs eventArgs)
        {
            this.OnEvent(eventArgs);
        }

        internal void OnEventNameChanged(string oldEventName, string newEventName)
        {
            if (this.AssociatedObject == null)
                return;
            var source = this.Source;
            if (this.Source is FrameworkElement && string.Compare(oldEventName, "Loaded", StringComparison.Ordinal) == 0)
                this.UnregisterLoaded(source as FrameworkElement);
            else if (!string.IsNullOrEmpty(oldEventName))
                this.UnregisterEvent(this.Source, oldEventName);
            if (source != null && string.Compare(newEventName, "Loaded", StringComparison.Ordinal) == 0)
            {
                this.RegisterLoaded(source as FrameworkElement);
            }
            else
            {
                if (string.IsNullOrEmpty(newEventName))
                    return;
                this.RegisterEvent(this.Source, newEventName);
            }
        }
    }
}
