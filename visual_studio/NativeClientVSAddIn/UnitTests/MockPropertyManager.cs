// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;

  /// <summary>
  /// This class fakes the mechanism for reading and writing properties on property pages.
  /// </summary>
  public class MockPropertyManager : PropertyManager
  {
    /// <summary>
    /// Relays property get calls to the provided delegate.
    /// </summary>
    private PropertyGetter getter_;

    /// <summary>
    /// Relays property set calls to the provided delegate.
    /// </summary>
    private PropertySetter setter_;

    /// <summary>
    /// Constructs the property manager.
    /// </summary>
    /// <param name="platformType">The platform type to represent.</param>
    /// <param name="getter">Receives property get requests and returns mock values.</param>
    /// <param name="setter">Receives property set requests and checks them.</param>
    public MockPropertyManager(
        ProjectPlatformType platformType, PropertyGetter getter, PropertySetter setter)
    {
      this.ProjectPlatform = platformType;
      getter_ = getter;
      setter_ = setter;
    }

    /// <summary>
    /// Can be used to capture the property requests and return whatever value is desired.
    /// If this returns null then the test will throw an error that the property was unexpected.
    /// </summary>
    /// <param name="page">Property page name.</param>
    /// <param name="name">Property name.</param>
    /// <returns>Value to return. Should return null if property was unexpected.</returns>
    public delegate string PropertyGetter(string page, string name);

    /// <summary>
    /// Can be used to capture the property requests and set whatever value is desired or do checks.
    /// If this returns false then the test will throw an error that the property was unexpected.
    /// </summary>
    /// <param name="page">Property page name.</param>
    /// <param name="name">Property name.</param>
    /// <param name="value">Value to set.</param>
    /// <returns>True if the value was expected, false if unexpected (error).</returns>
    public delegate bool PropertySetter(string page, string name, string value);

    /// <summary>
    /// The full path to the output assembly.
    /// </summary>
    public override string PluginAssembly
    {
      get { return getter_("Property", "PluginAssembly"); }
      protected set { setter_("Property", "PluginAssembly", value); }
    }

    /// <summary>
    /// The main project directory.
    /// </summary>
    public override string ProjectDirectory
    {
      get { return getter_("Property", "ProjectDirectory"); }
      protected set { setter_("Property", "ProjectDirectory", value); }
    }

    /// <summary>
    /// The directory where the output assembly is placed.
    /// </summary>
    public override string OutputDirectory
    {
      get { return getter_("Property", "OutputDirectory"); }
      protected set { setter_("Property", "OutputDirectory", value); }
    }

    /// <summary>
    /// Reads any generic property from the current target properties.
    /// </summary>
    /// <param name="page">Name of the page where the property is located.</param>
    /// <param name="name">Name of the property.</param>
    /// <returns>Mock value of the property as returned by the getter_.</returns>
    public override string GetProperty(string page, string name)
    {
      string value = getter_(page, name);
      if (value == null)
      {
        throw new Exception(string.Format(
            "Property request not expected by test! Page: {0}, Prop: {1}", page, name));
      }

      return value;
    }

    /// <summary>
    /// Sets any generic property to the current target properties.
    /// </summary>
    /// <param name="page">Page where property is located.</param>
    /// <param name="name">Name of the property.</param>
    /// <param name="value">Unevaluated string value to set.</param>
    public override void SetProperty(string page, string name, string value)
    {
      if (!setter_(page, name, value))
      {
        throw new Exception(string.Format(
            "Property set request was not expected by test! Page {0}, Prop: {1}", page, name));
      }
    }
  }
}
