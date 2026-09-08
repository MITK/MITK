/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCommandLineParser_h
#define mitkCommandLineParser_h

#include <map>
#include <usAny.h>

#include <MitkCommandLineExports.h>
#include <mitkVersion.h>

/** \brief Command line argument parser for MITK applications, based on the CTK command line parser.
 *
 * Use this class to add information about the command line arguments
 * your program understands and to easily parse them from a given list
 * of strings.
 *
 * This parser provides the following features:
 *
 * - Add arguments by supplying a long name and/or a short name.
 *   Arguments are validated using a regular expression. They can have
 *   a default value and a help string.
 * - Deprecated arguments.
 * - Custom regular expressions for argument validation.
 * - Set different argument name prefixes for native platform look and feel.
 * - Create a help text for the command line arguments with support for
 *   grouping arguments.
 *
 * The main difference between the MITK command line parser and the CTK command line
 * parser is that the former does not depend on Qt. Apart from that an image type was
 * added and XML output improved for automatic GUI generation.
 *
 * \c std::cout is used for output to keep dependencies to a minimum.
 */
class MITKCOMMANDLINE_EXPORT mitkCommandLineParser
{
public:
  /** \brief Supported argument value types.
   */
  enum Type
  {
    String = 0,    ///< A single string value.
    Bool = 1,      ///< A boolean flag (no parameters, presence means \c true).
    StringList = 2,///< A list of string values.
    Int = 3,       ///< An integer value.
    Float = 4,     ///< A floating-point value.
    Directory = 5, ///< A directory path.
    File = 6,      ///< A file path.
    Image = 7      ///< An image file path.
  };

  /** \brief I/O channel classification for arguments.
   *
   * Used in XML output generation for automatic GUI creation.
   */
  enum Channel
  {
    None = 0,   ///< Not an I/O channel.
    Input = 1,  ///< Argument represents an input resource.
    Output = 2  ///< Argument represents an output resource.
  };

  /** \brief Container type for string lists, used for argument values and unparsed arguments.
   */
  typedef std::vector<std::string> StringContainerType;

  /** \brief Construct a parser with default settings.
   *
   * By default, no argument prefixes are set and strict mode is disabled.
   */
  mitkCommandLineParser();

  /** \brief Destructor.
   *
   * Cleans up internal argument description storage.
   */
  ~mitkCommandLineParser();

  /** \brief Parse a given list of command line arguments.
   *
   * This method parses a list of string elements considering the known arguments
   * added by calls to addArgument(). If any one of the argument
   * values does not match the corresponding regular expression,
   * \p ok is set to \c false and an empty map object is returned.
   *
   * The keys in the returned map object correspond to the long argument string,
   * if it is not empty. Otherwise, the short argument string is used as key. The
   * us::Any values can safely be converted to the type specified in the
   * addArgument() method call.
   *
   * This overload performs plain parsing. The informational arguments
   * \c --help, \c --xml and \c --version are handled by the argc/argv
   * overload only.
   *
   * \param[in] arguments A StringContainerType containing command line arguments.
   * \param[out] ok If not \c nullptr, set to \c true if parsing succeeded, \c false otherwise.
   * \return A map object mapping argument names to us::Any values.
   */
  std::map<std::string, us::Any> parseArguments(const StringContainerType &arguments, bool *ok = nullptr);

  /** \brief Convenient overload to parse arguments from argc/argv.
   *
   * Converts \p argc and \p argv to a StringContainerType and delegates to
   * parseArguments(const StringContainerType&, bool*).
   *
   * Before parsing, the informational arguments are handled: \c --help (or
   * \c -h) prints helpText(), \c --xml (or \c -xml) prints the XML
   * description and \c --version prints the MITK revision. In all three
   * cases the process is terminated with \c EXIT_SUCCESS, since a
   * command-line app has nothing left to do. The short name \c -h is
   * therefore reserved for help. Otherwise a banner line naming the app and
   * the MITK version is printed and parsing proceeds.
   *
   * \param[in] argc The argument count from main().
   * \param[in] argv The argument array from main().
   * \param[out] ok If not \c nullptr, set to \c true if parsing succeeded, \c false otherwise.
   * \return A map object mapping argument names to us::Any values.
   *
   * \sa parseArguments(const StringContainerType&, bool*)
   */
  std::map<std::string, us::Any> parseArguments(int argc, char **argv, bool *ok = nullptr);

  /** \brief Get a detailed error description if a call to parseArguments() failed.
   *
   * \return The error description, or an empty string if no error occurred.
   *
   * \sa parseArguments()
   */
  std::string errorString() const;

  /** \brief Get all unparsed arguments.
   *
   * Returns all arguments for which no long or short name has been registered
   * via a call to addArgument().
   *
   * \return A reference to the list of unparsed argument strings.
   *
   * \sa addArgument()
   */
  const StringContainerType &unparsedArguments() const;

  /** \brief Check if a given argument has been registered.
   *
   * \param[in] argument The long or short argument name to check.
   * \return \c true if the argument was added via addArgument(), \c false otherwise.
   *
   * \sa addArgument()
   */
  bool argumentAdded(const std::string &argument) const;

