using System;
using System.Diagnostics;
using System.Reflection;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Project;

namespace Google.NaClVsx.ProjectSupport {
  public class ProjectPropertyAttribute : Attribute {
    public ProjectPropertyAttribute(string propertyName, bool perConfig) {
      PerConfig = perConfig;
      PropertyName = propertyName;
    }

    public string PropertyName { get; set; }
    public bool PerConfig { get; set; }
  }

  public class AutoSettingsPage : SettingsPage {
    #region Overrides of SettingsPage

    protected override void BindProperties() {
      Type derived = GetType();
      foreach (PropertyInfo prop in derived.GetProperties()) {
        object[] attributes = prop.GetCustomAttributes(
            typeof (ProjectPropertyAttribute), true);

        foreach (ProjectPropertyAttribute attribute in attributes) {
          string value = GetPropertyFromMsBuild(attribute);
          if (prop.PropertyType.IsEnum) {
            prop.SetValue(
                this, Enum.Parse(prop.PropertyType, value), null);
          } else {
            prop.SetValue(
                this,
                Convert.ChangeType(value, prop.PropertyType),
                null);
          }
        }
      }
      IsDirty = false;
    }

    protected override int ApplyChanges() {
      Type derived = GetType();
      foreach (PropertyInfo prop in derived.GetProperties()) {
        object[] attributes = prop.GetCustomAttributes(
            typeof (ProjectPropertyAttribute), true);

        foreach (ProjectPropertyAttribute attribute in attributes) {
          object value = prop.GetValue(this, null);
          string persistValue = "ERROR";
          if (prop.PropertyType.IsEnum) {
            persistValue = Enum.GetName(prop.PropertyType, value);
          } else {
            persistValue = Convert.ToString(value);
          }

          SetMsBuildProperty(attribute, persistValue);
        }
      }
      IsDirty = false;
      return VSConstants.S_OK;
    }

    #endregion

    // relative to active configuration.
    public string GetUnevaluatedConfigProperty(string propertyName) {
      if (ProjectMgr == null) {
        return String.Empty;
      }

      string unifiedResult = null;
      bool cacheNeedReset = true;

      ProjectConfig[] configurations = GetProjectConfigurations();

      for (int i = 0; i < configurations.Length; i++) {
        var nacl_config = configurations[i] as NaClProjectConfig;
        Debug.Assert(nacl_config != null);

        string property =
            nacl_config.GetRawConfigurationProperty(
                propertyName, cacheNeedReset);
        cacheNeedReset = false;

        if (property != null) {
          string text = property.Trim();

          if (i == 0) {
            unifiedResult = text;
          } else if (unifiedResult != text) {
            return ""; // tristate value is blank then
          }
        }
      }

      return unifiedResult;
    }

    #region Private Implementation

    private string GetPropertyFromMsBuild(ProjectPropertyAttribute attribute) {
      if (attribute.PerConfig) {
        return GetUnevaluatedConfigProperty(attribute.PropertyName);
      } else {
        return ProjectMgr.GetProjectProperty(attribute.PropertyName);
      }
    }

    private void SetMsBuildProperty(ProjectPropertyAttribute attribute,
                                    string persistValue) {
      if (attribute.PerConfig) {
        SetConfigProperty(attribute.PropertyName, persistValue);
      } else {
        ProjectMgr.SetProjectProperty(
            attribute.PropertyName, persistValue);
      }
    }

    #endregion
  }
}
