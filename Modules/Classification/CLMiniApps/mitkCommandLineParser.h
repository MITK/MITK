/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

#ifndef __mitkCommandLineParser_h
#define __mitkCommandLineParser_h

#include <usAny.h>
#include <map>


/**
 * \ingroup Core
 *
 * The CTK command line parser.
 *
 * Use this class to add information about the command line arguments
 * your program understands and to easily parse them from a given list
 * of strings.
 *
 * This parser provides the following features:
 *
 * <ul>
 * <li>Add arguments by supplying a long name and/or a short name.
 *     Arguments are validated using a regular expression. They can have
 *     a default value and a help string.</li>
 * <li>Deprecated arguments.</li>
 * <li>Custom regular expressions for argument validation.</li>
 * <li>Set different argument name prefixes for native platform look and feel.</li>
 * <li>QSettings support. Default values for arguments can be read from
 *     a QSettings object.</li>
 * <li>Create a help text for the command line arguments with support for
 *     grouping arguments.</li>
 * </ul>
 *
 * Here is an example how to use this class inside a main function:
 *
 * \code
 * #include <ctkCommandLineParser.h>
 * #include <QCoreApplication>
 * #include <QTextStream>
 *
 * int main(int argc, char** argv)
 * {
 *   QCoreApplication app(argc, argv);
 *   // This is used by QSettings
 *   QCoreApplication::setOrganizationName("MyOrg");
 *   QCoreApplication::setApplicationName("MyApp");
 *
 *   ctkCommandLineParser parser;
 *   // Use Unix-style argument names
 *   parser.setArgumentPrefix("--", "-");
 *   // Enable QSettings support
 *   parser.enableSettings("disable-settings");
 *
 *   // Add command line argument names
 *   parser.addArgument("disable-settings", "", us::Any::Bool, "Do not use QSettings");
 *   parser.addArgument("help", "h", us::Any::Bool, "Show this help text");
 *   parser.addArgument("search-paths", "s", us::Any::StringList, "A list of paths to search");
 *
 *   // Parse the command line arguments
 *   bool ok = false;
 *   map<string, us::Any> parsedArgs = parser.parseArguments(QCoreApplication::arguments(), &ok);
 *   if (!ok)
 *   {
 *     QTextStream(stderr, QIODevice::WriteOnly) << "Error parsing arguments: "
 *                                               << parser.errorString() << "\n";
 *     return EXIT_FAILURE;
 *   }
 *
 *   // Show a help message
 *   if (parsedArgs.contains("help") || parsedArgs.contains("h"))
 *   {
 *     QTextStream(stdout, QIODevice::WriteOnly) << parser.helpText();
 *     return EXIT_SUCCESS;
 *   }
 *
 *   // Do something
 *
 *   return EXIT_SUCCESS;
 * }
 * \endcode
 */

using namespace std;

class mitkCommandLineParser
{

public:

    enum Type {
        String = 0,
        Bool = 1,
        StringList = 2,
        Int = 3,
        Float = 4,
        InputDirectory = 5,
        InputFile = 6,
        OutputDirectory = 7,
        OutputFile = 8,
        InputImage = 9
    };

    typedef std::vector< std::string > StringContainerType;

    /**
   * Constructs a parser instance.
   *
   * If QSettings support is enabled by a call to <code>enableSettings()</code>
   * a default constructed QSettings instance will be used when parsing
   * the command line arguments. Make sure to call <code>QCoreApplication::setOrganizationName()</code>
   * and <code>QCoreApplication::setApplicationName()</code> before using default
   * constructed QSettings objects.
   *
   * @param newParent The QObject parent.
   */
    mitkCommandLineParser();

    ~mitkCommandLineParser();

    /**
   * Parse a given list of command line arguments.
   *
   * This method parses a list of string elements considering the known arguments
   * added by calls to <code>addArgument()</code>. If any one of the argument
   * values does not match the corresponding regular expression,
   * <code>ok</code> is set to false and an empty map object is returned.
   *
   * The keys in the returned map object correspond to the long argument string,
   * if it is not empty. Otherwise, the short argument string is used as key. The
   * us::Any values can safely be converted to the type specified in the
   * <code>addArgument()</code> method call.
   *
   * @param arguments A StringContainerType containing command line arguments. Usually
   *        given by <code>QCoreApplication::arguments()</code>.
   * @param ok A pointer to a boolean variable. Will be set to <code>true</code>
   *        if all regular expressions matched, <code>false</code> otherwise.
   * @return A map object mapping the long argument (if empty, the short one)
   *         to a us::Any containing the value.
   */

    map<string, us::Any> parseArguments(const StringContainerType &arguments, bool* ok = 0);

    /**
    * Convenient method allowing to parse a given list of command line arguments.
    * @see parseArguments(const StringContainerType &, bool*)
    */
    map<string, us::Any> parseArguments(int argc, char** argv, bool* ok = 0);