  /** \brief Check if a given argument has been parsed successfully.
   *
   * \param[in] argument The long or short argument name to check.
   * \return \c true if the argument was parsed in the last call to parseArguments(),
   *         \c false otherwise.
   *
   * \sa parseArguments()
   */
  bool argumentParsed(const std::string &argument) const;

  /** \brief Add a command line argument.
   *
   * An argument can have a long name (like \c --long-argument-name), a short name
   * (like \c -l), or both. The type of the argument can be specified by using the
   * \p type parameter. The following types are supported:
   *
   * | Type       | # of parameters | Default regex | Example                        |
   * |------------|-----------------|---------------|--------------------------------|
   * | String     | 1               | .*            | --test-string StringParameter   |
   * | Bool       | 0               | n/a           | --enable-something              |
   * | StringList | -1              | .*            | --test-list string1 string2     |
   * | Int        | 1               | -?[0-9]+      | --test-int -5                   |
   *
   * The regular expressions are used to validate the parameters of command line
   * arguments. You can restrict the valid set of parameters by calling
   * setExactMatchRegularExpression() for your argument.
   *
   * Optionally, a help string and a default value can be provided for the argument. If
   * the us::Any type of the default value does not match \p type, an
   * exception is thrown. Arguments with default values are always returned by
   * parseArguments().
   *
   * If the long or short argument has already been added, or if both are empty strings,
   * the method call has no effect.
   *
   * \param[in] longarg The long argument name (without prefix).
   * \param[in] shortarg The short argument name (without prefix).
   * \param[in] type The argument value type.
   * \param[in] argLabel The label of this argument for auto-generated interfaces.
   * \param[in] argHelp A help string describing the argument.
   * \param[in] defaultValue A default value for the argument.
   * \param[in] optional Whether the argument is optional (\c true by default).
   * \param[in] ignoreRest If \c true, all arguments after this one will be ignored.
   * \param[in] deprecated If \c true, marks the argument as deprecated.
   * \param[in] channel The I/O channel classification for XML output.
   *
   * \throw std::logic_error If the type of \p defaultValue does not match \p type.
   *
   * \sa setExactMatchRegularExpression(), addDeprecatedArgument()
   */
  void addArgument(const std::string &longarg,
                   const std::string &shortarg,
                   Type type,
                   const std::string &argLabel,
                   const std::string &argHelp = std::string(),
                   const us::Any &defaultValue = us::Any(),
                   bool optional = true,
                   bool ignoreRest = false,
                   bool deprecated = false,
                   mitkCommandLineParser::Channel channel = mitkCommandLineParser::Channel::None);

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
 * @param argLabel
 * @param argHelp A help string describing alternatives to the deprecated argument.
 */
  void addDeprecatedArgument(const std::string &longarg,
                             const std::string &shortarg,
                             const std::string &argLabel,
                             const std::string &argHelp);


  /**
  * Returns the vector of current Command line Parameter
  *
  */
  std::vector < std::map<std::string, us::Any> > getArgumentList();

  /**
 * Sets a custom regular expression for validating argument parameters. The method
 * <code>errorString()</code> can be used the get the last error description.
 *
 * @param argument The previously added long or short argument name.
 * @param expression A regular expression which the argument parameters must match.
 * @param exactMatchFailedMessage An error message explaining why the parameter did
 *        not match.
 *
 * @return <code>true</code> if the argument was found and the regular expression was set,
 *         <code>false</code> otherwise.
 *
 * @see errorString()
 */
  bool setExactMatchRegularExpression(const std::string &argument,
                                      const std::string &expression,
                                      const std::string &exactMatchFailedMessage);

  /**
 * The field width for the argument names without the help text.
 *
 * @return The argument names field width in the help text.
 */
  std::string::size_type fieldWidth() const;

  /**
 * Creates a help text containing properly formatted argument names and help strings
 * provided by calls to <code>addArgument()</code>. The arguments can be grouped by
 * using <code>beginGroup()</code> and <code>endGroup()</code>.
 *
 * @return The formatted help text.
 */
  std::string helpText() const;

  /**
 * Sets the argument prefix for long and short argument names. This can be used
 * to create native command line arguments without changing the calls to
 * <code>addArgument()</code>. For example on Unix-based systems, long argument
 * names start with "--" and short names with "-", while on Windows argument names
 * always start with "/".
 *
 * Note that all methods in mitkCommandLineParser which take an argument name
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
  void setArgumentPrefix(const std::string &longPrefix, const std::string &shortPrefix);

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
  void beginGroup(const std::string &description);

  /**
 * Ends the current group.
 *
 * @see beginGroup(const std::string&)
 */
  void endGroup();

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

protected:
  class ctkInternal;
  ctkInternal *Internal;

  std::string Title;
  std::string Contributor;
  std::string Category;
  std::string Description;
  std::string ParameterGroupName;
  std::string ParameterGroupDescription;
};



#endif
