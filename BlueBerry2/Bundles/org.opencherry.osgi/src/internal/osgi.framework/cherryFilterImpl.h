/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYFILTERIMPL_H_
#define CHERRYFILTERIMPL_H_

#include <osgi/framework/Filter.h>

#include <limits>

namespace cherry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;

/**
 * RFC 1960-based Filter. Filter objects can be created by calling
 * the constructor with the desired filter string.
 * A Filter object can be called numerous times to determine if the
 * match argument matches the filter string that was used to create the Filter
 * object.
 *
 * <p>The syntax of a filter string is the string representation
 * of LDAP search filters as defined in RFC 1960:
 * <i>A String Representation of LDAP Search Filters</i> (available at
 * http://www.ietf.org/rfc/rfc1960.txt).
 * It should be noted that RFC 2254:
 * <i>A String Representation of LDAP Search Filters</i>
 * (available at http://www.ietf.org/rfc/rfc2254.txt) supersedes
 * RFC 1960 but only adds extensible matching and is not applicable for this
 * API.
 *
 * <p>The string representation of an LDAP search filter is defined by the
 * following grammar. It uses a prefix format.
 * <pre>
 *   &lt;filter&gt; ::= '(' &lt;filtercomp&gt; ')'
 *   &lt;filtercomp&gt; ::= &lt;and&gt; | &lt;or&gt; | &lt;not&gt; | &lt;item&gt;
 *   &lt;and&gt; ::= '&' &lt;filterlist&gt;
 *   &lt;or&gt; ::= '|' &lt;filterlist&gt;
 *   &lt;not&gt; ::= '!' &lt;filter&gt;
 *   &lt;filterlist&gt; ::= &lt;filter&gt; | &lt;filter&gt; &lt;filterlist&gt;
 *   &lt;item&gt; ::= &lt;simple&gt; | &lt;present&gt; | &lt;substring&gt;
 *   &lt;simple&gt; ::= &lt;attr&gt; &lt;filtertype&gt; &lt;value&gt;
 *   &lt;filtertype&gt; ::= &lt;equal&gt; | &lt;approx&gt; | &lt;greater&gt; | &lt;less&gt;
 *   &lt;equal&gt; ::= '='
 *   &lt;approx&gt; ::= '~='
 *   &lt;greater&gt; ::= '&gt;='
 *   &lt;less&gt; ::= '&lt;='
 *   &lt;present&gt; ::= &lt;attr&gt; '=*'
 *   &lt;substring&gt; ::= &lt;attr&gt; '=' &lt;initial&gt; &lt;any&gt; &lt;final&gt;
 *   &lt;initial&gt; ::= NULL | &lt;value&gt;
 *   &lt;any&gt; ::= '*' &lt;starval&gt;
 *   &lt;starval&gt; ::= NULL | &lt;value&gt; '*' &lt;starval&gt;
 *   &lt;final&gt; ::= NULL | &lt;value&gt;
 * </pre>
 *
 * <code>&lt;attr&gt;</code> is a string representing an attribute, or
 * key, in the properties objects of the registered services.
 * Attribute names are not case sensitive;
 * that is cn and CN both refer to the same attribute.
 * <code>&lt;value&gt;</code> is a string representing the value, or part of
 * one, of a key in the properties objects of the registered services.
 * If a <code>&lt;value&gt;</code> must
 * contain one of the characters '<code>*</code>' or '<code>(</code>'
 * or '<code>)</code>', these characters
 * should be escaped by preceding them with the backslash '<code>\</code>'
 * character.
 * Note that although both the <code>&lt;substring&gt;</code> and
 * <code>&lt;present&gt;</code> productions can
 * produce the <code>'attr=*'</code> construct, this construct is used only to
 * denote a presence filter.
 *
 * <p>Examples of LDAP filters are:
 *
 * <pre>
 *   &quot;(cn=Babs Jensen)&quot;
 *   &quot;(!(cn=Tim Howes))&quot;
 *   &quot;(&(&quot; + Constants.OBJECTCLASS + &quot;=Person)(|(sn=Jensen)(cn=Babs J*)))&quot;
 *   &quot;(o=univ*of*mich*)&quot;
 * </pre>
 *
 * <p>The approximate match (<code>~=</code>) is implementation specific but
 * should at least ignore case and white space differences. Optional are
 * codes like soundex or other smart "closeness" comparisons.
 *
 * <p>Comparison of values is not straightforward. Strings
 * are compared differently than numbers and it is
 * possible for a key to have multiple values. Note that
 * that keys in the match argument must always be strings.
 * The comparison is defined by the object type of the key's
 * value. The following rules apply for comparison:
 *
 * <blockquote>
 * <TABLE BORDER=0>
 * <TR><TD><b>Property Value Type </b></TD><TD><b>Comparison Type</b></TD></TR>
 * <TR><TD>String </TD><TD>String comparison</TD></TR>
 * <TR valign=top><TD>Integer, Long, Float, Double, Byte, Short, BigInteger, BigDecimal </TD><TD>numerical comparison</TD></TR>
 * <TR><TD>Character </TD><TD>character comparison</TD></TR>
 * <TR><TD>Boolean </TD><TD>equality comparisons only</TD></TR>
 * <TR><TD>[] (array)</TD><TD>recursively applied to values </TD></TR>
 * <TR><TD>Vector</TD><TD>recursively applied to elements </TD></TR>
 * </TABLE>
 * Note: arrays of primitives are also supported.
 * </blockquote>
 *
 * A filter matches a key that has multiple values if it
 * matches at least one of those values. For example,
 * <pre>
 *   Dictionary d = new Hashtable();
 *   d.put( "cn", new String[] { "a", "b", "c" } );
 * </pre>
 *   d will match <code>(cn=a)</code> and also <code>(cn=b)</code>
 *
 * <p>A filter component that references a key having an unrecognizable
 * data type will evaluate to <code>false</code> .
 */