    /**
   * Returns a detailed error description if a call to <code>parseArguments()</code>
   * failed.
   *
   * @return The error description, empty if no error occured.
   * @see parseArguments(const StringContainerType&, bool*)
   */
    string errorString() const;

    /**
   * This method returns all unparsed arguments, i.e. all arguments
   * for which no long or short name has been registered via a call
   * to <code>addArgument()</code>.
   *
   * @see addArgument()
   *
   * @return A list containing unparsed arguments.
   */
    const StringContainerType& unparsedArguments() const;

    /**
   * Checks if the given argument has been added via a call
   * to <code>addArgument()</code>.
   *
   * @see addArgument()
   *
   * @param argument The argument to be checked.
   * @return <code>true</code> if the argument was added, <code>false</code>
   *         otherwise.
   */
    bool argumentAdded(const string& argument) const;

    /**
   * Checks if the given argument has been parsed successfully by a previous
   * call to <code>parseArguments()</code>.
   *
   * @param argument The argument to be checked.
   * @return <code>true</code> if the argument was parsed, <code>false</code>
   *         otherwise.
   */
    bool argumentParsed(const string& argument) const;

    /**
   * Adds a command line argument. An argument can have a long name
   * (like --long-argument-name), a short name (like -l), or both. The type
   * of the argument can be specified by using the <code>type</code> parameter.
   * The following types are supported:
   *
   * <table>
   * <tr><td><b>Type</b></td><td><b># of parameters</b></td><td><b>Default regular expr</b></td>
   *        <td><b>Example</b></td></tr>
   * <tr><td>us::Any::String</td><td>1</td><td>.*</td><td>--test-string StringParameter</td></tr>
   * <tr><td>us::Any::Bool</td><td>0</td><td>does not apply</td><td>--enable-something</td></tr>
   * <tr><td>us::Any::StringList</td><td>-1</td><td>.*</td><td>--test-list string1 string2</td></tr>
   * <tr><td>us::Any::Int</td><td>1</td><td>-?[0-9]+</td><td>--test-int -5</td></tr>
   * </table>
   *
   * The regular expressions are used to validate the parameters of command line
   * arguments. You can restrict the valid set of parameters by calling
   * <code>setExactMatchRegularExpression()</code> for your argument.
   *
   * Optionally, a help string and a default value can be provided for the argument. If
   * the us::Any type of the default value does not match <code>type</code>, an
   * exception is thrown. Arguments with default values are always returned by
   * <code>parseArguments()</code>.
   *
   * You can also declare an argument deprecated, by setting <code>deprecated</code>
   * to <code>true</code>. Alternatively you can add a deprecated argument by calling
   * <code>addDeprecatedArgument()</code>.
   *
   * If the long or short argument has already been added, or if both are empty strings,
   * the method call has no effect.
   *
   * @param longarg The long argument name.
   * @param shortarg The short argument name.
   * @param type The argument type (see the list above for supported types).
   * @param argLabel The label of this argument, when auto generated interface is used.
   * @param argHelp A help string describing the argument.
   * @param defaultValue A default value for the argument.
   * @param ignoreRest All arguments after the current one will be ignored.
   * @param deprecated Declares the argument deprecated.
   *
   * @see setExactMatchRegularExpression()
   * @see addDeprecatedArgument()
   * @throws std::logic_error If the us::Any type of <code>defaultValue</code>
   *         does not match <code>type</code>, a <code>std::logic_error</code> is thrown.
   */
    void addArgument(const string& longarg, const string& shortarg,
                     Type type, const string& argLabel, const string& argHelp = string(),
                     const us::Any& defaultValue = us::Any(), bool optional=true,
                     bool ignoreRest = false, bool deprecated = false);

    /**
   * Adds a deprecated command line argument. If a deprecated argument is provided
   * on the command line, <code>argHelp</code> is displayed in the console and
   * processing continues with the next argument.
   *
   * Deprecated arguments are grouped separately at the end of the help text
   * returned by <code>helpText()</code>.
   *
   * @param longarg The long argument name.
   * @param shortarg The short argument name.
   * @param argHelp A help string describing alternatives to the deprecated argument.
   */
    void addDeprecatedArgument(const string& longarg, const string& shortarg, const string& argLabel,
                               const string& argHelp);

    /**
   * Sets a custom regular expression for validating argument parameters. The method
   * <code>errorString()</code> can be used the get the last error description.
   *
   * @param argument The previously added long or short argument name.
   * @param expression A regular expression which the arugment parameters must match.
   * @param exactMatchFailedMessage An error message explaining why the parameter did
   *        not match.
   *
   * @return <code>true</code> if the argument was found and the regular expression was set,
   *         <code>false</code> otherwise.
   *
   * @see errorString()
   */
    bool setExactMatchRegularExpression(const string& argument, const string& expression,
                                        const string& exactMatchFailedMessage);

    /**
   * The field width for the argument names without the help text.
   *
   * @return The argument names field width in the help text.
   */
    int fieldWidth() const;

