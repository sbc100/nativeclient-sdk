using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Microsoft.Build.Framework;
using System.Xaml;
using Microsoft.Build.Framework.XamlTypes;
using Microsoft.Build.Utilities;

namespace NaCl.Build.CPPTasks
{
    class XamlParser
    {
        public XamlParser(string path)
        {
            // load and store properties from xaml file
            m_parsedBuildRule = (Rule)XamlServices.Load(path);

            // NOTE:
            // There are MSBuild classes which support command line building,
            // argument switch encapsulation and more.  Code within VCToolTask,
            // a hidden interface, uses some these classes to generate command line
            // switches given project settings. As the VCToolTask class is a hidden
            // class and the MSBuild documentation is very sparse, we are going
            // with a small custom solution.  For reference see the
            // Microsoft.Build.Tasks.Xaml, Microsoft.Build.Framework.XamlTypes namespaces.

            // move the properties to a property name keyed dictionary for faster lookup
            ToolProperties = new Dictionary<string, PropertyWrapper>();
            ToolProperties = m_parsedBuildRule.Properties.ToDictionary(x => x.Name, x => (new PropertyWrapper(x.GetType(), x)));

            InitFunctionMap();
        }

        private void InitFunctionMap()
        {
            m_typeFunctionMap = new Dictionary<Type, Action<CommandLineBuilder, BaseProperty, string>>
            {
                { typeof(StringListProperty), GenerateArgumentStringList },
                { typeof(StringProperty), GenerateArgumentString },
                { typeof(IntProperty), GenerateArgumentInt },
                { typeof(BoolProperty), GenerateArgumentBool },
                { typeof(EnumProperty), GenerateArgumentEnum }
            };
        }

        public string Parse(ITaskItem taskItem, bool fullOutputName)
        {
            CommandLineBuilder builder = new CommandLineBuilder();

            foreach (string name in taskItem.MetadataNames)
            {
                string value = taskItem.GetMetadata(name);
                if (fullOutputName && name == "ObjectFileName")
                {
                    if ((File.GetAttributes(value) & FileAttributes.Directory) != 0)
                    {
                        value = Path.Combine(value, Path.GetFileName(taskItem.ItemSpec));
                        value = Path.ChangeExtension(value, ".obj");
                    }
                }
                AppendArgumentForProperty(builder, name, value);
            }

            string result = builder.ToString();
            result = result.Replace('\\', '/'); // posix paths
            return result;
        }

        private string AppendArgumentForProperty(CommandLineBuilder builder, string name, string value)
        {
            PropertyWrapper current;
            ToolProperties.TryGetValue(name, out current);
            if (current != null && value.Length > 0)
            {
                // call appropriate function for type
                m_typeFunctionMap[current.PropertyType](builder, current.Property, value);
            }
            return string.Empty;
        }

        private void GenerateArgumentEnum(CommandLineBuilder builder, BaseProperty property, string value)
        {
            var result = ((EnumProperty)property).AdmissibleValues.Find(x => (x.Name == value));
            if (result != null)
            {
                builder.AppendSwitchUnquotedIfNotNull(m_parsedBuildRule.SwitchPrefix, result.Switch);
            }
        }

        // helper for string-based properties
        private void AppendStringValue(CommandLineBuilder builder, BaseProperty property, string subtype, string value)
        {
            value = value.Trim();

            // could cache this SubType test off in property wrapper or somewhere if performance were an issue
            string switchName = m_parsedBuildRule.SwitchPrefix + property.Switch;
            switchName += property.Separator;
            if (subtype == "file" || subtype == "folder")
            {
                // for switches that contains files or folders we need quoting
                builder.AppendSwitchIfNotNull(switchName, value);
            }
            else if (!string.IsNullOrEmpty(property.Switch))
            {
                builder.AppendSwitchUnquotedIfNotNull(switchName, value);
            }
            else if (!string.IsNullOrEmpty(value))
            {
                // for non-switches such as AdditionalOptions we just append the value
                builder.AppendTextUnquoted(" " + value);
            }
        }

        private void GenerateArgumentStringList(CommandLineBuilder builder, BaseProperty property, string value)
        {
            string[] arguments = value.Split(';');

            foreach (string argument in arguments)
            {
                if (argument.Length > 0)
                {
                    StringListProperty casted = (StringListProperty)property;
                    AppendStringValue(builder, property, casted.Subtype, argument);
                }
            }
        }

        private void GenerateArgumentString(CommandLineBuilder builder, BaseProperty property, string value)
        {
            // could cache this SubType test off in property wrapper or somewhere if performance were an issue
            StringProperty casted = (StringProperty)property;
            AppendStringValue(builder, property, casted.Subtype, value);
        }

        private void GenerateArgumentInt(CommandLineBuilder builder, BaseProperty property, string value)
        {
            // Currently we only have one Int property and it doesn't correspond to a
            // command line arguemnt (ProcessorNumber) so we ignore it here.
        }

        private void GenerateArgumentBool(CommandLineBuilder builder, BaseProperty property, string value)
        {
            if (value == "true")
            {
                builder.AppendSwitchUnquotedIfNotNull(m_parsedBuildRule.SwitchPrefix, property.Switch);
            }
            else if (value == "false" && ((BoolProperty)property).ReverseSwitch != null)
            {
                builder.AppendSwitchUnquotedIfNotNull(m_parsedBuildRule.SwitchPrefix, ((BoolProperty)property).ReverseSwitch);
            }
        }

        // to store off MSBuild property type and allow faster searching on them
        private class PropertyWrapper
        {
            public PropertyWrapper(Type newType, BaseProperty newProperty)
            {
                PropertyType = newType;
                Property = newProperty;
            }
            public Type PropertyType { get; set; }
            public BaseProperty Property { get; set; }
        } // class

        private Rule m_parsedBuildRule;
        private Dictionary<string, PropertyWrapper> ToolProperties { get; set; }

        // function mapping for easy property function calling
        private Dictionary<Type, Action<CommandLineBuilder, BaseProperty, string>> m_typeFunctionMap;
    } // XamlParser
} // namespace NaCl.Build.CPPTasks