class FilterImpl : public ::osgi::framework::Filter {

public:

  osgiObjectMacro(cherry::osgi::internal::FilterImpl)


  /* public methods in osgi::framework::Filter */

  /**
   * Constructs a {@link FilterImpl} object. This filter object may be used
   * to match a {@link ServiceReferenceImpl} or a Dictionary.
   *
   * <p> If the filter cannot be parsed, an {@link InvalidSyntaxException}
   * will be thrown with a human readable message where the
   * filter became unparsable.
   *
   * @param filterString the filter string.
   * @exception InvalidSyntaxException If the filter parameter contains
   * an invalid filter string that cannot be parsed.
   */
  static FilterImpl::Pointer NewInstance(const std::string& filterString) throw(InvalidSyntaxException);

  /**
   * Filter using a service's properties.
   * <p>
   * The filter is executed using the keys and values of the referenced
   * service's properties. The keys are case insensitively matched with
   * the filter.
   *
   * @param reference The reference to the service whose properties are
   *        used in the match.
   * @return <code>true</code> if the service's properties match this
   *         filter; <code>false</code> otherwise.
   */
  bool Match(SmartPointer<ServiceReference> reference);

  /**
   * Filter using a <code>Dictionary</code> object. The Filter is executed
   * using the <code>Dictionary</code> object's keys and values. The keys
   * are case insensitively matched with the filter.
   *
   * @param dictionary The <code>Dictionary</code> object whose keys are
   *        used in the match.
   * @return <code>true</code> if the <code>Dictionary</code> object's
   *         keys and values match this filter; <code>false</code>
   *         otherwise.
   * @throws IllegalArgumentException If <code>dictionary</code> contains
   *         case variants of the same key name.
   */
  bool Match(Dictionary::ConstPointer dictionary) throw(IllegalArgumentException);

  /**
   * Filter with case sensitivity using a <code>Dictionary</code> object.
   * The Filter is executed using the <code>Dictionary</code> object's
   * keys and values. The keys are case sensitively matched with the
   * filter.
   *
   * @param dictionary The <code>Dictionary</code> object whose keys are
   *        used in the match.
   * @return <code>true</code> if the <code>Dictionary</code> object's
   *         keys and values match this filter; <code>false</code>
   *         otherwise.
   * @since 1.3
   */
  bool MatchCase(Dictionary::ConstPointer dictionary);

  /**
   * Returns this <code>Filter</code> object's filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return Filter string.
   */

  std::string ToString() const;

  /**
   * Compares this <code>Filter</code> object to another object.
   *
   * @param obj The object to compare against this <code>Filter</code>
   *        object.
   * @return If the other object is a <code>Filter</code> object, then
   *         returns <code>this.toString().equals(obj.toString()</code>;
   *         <code>false</code> otherwise.
   */
  bool operator==(const Object* obj) const;

  /**
     * Returns the hashCode for this <code>Filter</code> object.
   *
     * @return The hashCode of the filter string; that is,
   * <code>this.toString().hashCode()</code>.
   */
  std::size_t HashCode() const;

  /* non public fields and methods for the Filter implementation */

  private:

  /**
   * Returns this <code>Filter</code>'s normalized filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return This <code>Filter</code>'s filter string.
   */
  std::string Normalize() const;

  /** filter operation */
  enum OperationType {
  EQUAL,
  APPROX,
  GREATER,
  LESS,
  PRESENT,
  SUBSTRING,
  AND,
  OR,
  NOT
  };

  const OperationType op;

  /** filter attribute or null if operation AND, OR or NOT */
  const std::string attr;
  /** filter operands */
  const Object::Pointer value;

  /* normalized filter string for topLevel Filter object */
  mutable std::string filterString;

  FilterImpl(OperationType operation, const std::string& attr, Object::Pointer value);

  /**
   * Internal match routine.
   * Dictionary parameter must support case-insensitive get.
   *
   * @param properties A dictionary whose
   * keys are used in the match.
   * @return If the Dictionary's keys match the filter,
   * return <code>true</code>. Otherwise, return <code>false</code>.
   */
  bool Match0(Dictionary::ConstPointer properties) const;

