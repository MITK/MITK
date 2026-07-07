/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USLDAPEXPR_H
#define USLDAPEXPR_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>
#include <unordered_map>
#include <unordered_set>

#include <usSharedData.h>

#include <vector>
#include <string>

namespace us {

class Any;
class LDAPExprData;
class ServicePropertiesImpl;

/**
 * \brief Internal representation of an LDAP search filter expression.
 *
 * This class is not part of the public API. It parses and evaluates
 * RFC 1960-based LDAP filter strings used to match service properties.
 *
 * \sa ServicePropertiesImpl
 */
class LDAPExpr {

public:

  const static int AND = 0;
  const static int OR = 1;
  const static int NOT = 2;
  const static int EQ = 4;
  const static int LE = 8;
  const static int GE = 16;
  const static int APPROX = 32;
  const static int COMPLEX = AND | OR | NOT;
  const static int SIMPLE = EQ | LE | GE | APPROX;

  typedef char Byte;
  typedef std::vector<std::string> StringList;
  typedef std::vector<StringList> LocalCache;
  typedef std::unordered_set<std::string> ObjectClassSet;


  /**
   * \brief Creates an invalid LDAPExpr object. Use with care.
   *
   * \sa IsNull()
   */
  LDAPExpr();

  /**
   * \brief Construct an LDAPExpr by parsing the given filter string.
   *
   * \param[in] filter LDAP filter string to parse.
   * \throws InvalidSyntaxException if the filter is malformed.
   */
  LDAPExpr(const std::string& filter);

  /** \brief Copy constructor. */
  LDAPExpr(const LDAPExpr& other);

  /** \brief Copy assignment operator. */
  LDAPExpr& operator=(const LDAPExpr& other);

  /** \brief Destructor. */
  ~LDAPExpr();

  /**
   * \brief Get object class set matched by this LDAP expression.
   *
   * This will not work with wildcards and NOT expressions. If a set
   * can not be determined return \c false.
   *
   * \param[out] objClasses The set of matched classes will be added to objClasses.
   * \return If the set cannot be determined, \c false is returned, \c true otherwise.
   */
  bool GetMatchedObjectClasses(ObjectClassSet& objClasses) const;

  /**
   * \brief Checks if this LDAP expression is "simple". The definition of
   * a simple filter is:
   * <ul>
   *  <li><code>(<it>name</it>=<it>value</it>)</code> is simple if
   *      <it>name</it> is a member of the provided <code>keywords</code>,
   *      and <it>value</it> does not contain a wildcard character;</li>
   *  <li><code>(| EXPR+ )</code> is simple if all <code>EXPR</code>
   *      expressions are simple;</li>
   *  <li>No other expressions are simple.</li>
   * </ul>
   * If the filter is found to be simple, the <code>cache</code> is
   * filled with mappings from the provided keywords to lists
   * of attribute values. The keyword-value-pairs are the ones that
   * satisfy this expression, for the given keywords.
   *
   * \param[in] keywords The keywords to look for.
   * \param[out] cache An array (indexed by the keyword indexes) of lists to
   * fill in with values saturating this expression.
   * \return \c true if this expression is simple,
   * \c false otherwise.
   */
  bool IsSimple(
    const StringList& keywords,
    LocalCache& cache,
    bool matchCase) const;

  /**
   * \brief Returns \c true if this instance is invalid, i.e. it was
   * constructed using LDAPExpr().
   *
   * \return \c true if the expression is invalid,
   *         \c false otherwise.
   */
  bool IsNull() const;

  /**
   * \brief Parse and evaluate a filter string against service properties.
   *
   * \param[in] filter LDAP filter string.
   * \param[in] pd Service properties to match against.
   * \return \c true if the properties match the filter.
   */
  static bool Query(const std::string& filter, const ServicePropertiesImpl& pd);

  /**
   * \brief Evaluate this LDAP filter against the given service properties.
   *
   * \param[in] p Service properties to match against.
   * \param[in] matchCase Whether the comparison is case-sensitive.
   * \return \c true if the properties satisfy the expression.
   */
  bool Evaluate(const ServicePropertiesImpl& p, bool matchCase) const;

  /**
   * \brief Return the string representation of this LDAP expression.
   *
   * \return The filter string.
   */
  const std::string ToString() const;


private:

  /** \brief Contains the current parser position and parsing utility methods. */
  class ParseState
  {

  private:

    std::size_t m_pos; ///< \brief Current parse position.
    std::string m_str; ///< \brief The string being parsed.

  public:

    /**
     * \brief Construct a ParseState from the given string.
     *
     * \param[in] str The LDAP filter string to parse.
     */
    ParseState(const std::string& str);

    /**
     * \brief Move m_pos to remove the prefix \a pre.
     *
     * \param[in] pre Prefix to match and skip.
     * \return \c true if the prefix was found and skipped.
     */
    bool prefix(const std::string& pre);

    /**
     * \brief Peek a char at m_pos.
     *
     * \note If index is out of bounds, throws an exception.
     * \return The character at the current position.
     */
    LDAPExpr::Byte peek();

    /**
     * \brief Increment m_pos by \a n.
     *
     * \param[in] n Number of characters to skip.
     */
    void skip(int n);

    /**
     * \brief Return the substring from m_pos until the end.
     *
     * \return The remaining unparsed string.
     */
    std::string rest() const;

    /** \brief Advance m_pos past any whitespace characters. */
    void skipWhite();

    /**
     * \brief Get the attribute name up to the next special character and advance m_pos.
     *
     * \return The attribute name.
     */
    std::string getAttributeName();

    /**
     * \brief Get the attribute value, converting '*' to WILDCARD, and advance m_pos.
     *
     * \return The attribute value string.
     */
    std::string getAttributeValue();

    /**
     * \brief Throw an InvalidSyntaxException with the given message.
     *
     * \param[in] m Error message.
     */
    void error(const std::string& m) const;

  };

  /** \brief Construct a complex (AND/OR/NOT) expression from sub-expressions. */
  LDAPExpr(int op, const std::vector<LDAPExpr>& args);

  /** \brief Construct a simple (EQ/LE/GE/APPROX) expression. */
  LDAPExpr(int op, const std::string& attrName, const std::string& attrValue);

  /** \brief Parse a full LDAP expression from the given parse state. */
  static LDAPExpr ParseExpr(ParseState& ps);

  /** \brief Parse a simple LDAP expression from the given parse state. */
  static LDAPExpr ParseSimple(ParseState& ps);

  /** \brief Trim leading and trailing whitespace from \a str. */
  static std::string Trim(std::string str);

  /** \brief Convert \a str to lowercase. */
  static std::string ToLower(const std::string& str);

  /** \brief Compare a property value against a string using the given operator. */
  bool Compare(const Any& obj, int op, const std::string& s) const;

  /** \brief Compare an integral-type property value using the given operator. */
  template<typename T>
  bool CompareIntegralType(const Any& obj, const int op, const std::string& s) const;

  /** \brief Compare two strings using the given LDAP operator. */
  static bool CompareString(const std::string& s1, int op, const std::string& s2);

  /** \brief Prepare a wildcard pattern string for matching. */
  static std::string FixupString(const std::string &s);

  /** \brief Check if the string \a s matches the wildcard pattern \a pat. */
  static bool PatSubstr(const std::string& s, const std::string& pat);

  /** \brief Recursive helper for wildcard pattern matching. */
  static bool PatSubstr(const std::string& s, int si, const std::string& pat, int pi);

  /** \brief Shared pointer to the expression data. */
  SharedDataPointer<LDAPExprData> d;

};

}

#endif // USLDAPEXPR_H
