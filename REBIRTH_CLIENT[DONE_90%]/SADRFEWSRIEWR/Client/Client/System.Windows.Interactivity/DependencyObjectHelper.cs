// Decompiled with JetBrains decompiler
// Type: System.Windows.Interactivity.DependencyObjectHelper
// Assembly: System.Windows.Interactivity, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35
// MVID: AAE9A92C-FB4A-4427-A2C1-2E6256CD1F02
// Assembly location: C:\Projects\AnotherWarcraftServer\Client\Release\System.Windows.Interactivity.dll

using System.Collections.Generic;
using System.Windows.Media;

namespace System.Windows.Interactivity
{
  public static class DependencyObjectHelper
  {
    public static IEnumerable<DependencyObject> GetSelfAndAncestors(
      this DependencyObject dependencyObject)
    {
      for (; dependencyObject != null; dependencyObject = VisualTreeHelper.GetParent(dependencyObject))
        yield return dependencyObject;
    }
  }
}