  /**
   * Encode the value string such that '(', '*', ')'
   * and '\' are escaped.
   *
   * @param value unencoded value string.
   * @return encoded value string.
   */
  static std::string EncodeValue(const std::string& value);

  bool Compare(int operation, Object::Pointer value1, Object::Pointer value2) const;

  bool Compare_Vector(int operation, const std::vector<Object::Pointer>& collection, Object::Pointer value2) const;

  bool Compare_String(int operation, const std::string& string, Object::Pointer value2) const;

  bool Compare_Integer(int operation, int intval, Object::Pointer value2) const;

  bool Compare_Long(int operation, long longval, Object::Pointer value2) const;

  bool Compare_Short(int operation, short shortval, Object::Pointer value2) const;

  bool Compare_Character(int operation, char charval, Object::Pointer value2) const;

  bool Compare_Bool(int operation, bool boolval, Object::Pointer value2) const;

  bool Compare_Float(int operation, float floatval, Object::Pointer value2) const;

  bool Compare_Double(int operation, double doubleval, Object::Pointer value2) const;

  /**
   * Map a string for an APPROX (~=) comparison.
   *
   * This implementation removes white spaces.
   * This is the minimum implementation allowed by
   * the OSGi spec.
   *
   * @param input Input string.
   * @return String ready for APPROX comparison.
   */
  static std::string ApproxString(const std::string& input);


public:

  /**
   * Returns the leftmost required objectClass value for the filter to evaluate to true.
   *
   * @return The leftmost required objectClass value or null if none could be determined.
   */
  std::string GetRequiredObjectClass() const;

  /**
   * Returns all the attributes contained within this filter
   * @return all the attributes contained within this filter
   */
  std::vector<std::string> GetAttributes() const;


private:

  template<typename T>
  inline int NumericCompare(const T& x, const T& y) const
  {
    if (((x-y) < 0 ? (x-y)*(T)(-1) : (x-y)) <= std::numeric_limits<T>::epsilon())
      return 0;
    else if (x < y) return -1;
    else return 1;
  }

  struct EqualWhitespace : public std::unary_function<std::string::value_type, bool> {
    bool operator()(const std::string::value_type& x) const {
      if (x == ' ' || x == '\t' || x == '\f' || x == '\v' || x == '\n' || x == '\r')
        return true;
      return false;
    }
  };

  void GetAttributesInternal(std::vector<std::string>& results) const;

  /**
   * Parser class for OSGi filter strings. This class parses
   * the complete filter string and builds a tree of Filter
   * objects rooted at the parent.
   */
  class Parser {

  private:
    const std::string filterstring;
    //const char filterChars[];
    std::size_t pos;

    static const std::string FILTER_MISSING_LEFTPAREN;
    static const std::string FILTER_MISSING_RIGHTPAREN;

  public:

    Parser(const std::string& filterstring);

    FilterImpl::Pointer Parse() throw(InvalidSyntaxException);

  private:

    FilterImpl::Pointer Parse_Filter() throw(InvalidSyntaxException);

    FilterImpl::Pointer Parse_FilterComp() throw(InvalidSyntaxException);

    FilterImpl::Pointer Parse_And() throw(InvalidSyntaxException);

    FilterImpl::Pointer Parse_Or() throw(InvalidSyntaxException);

    FilterImpl::Pointer Parse_Not() throw(InvalidSyntaxException);

    FilterImpl::Pointer Parse_Item() throw(InvalidSyntaxException);

    std::string Parse_Attr() throw(InvalidSyntaxException);

    Object::Pointer Parse_Value() throw(InvalidSyntaxException);

    Object::Pointer Parse_Substring() throw(InvalidSyntaxException);

    void SkipWhiteSpace();
  };

  /**
   * This Dictionary is used for key lookup from a ServiceReference during
   * filter evaluation. This Dictionary implementation only supports the get
   * operation using a String key as no other operations are used by the
   * Filter implementation.
   *
   */
  class ServiceReferenceDictionary : public Dictionary {

  private:
    const SmartPointer<ServiceReference> reference;


  public:

    ServiceReferenceDictionary(SmartPointer<ServiceReference> reference);

    const Value Get(const Key& key) const;

    const Value& operator[](const Key& key) const;

    void Clear();

   bool ContainsKey(const Key& key);

   bool ContainsValue(const Value& value);

   std::set<Entry> EntrySet() const;

   std::set<Key> KeySet() const;

   std::vector<ConstValue> Values() const;

   std::vector<Value> Values();

   Value Get(const Key& key);

   Value Put(const Key& key, const Value& value);

   void PutAll(const Dictionary& dict);

   Value& operator[](const Key& key);

   Value Remove(const Key& key);

   std::size_t Size() const ;

    bool IsEmpty() const;

  };


};

}
}
}


#endif /* CHERRYFILTERIMPL_H_ */
