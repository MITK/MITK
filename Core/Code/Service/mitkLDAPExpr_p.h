/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKLDAPEXPR_H
#define MITKLDAPEXPR_H

#include <vector>
#include <string>

#include <mitkConfig.h>

#ifdef MITK_HAS_UNORDERED_SET_H
#include <unordered_set>
#else
#include "mitkItkHashSet.h"
#endif

#include "mitkSharedData.h"
#include "mitkServiceProperties.h"

namespace mitk {

class LDAPExprData;

/**
 * This class is not part of the public API.
 */
class LDAPExpr {

public:

  const static int AND;     // =  0;
  const static int OR;      // =  1;
  const static int NOT;     // =  2;
  const static int EQ;      // =  4;
  const static int LE;      // =  8;
  const static int GE;      // = 16;
  const static int APPROX;  // = 32;
  const static int COMPLEX; // = AND | OR | NOT;
  const static int SIMPLE;  // = EQ | LE | GE | APPROX;

  typedef char Byte;
  typedef std::vector<std::string> StringList;
  typedef std::vector<StringList> LocalCache;
#ifdef MITK_HAS_UNORDERED_SET_H
  typedef std::unordered_set<std::string> ObjectClassSet;
#else
  typedef itk::hash_set<std::string> ObjectClassSet;
#endif

  /**
   * Creates an invalid LDAPExpr object. Use with care.
   *
   * @see IsNull()
   */
  LDAPExpr();

  LDAPExpr(const std::string& filter);

  LDAPExpr(const LDAPExpr& other);

  LDAPExpr& operator=(const LDAPExpr& other);

  ~LDAPExpr();

  /**
   * Get object class set matched by this LDAP expression. This will not work
   * with wildcards and NOT expressions. If a set can not be determined return <code>false</code>.
   *
   * \param objClasses The set of matched classes will be added to objClasses.
   * \return If the set cannot be determined, <code>false</code> is returned, <code>true</code> otherwise.
   */
  bool GetMatchedObjectClasses(ObjectClassSet& objClasses) const;

  /**
   * Checks if this LDAP expression is "simple". The definition of
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
   * @param keywords The keywords to look for.
   * @param cache An array (indexed by the keyword indexes) of lists to
   * fill in with values saturating this expression.
   * @return <code>true</code> if this expression is simple,
   * <code>false</code> otherwise.
   */
  bool IsSimple(
    const StringList& keywords,
    LocalCache& cache,
    bool matchCase) const;

  /**
   * Returns <code>true</code> if this instance is invalid, i.e. it was
   * constructed using LDAPExpr().
   *
   * @return <code>true</code> if the expression is invalid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  //!
  static bool Query(const std::string& filter, const ServiceProperties& pd);

  //! Evaluate this LDAP filter.
  bool Evaluate(const ServiceProperties& p, bool matchCase) const;

  //!
  const std::string ToString() const;


private:

  class ParseState;

  //!
  LDAPExpr(int op, const std::vector<LDAPExpr>& args);

  //!
  LDAPExpr(int op, const std::string& attrName, const std::string& attrValue);

  //!
  static LDAPExpr ParseExpr(ParseState& ps);

  //!
  static LDAPExpr ParseSimple(ParseState& ps);

  static std::string Trim(std::string str);

  static std::string ToLower(const std::string& str);

  //!
  bool Compare(const Any& obj, int op, const std::string& s) const;

  //!
  static bool CompareString(const std::string& s1, int op, const std::string& s2);

  //!
  static std::string FixupString(const std::string &s);

  //!
  static bool PatSubstr(const std::string& s, const std::string& pat);

  //!
  static bool PatSubstr(const std::string& s, int si, const std::string& pat, int pi);


  const static Byte WILDCARD; // = 65535;
  const static std::string WILDCARD_STRING;// = std::string( WILDCARD );

  const static std::string NULLQ;     // = "Null query";
  const static std::string GARBAGE;   // = "Trailing garbage";
  const static std::string EOS;       // = "Unexpected end of query";
  const static std::string MALFORMED; // = "Malformed query";
  const static std::string OPERATOR;  // = "Undefined m_operator";

  //! Shared pointer
  SharedDataPointer<LDAPExprData> d;

};

}

#endif // MITKLDAPEXPR_H