    /**
   * Creates a help text containing properly formatted argument names and help strings
   * provided by calls to <code>addArgument()</code>. The arguments can be grouped by
   * using <code>beginGroup()</code> and <code>endGroup()</code>.
   *
   * @param charPad The padding character.
   * @return The formatted help text.
   */
    string helpText() const;

    /**
   * Sets the argument prefix for long and short argument names. This can be used
   * to create native command line arguments without changing the calls to
   * <code>addArgument()</code>. For example on Unix-based systems, long argument
   * names start with "--" and short names with "-", while on Windows argument names
   * always start with "/".
   *
   * Note that all methods in ctkCommandLineParser which take an argument name
   * expect the name as it was supplied to <code>addArgument</code>.
   *
   * Example usage:
   *
   * \code
   * ctkCommandLineParser parser;
   * parser.setArgumentPrefix("--", "-");
   * parser.addArgument("long-argument", "l", us::Any::String);
   * StringContainerType args;
   * args << "program name" << "--long-argument Hi";
   * parser.parseArguments(args);
   * \endcode
   *
   * @param longPrefix The prefix for long argument names.
   * @param shortPrefix The prefix for short argument names.
   */
    void setArgumentPrefix(const string& longPrefix, const string& shortPrefix);

    /**
   * Begins a new group for documenting arguments. All newly added arguments via
   * <code>addArgument()</code> will be put in the new group. You can close the
   * current group by calling <code>endGroup()</code> or be opening a new group.
   *
   * Note that groups cannot be nested and all arguments which do not belong to
   * a group will be listed at the top of the text created by <code>helpText()</code>.
   *
   * @param description The description of the group
   */
    void beginGroup(const string& description);

    /**
   * Ends the current group.
   *
   * @see beginGroup(const string&)
   */
    void endGroup();

    /**
   * Enables QSettings support in ctkCommandLineParser. If an argument name is found
   * in the QSettings instance with a valid us::Any, the value is considered as
   * a default value and overwrites default values registered with
   * <code>addArgument()</code>. User supplied values on the command line overwrite
   * values in the QSettings instance, except for arguments with multiple parameters
   * which are merged with QSettings values. Call <code>mergeSettings(false)</code>
   * to disable merging.
   *
   * See <code>ctkCommandLineParser(QSettings*)</code> for information about how to
   * supply a QSettings instance.
   *
   * Additionally, a long and short argument name can be specified which will disable
   * QSettings support if supplied on the command line. The argument name must be
   * registered as a regular argument via <code>addArgument()</code>.
   *
   * @param disableLongArg Long argument name.
   * @param disableShortArg Short argument name.
   *
   * @see ctkCommandLineParser(QSettings*)
   */
    void enableSettings(const string& disableLongArg = "",
                        const string& disableShortArg = "");

    /**
   * Controlls the merging behavior of user values and QSettings values.
   *
   * If merging is on (the default), user supplied values for an argument
   * which can take more than one parameter are merged with values stored
   * in the QSettings instance. If merging is off, the user values overwrite
   * the QSettings values.
   *
   * @param merge <code>true</code> enables QSettings merging, <code>false</code>
   *        disables it.
   */
    void mergeSettings(bool merge);

    /**
   * Can be used to check if QSettings support has been enabled by a call to
   * <code>enableSettings()</code>.
   *
   * @return <code>true</code> if QSettings support is enabled, <code>false</code>
   *         otherwise.
   */
    bool settingsEnabled() const;


    /**
    * Can be used to teach the parser to stop parsing the arguments and return False when
    * an unknown argument is encountered. By default <code>StrictMode</code> is disabled.
    *
    * @see parseArguments(const StringContainerType &, bool*)
    */
    void setStrictModeEnabled(bool strictMode);

    /**
     * Is used to generate an XML output for any commandline program.
     */
    void generateXmlOutput();

    /**
     * Is used to set the title of the auto generated interface.
     *
     * @param title The title of the app.
     */
    void setTitle(std::string title);
    /**
     * Is used to set the contributor for the help view in the auto generated interface.
     *
     * @param contributor Contributor of the app.
     */
    void setContributor(std::string contributor);
    /**
     * Is used to categorize the apps in the commandline module.
     *
     * @param category The category of the app.
     */
    void setCategory(std::string category);
    /**
     * Is used as the help text in the auto generated interface.
     *
     * @param description A short description for the app.
     */
    void setDescription(std::string description);
    /**
     * Is used to group several Parameters in one groupbox in the auto generated interface.
     * Default name is "Parameters", with the tooltip: "Groupbox containing parameters."
     *
     * To change the group of several arguments, call this method before the arguments are added.
     *
     * @param name The name of the groupbox.
     * @param tooltip The tooltip of the groupbox.
     */
    void changeParameterGroup(std::string name, std::string tooltip);

private:
    class ctkInternal;
    ctkInternal * Internal;

    string Title;
    string Contributor;
    string Category;
    string Description;
    string ParameterGroupName;
    string ParameterGroupDescription;
};

#endif
