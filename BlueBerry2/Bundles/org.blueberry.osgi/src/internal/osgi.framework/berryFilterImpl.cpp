/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryFilterImpl.h"

#include "../osgi.serviceregistry/berryServiceReferenceImpl.h"
#include "../osgi.serviceregistry/berryServiceRegistrationImpl.h"

#include <osgi/framework/Objects.h>
#include <osgi/framework/ObjectString.h>
#include <osgi/framework/ObjectVector.h>
#include <osgi/framework/Constants.h>

#include <osgi/util/Headers.h>

#include <supplement/framework.debug/berryDebug.h>

#include <Poco/NumberParser.h>

namespace berry {
namespace osgi {
namespace internal {

using namespace ::osgi::util;

FilterImpl::Pointer FilterImpl::NewInstance(const std::string& filterString) throw(InvalidSyntaxException) {
    Parser parser(filterString);
    return parser.Parse();
  }

  bool FilterImpl::Match(SmartPointer<ServiceReference> reference) {
    if (ServiceReferenceImpl::Pointer ref = reference.Cast<ServiceReferenceImpl>()) {
      return Match0(ref->GetRegistration()->GetProperties());
    }
    Dictionary::Pointer dict(new ServiceReferenceDictionary(reference));
    return Match0(dict);
  }

  bool FilterImpl::Match(Dictionary::ConstPointer dictionary) throw(IllegalArgumentException) {
    //if (dictionary != null) {
    //  dictionary = new Headers(dictionary);
    //}

    Dictionary::Pointer dict(new Headers(*dictionary));
    return Match0(dict);
  }

  bool FilterImpl::MatchCase(Dictionary::ConstPointer dictionary) {
    return Match0(dictionary);
  }

  std::string FilterImpl::ToString() const {
//    std::string result(filterString);
//    if (result.empty()) {
//      result = Normalize();
//      filterString = result;
//    }
//    return result;

    if (filterString.empty()) {
      filterString = Normalize();
    }
    return filterString;
  }

  bool FilterImpl::operator==(const Object* obj) const {
    if (obj == this) {
      return true;
    }

    if (!dynamic_cast<const Filter*>(obj)) {
      return false;
    }

    return this->ToString() == obj->ToString();
  }

  std::size_t FilterImpl::HashCode() const {
    return Poco::hash(this->ToString());
  }

  std::string FilterImpl::Normalize() const {
    std::string sb = "(";

    switch (op) {
      case AND : {
        sb += '&';

        ObjectVector<FilterImpl::Pointer>::Pointer filters = value.Cast<ObjectVector<FilterImpl::Pointer> >();
        for (unsigned int i = 0, size = filters->size(); i < size; i++) {
          sb += (*filters)[i]->Normalize();
        }

        break;
      }

      case OR : {
        sb += '|';

        ObjectVector<FilterImpl::Pointer>::Pointer filters = value.Cast<ObjectVector<FilterImpl::Pointer> >();
        for (unsigned int i = 0, size = filters->size(); i < size; i++) {
          sb += (*filters)[i]->Normalize();
        }

        break;
      }

      case NOT : {
        sb += '!';
        FilterImpl::Pointer filter = value.Cast<FilterImpl>();
        sb += filter->Normalize();

        break;
      }

      case SUBSTRING : {
        sb += attr + '=';

        ObjectVector<std::string>::Pointer substrings = value.Cast<ObjectVector<std::string> >();

        for (unsigned int i = 0, size = substrings->size(); i < size; i++) {
          const std::string& substr = (*substrings)[i];

          if (substr.empty()) /* * */{
            sb += '*';
          } else /* xxx */{
            sb += EncodeValue(substr);
          }
        }

        break;
      }
      case EQUAL : {
        sb += attr + '=';
        ObjectString::Pointer str = value.Cast<ObjectString>();
        sb += EncodeValue(*str);

        break;
      }
      case GREATER : {
        sb += attr + ">=";
        ObjectString::Pointer str = value.Cast<ObjectString>();
        sb += EncodeValue(*str);

        break;
      }
      case LESS : {
        sb += attr + "<=";
        ObjectString::Pointer str = value.Cast<ObjectString>();
        sb += EncodeValue(*str);

        break;
      }
      case APPROX : {
        sb += attr + "~=";
        ObjectString::Pointer str = value.Cast<ObjectString>();
        sb += EncodeValue(ApproxString(*str));

        break;
      }

      case PRESENT : {
        sb += attr + "=*";

        break;
      }
    }

    sb += ')';

    return sb;
  }


//  const int FilterImpl::EQUAL = 1;
//  const int FilterImpl::APPROX = 2;
//  const int FilterImpl::GREATER = 3;
//   const int FilterImpl::LESS = 4;
//   const int FilterImpl::PRESENT = 5;
//   const int FilterImpl::SUBSTRING = 6;
//   const int FilterImpl::AND = 7;
//   const int FilterImpl::OR = 8;
//   const int FilterImpl::NOT = 9;



  FilterImpl::FilterImpl(OperationType operation, const std::string& attr, Object::Pointer value)
  : op(operation), attr(attr), value(value)
  {

  }

  bool FilterImpl::Match0(Dictionary::ConstPointer properties) const {
    switch (op) {
      case AND : {
        ObjectVector<FilterImpl::Pointer>::Pointer filters = value.Cast<ObjectVector<FilterImpl::Pointer> >();
        for (unsigned int i = 0, size = filters->size(); i < size; i++) {
          if (!(*filters)[i]->Match0(properties)) {
            return false;
          }
        }

        return true;
      }

      case OR : {
        ObjectVector<FilterImpl::Pointer>::Pointer filters = value.Cast<ObjectVector<FilterImpl::Pointer> >();
        for (unsigned int i = 0, size = filters->size(); i < size; i++) {
          if ((*filters)[i]->Match0(properties)) {
            return true;
          }
        }

        return false;
      }

      case NOT : {
        FilterImpl::Pointer filter = value.Cast<FilterImpl>();

        return !filter->Match0(properties);
      }

      case SUBSTRING :
      case EQUAL :
      case GREATER :
      case LESS :
      case APPROX : {
        //Object prop = (properties == null) ? null : properties.get(attr);
        Object::Pointer prop = properties->Get(attr);

        return Compare(op, prop, value);
      }

      case PRESENT : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "PRESENT(" << attr << ")";
        }

        //Object prop = (properties == null) ? null : properties.get(attr);
        Object::Pointer prop = properties->Get(attr);

        return prop;
      }
    }

    return false;
  }

  std::string FilterImpl::EncodeValue(const std::string& value) {
    bool encoded = false;
    int inlen = value.size();
    int outlen = inlen << 1; /* inlen * 2 */

    char* output = new char[outlen];
    //value.getChars(0, inlen, output, inlen);

    unsigned int cursor = 0;
    for (int i = 0; i < inlen; i++) {
      char c = value[i];

      switch (c) {
        case '(' :
        case '*' :
        case ')' :
        case '\\' : {
          output[cursor] = '\\';
          cursor++;
          encoded = true;

          break;
        }
      }

      output[cursor] = c;
      cursor++;
    }

    return encoded ? std::string(output, cursor) : value;
  }

  bool FilterImpl::Compare(int operation, Object::Pointer value1, Object::Pointer value2) const {
    if (!value1) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "Compare(" << value1 << "," << value2 << ")";
      }

      return false;
    }

    if (ObjectString::Pointer s = value1.Cast<ObjectString>()) {
      return Compare_String(operation, *s, value2);
    }

    if (ObjectVector<Object::Pointer>::Pointer v = value1.Cast<ObjectVector<Object::Pointer> >()) {
      return Compare_Vector(operation, *v, value2);
    }

    if (ObjectInt::Pointer v = value1.Cast<ObjectInt>()) {
      return Compare_Integer(operation, v->GetValue(), value2);
    }

    if (ObjectChar::Pointer v = value1.Cast<ObjectChar>()) {
      return Compare_Character(operation, v->GetValue(), value2);
    }

    if (ObjectFloat::Pointer v = value1.Cast<ObjectFloat>()) {
      return Compare_Float(operation, v->GetValue(), value2);
    }

    if (ObjectDouble::Pointer v = value1.Cast<ObjectDouble>()) {
      return Compare_Double(operation, v->GetValue(), value2);
    }

    if (ObjectBool::Pointer v = value1.Cast<ObjectBool>()) {
      return Compare_Bool(operation, v->GetValue(), value2);
    }

    //return compare_Unknown(operation, value1, value2); // RFC 59
    return false;
  }

  bool FilterImpl::Compare_Vector(int operation, const std::vector<Object::Pointer>& collection, Object::Pointer value2) const {
    for (std::vector<Object::Pointer>::const_iterator iterator = collection.begin(); iterator != collection.end(); ++iterator) {
      if (Compare(operation, *iterator, value2)) {
        return true;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_String(int operation, const std::string& string, Object::Pointer value2) const {
    switch (operation) {
      case SUBSTRING : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "SUBSTRING(" << string << "," << value2 << ")";
        }

        ObjectVector<std::string>::Pointer substrings = value2.Cast<ObjectVector<std::string> >();
        std::string::size_type pos = 0;
        for (std::string::size_type i = 0, size = substrings->size(); i < size; i++) {
          const std::string& substr = (*substrings)[i];

          if (i + 1 < size) /* if this is not that last substr */{
            if (substr.empty()) /* * */{
              const std::string& substr2 = (*substrings)[i + 1];

              if (substr2.empty()) /* ** */
                continue; /* ignore first star */
              /* *xxx */
              if (Debug::DEBUG && Debug::DEBUG_FILTER) {
                std::cout << "find(\"" << substr2 << "\"," << pos << ")";
              }
              std::string::size_type index = string.find(substr2, pos);
              if (index == std::string::npos) {
                return false;
              }

              pos = index + substr2.size();
              if (i + 2 < size) // if there are more substrings, increment over the string we just matched; otherwise need to do the last substr check
                i++;
            } else /* xxx */{
              std::string::size_type len = substr.size();

              if (Debug::DEBUG && Debug::DEBUG_FILTER) {
                std::cout << "regionMatches(" << pos << ",\"" << substr << "\")";
              }
              if (string.substr(pos, len) == substr.substr(0, len)) {
                pos += len;
              } else {
                return false;
              }
            }
          } else /* last substr */{
            if (substr.empty()) /* * */{
              return true;
            }
            /* xxx */
            if (Debug::DEBUG && Debug::DEBUG_FILTER) {
              std::cout << "regionMatches(" << pos << "," << substr << ")";
            }
            std::string::size_type end = string.rfind(substr);
            return (end == string.size() - substr.size());
          }
        }

        return true;
      }
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << string << "," << value2 << ")";
        }
        const std::string& str2 = *(value2.Cast<ObjectString>());
        return string == str2;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << string << "," << value2 << ")";
        }

        std::string astring = ApproxString(string);
        std::string string2 = ApproxString(*(value2.Cast<ObjectString>()));

        return Poco::icompare(astring, string2);
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << string << "," << value2 << ")";
        }
        const std::string& string2 = *(value2.Cast<ObjectString>());
        return string.compare(string2) >= 0;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << string << "," << value2 << ")";
        }
        return string.compare(*(value2.Cast<ObjectString>())) <= 0;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_Integer(int operation, int intval, Object::Pointer value2) const {
    if (operation == SUBSTRING) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "SUBSTRING(" << intval << "," << value2 << ")";
      }
      return false;
    }

    int intval2 = Poco::NumberParser::parse(*(value2.Cast<ObjectString>()));
    switch (operation) {
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << intval << "," << value2 << ")";
        }
        return intval == intval2;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << intval << "," << value2 << ")";
        }
        return intval == intval2;
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << intval << "," << value2 << ")";
        }
        return intval >= intval2;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << intval << "," << value2 << ")";
        }
        return intval <= intval2;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_Character(int operation, char charval, Object::Pointer value2) const {
    if (operation == SUBSTRING) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "SUBSTRING(" << charval << "," << value2 << ")";
      }
      return false;
    }

    char charval2 = Poco::trim(static_cast<std::string&>(*(value2.Cast<ObjectString>())))[0];
    switch (operation) {
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << charval << "," << value2 << ")";
        }
        return charval == charval2;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << charval << "," << value2 << ")";
        }
        return (charval == charval2) || std::toupper(charval) == std::toupper(charval2) || std::tolower(charval) == std::tolower(charval2);
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << charval << "," << value2 << ")";
        }
        return charval >= charval2;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << charval << "," << value2 << ")";
        }
        return charval <= charval2;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_Bool(int operation, bool boolval, Object::Pointer value2) const {
    if (operation == SUBSTRING) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "SUBSTRING(" << boolval << "," << value2 << ")";
      }
      return false;
    }

    bool boolval2 = Poco::icompare(Poco::trim(static_cast<std::string&>(*(value2.Cast<ObjectString>()))), "true") == 0;
    switch (operation) {
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << boolval << "," << value2 << ")";
        }
        return boolval == boolval2;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << boolval << "," << value2 << ")";
        }
        return boolval == boolval2;
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << boolval << "," << value2 << ")";
        }
        return boolval == boolval2;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << boolval << "," << value2 << ")";
        }
        return boolval == boolval2;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_Float(int operation, float floatval, Object::Pointer value2) const {
    if (operation == SUBSTRING) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "SUBSTRING(" << floatval << "," << value2 << ")";
      }
      return false;
    }

    float floatval2 = (float)Poco::NumberParser::parseFloat(*(value2.Cast<ObjectString>()));
    switch (operation) {
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << floatval << "," << value2 << ")";
        }
        return NumericCompare(floatval, floatval2) == 0;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << floatval << "," << value2 << ")";
        }
        return NumericCompare(floatval, floatval2) == 0;
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << floatval << "," << value2 << ")";
        }
        return NumericCompare(floatval, floatval2) >= 0;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << floatval << "," << value2 << ")";
        }
        return NumericCompare(floatval, floatval2) <= 0;
      }
    }

    return false;
  }

  bool FilterImpl::Compare_Double(int operation, double doubleval, Object::Pointer value2) const {
    if (operation == SUBSTRING) {
      if (Debug::DEBUG && Debug::DEBUG_FILTER) {
        std::cout << "SUBSTRING(" << doubleval << "," << value2 << ")";
      }
      return false;
    }

    double doubleval2 = Poco::NumberParser::parseFloat(*(value2.Cast<ObjectString>()));
    switch (operation) {
      case EQUAL : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "EQUAL(" << doubleval << "," << value2 << ")";
        }
        return NumericCompare(doubleval, doubleval2) == 0;
      }
      case APPROX : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "APPROX(" << doubleval << "," << value2 << ")";
        }
        return NumericCompare(doubleval, doubleval2) == 0;
      }
      case GREATER : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "GREATER(" << doubleval << "," << value2 << ")";
        }
        return NumericCompare(doubleval, doubleval2) >= 0;
      }
      case LESS : {
        if (Debug::DEBUG && Debug::DEBUG_FILTER) {
          std::cout << "LESS(" << doubleval << "," << value2 << ")";
        }
        return NumericCompare(doubleval, doubleval2) <= 0;
      }
    }

    return false;
  }

   std::string FilterImpl::ApproxString(const std::string& input) {
     std::string output(input);
     output.erase(std::remove_if(output.begin(), output.end(), EqualWhitespace()));

     return output;
  }


  std::string FilterImpl::GetRequiredObjectClass() const {
    // just checking for simple filters here where objectClass is the only attr or it is one attr of a base '&' clause
    // (objectClass=org.acme.BrickService) OK
    // (&(objectClass=org.acme.BrickService)(|(vendor=IBM)(vendor=SUN))) OK
    // (objectClass=org.acme.*) NOT OK
    // (|(objectClass=org.acme.BrickService)(objectClass=org.acme.CementService)) NOT OK
    // (&(objectClass=org.acme.BrickService)(objectClass=org.acme.CementService)) OK but only the first objectClass is returned
    switch (op) {
      case EQUAL :
        if (Poco::icompare(attr, Constants::OBJECTCLASS) == 0 && (value.Cast<ObjectString>()))
          return *(value.Cast<ObjectString>());
        break;
      case AND :
        ObjectVector<FilterImpl::Pointer>::Pointer clauses = value.Cast<ObjectVector<FilterImpl::Pointer> >();
        for (std::size_t i = 0; i < clauses->size(); i++)
          if ((*clauses)[i]->op == EQUAL) {
            std::string result = (*clauses)[i]->GetRequiredObjectClass();
            if (!result.empty())
              return result;
          }
        break;
    }
    return "";
  }

  /**
   * Returns all the attributes contained within this filter
   * @return all the attributes contained within this filter
   */
  std::vector<std::string> FilterImpl::GetAttributes() const {
    std::vector<std::string> results;
    GetAttributesInternal(results);
    return results;
  }

  void FilterImpl::GetAttributesInternal(std::vector<std::string>& results) const {
    if (ObjectVector<FilterImpl::Pointer>::Pointer children = value.Cast<ObjectVector<FilterImpl::Pointer> >()) {
      for (std::size_t i = 0; i < children->size(); i++)
        (*children)[i]->GetAttributesInternal(results);
      return;
    } else if (FilterImpl::Pointer filter = value.Cast<FilterImpl>()) {
      // The NOT operation only has one child filter (bug 188075)
      filter->GetAttributesInternal(results);
      return;
    }
    if (!attr.empty())
      results.push_back(attr);
  }

  const std::string FilterImpl::Parser::FILTER_MISSING_LEFTPAREN = "Missing \"(\" at \"";
  const std::string FilterImpl::Parser::FILTER_MISSING_RIGHTPAREN = "Missing \")\" at \"";

    FilterImpl::Parser::Parser(const std::string& filterstring) :
    filterstring(filterstring), pos(0)
    {

    }

    FilterImpl::Pointer FilterImpl::Parser::Parse() throw(InvalidSyntaxException) {
      FilterImpl::Pointer filter;
      try {
        filter = Parse_Filter();
      } catch (std::out_of_range e) {
        throw InvalidSyntaxException("Filter ended abruptly", filterstring);
      }

      if (pos != filterstring.size()) {
        throw InvalidSyntaxException("Extraneous trailing characters at \"" + filterstring.substr(pos) + "\"", filterstring);
      }
      return filter;
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_Filter() throw(InvalidSyntaxException) {
      FilterImpl::Pointer filter;
      SkipWhiteSpace();

      if (filterstring.at(pos) != '(') {
        throw InvalidSyntaxException(FILTER_MISSING_LEFTPAREN + filterstring.substr(pos) + "\"", filterstring);
      }

      pos++;

      filter = Parse_FilterComp();

      SkipWhiteSpace();

      if (filterstring.at(pos) != ')') {
        throw InvalidSyntaxException(FILTER_MISSING_RIGHTPAREN + filterstring.substr(pos) + "\"", filterstring);
      }

      pos++;

      SkipWhiteSpace();

      return filter;
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_FilterComp() throw(InvalidSyntaxException) {
      SkipWhiteSpace();

      char c = filterstring.at(pos);

      switch (c) {
        case '&' : {
          pos++;
          return Parse_And();
        }
        case '|' : {
          pos++;
          return Parse_Or();
        }
        case '!' : {
          pos++;
          return Parse_Not();
        }
      }
      return Parse_Item();
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_And() throw(InvalidSyntaxException) {
      SkipWhiteSpace();

      if (filterstring.at(pos) != '(') {
        throw InvalidSyntaxException(FILTER_MISSING_LEFTPAREN + filterstring.substr(pos) + "\"", filterstring);
      }

      ObjectVector<FilterImpl::Pointer>::Pointer operands(new ObjectVector<FilterImpl::Pointer>());

      while (filterstring.at(pos) == '(') {
        FilterImpl::Pointer child = Parse_Filter();
        operands->push_back(child);
      }

      FilterImpl::Pointer result(new FilterImpl(FilterImpl::AND, "", operands));
      return result;
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_Or() throw(InvalidSyntaxException) {
      SkipWhiteSpace();

      if (filterstring.at(pos) != '(') {
        throw InvalidSyntaxException(FILTER_MISSING_LEFTPAREN + filterstring.substr(pos) + "\"", filterstring);
      }

      ObjectVector<FilterImpl::Pointer>::Pointer operands(new ObjectVector<FilterImpl::Pointer>());

      while (filterstring.at(pos) == '(') {
        FilterImpl::Pointer child = Parse_Filter();
        operands->push_back(child);
      }

      FilterImpl::Pointer result(new FilterImpl(FilterImpl::OR, "", operands));
      return result;
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_Not() throw(InvalidSyntaxException) {
      SkipWhiteSpace();

      if (filterstring.at(pos) != '(') {
        throw InvalidSyntaxException(FILTER_MISSING_LEFTPAREN + filterstring.substr(pos) + "\"", filterstring);
      }

      FilterImpl::Pointer child = Parse_Filter();

      FilterImpl::Pointer result(new FilterImpl(FilterImpl::NOT, "", child));
      return result;
    }

    FilterImpl::Pointer FilterImpl::Parser::Parse_Item() throw(InvalidSyntaxException) {
      std::string attr = Parse_Attr();

      SkipWhiteSpace();

      switch (filterstring.at(pos)) {
        case '~' : {
          if (filterstring.at(pos + 1) == '=') {
            pos += 2;
            FilterImpl::Pointer result(new FilterImpl(FilterImpl::APPROX, attr, Parse_Value()));
            return result;
          }
          break;
        }
        case '>' : {
          if (filterstring.at(pos + 1) == '=') {
            pos += 2;
            FilterImpl::Pointer result(new FilterImpl(FilterImpl::GREATER, attr, Parse_Value()));
            return result;
          }
          break;
        }
        case '<' : {
          if (filterstring.at(pos + 1) == '=') {
            pos += 2;
            FilterImpl::Pointer result(new FilterImpl(FilterImpl::LESS, attr, Parse_Value()));
            return result;
          }
          break;
        }
        case '=' : {
          if (filterstring.at(pos + 1) == '*') {
            int oldpos = pos;
            pos += 2;
            SkipWhiteSpace();
            if (filterstring.at(pos) == ')') {
              FilterImpl::Pointer result(new FilterImpl(FilterImpl::PRESENT, attr, Object::Pointer(0)));
              return result;
            }
            pos = oldpos;
          }

          pos++;
          Object::Pointer string = Parse_Substring();

          if (string.Cast<ObjectString>()) {
            FilterImpl::Pointer result(new FilterImpl(FilterImpl::EQUAL, attr, string));
            return result;
          }

          FilterImpl::Pointer result(new FilterImpl(FilterImpl::SUBSTRING, attr, string));
          return result;
        }
      }

      throw InvalidSyntaxException("Invalid operator at \"" + filterstring.substr(pos) + "\"", filterstring);
    }

    std::string FilterImpl::Parser::Parse_Attr() throw(InvalidSyntaxException) {
      SkipWhiteSpace();

      std::size_t begin = pos;
      std::size_t end = pos;

      char c = filterstring.at(pos);

      while (c != '~' && c != '<' && c != '>' && c != '=' && c != '(' && c != ')') {
        pos++;

        if (!EqualWhitespace()(c)) {
          end = pos;
        }

        c = filterstring.at(pos);
      }

      std::size_t length = end - begin;

      if (length == 0) {
        throw InvalidSyntaxException("Missing attr at \"" + filterstring.substr(pos) + "\"", filterstring);
      }

      return filterstring.substr(begin, length);
    }

    Object::Pointer FilterImpl::Parser::Parse_Value() throw(InvalidSyntaxException) {
      std::string sb;

      bool breakParseLoop = false;
      while (true) {
        char c = filterstring.at(pos);

        switch (c) {
          case ')' : {
            breakParseLoop = true;
            break;
          }

          case '(' : {
            throw InvalidSyntaxException("Invalid value at \"" + filterstring.substr(pos) + "\"", filterstring);
          }

          case '\\' : {
            pos++;
            c = filterstring.at(pos);
            /* fall through into default */
          }

          default : {
            sb.push_back(c);
            pos++;
            break;
          }
        }
        if (breakParseLoop) break;
      }

      if (sb.empty()) {
        throw InvalidSyntaxException("Missing value at \"" + filterstring.substr(pos) + "\"", filterstring);
      }

      ObjectString::Pointer result(new ObjectString(sb));
      return result;
    }

    Object::Pointer FilterImpl::Parser::Parse_Substring() throw(InvalidSyntaxException) {
      std::string sb;

      ObjectVector<std::string>::Pointer operands(new ObjectVector<std::string>());

      bool breakParseLoop = false;
      while (true) {
        char c = filterstring.at(pos);

        switch (c) {
          case ')' : {
            if (!sb.empty()) {
              operands->push_back(sb);
            }

            breakParseLoop = true;
            break;
          }

          case '(' : {
            throw InvalidSyntaxException("Invalid value at \"" + filterstring.substr(pos) + "\"", filterstring);
          }

          case '*' : {
            if (!sb.empty()) {
              operands->push_back(sb);
            }

            sb.clear();

            operands->push_back("");
            pos++;

            break;
          }

          case '\\' : {
            pos++;
            c = filterstring.at(pos);
            /* fall through into default */
          }

          default : {
            sb.push_back(c);
            pos++;
            break;
          }
        }

        if (breakParseLoop) break;
      }

      std::size_t size = operands->size();

      if (size == 0) {
        throw InvalidSyntaxException("Missing value at \"" + filterstring.substr(pos) + "\"", filterstring);
      }

      if (size == 1) {
        ObjectString::Pointer single(new ObjectString(operands->operator[](0)));

        if (!single->empty()) {
          return single;
        }
      }

      return operands;
    }

    void FilterImpl::Parser::SkipWhiteSpace() {
      EqualWhitespace isWhitespace;
      for (std::size_t length = filterstring.size(); (pos < length) && isWhitespace(filterstring[pos]);) {
        pos++;
      }
    }


    FilterImpl::ServiceReferenceDictionary::ServiceReferenceDictionary(SmartPointer<ServiceReference> reference)
    : reference(reference)
    {

    }

    const Dictionary::Value FilterImpl::ServiceReferenceDictionary::Get(const Key& key) const {
      if (!reference) {
        return Value();
      }
      return reference->GetProperty(key);
    }

    const Dictionary::Value& FilterImpl::ServiceReferenceDictionary::operator[](const Key& key) const {
      throw UnsupportedOperationException();
    }

    void FilterImpl::ServiceReferenceDictionary::Clear() {
      throw UnsupportedOperationException();
    }

   bool FilterImpl::ServiceReferenceDictionary::ContainsKey(const Key& key) {
      throw UnsupportedOperationException();
    }

   bool FilterImpl::ServiceReferenceDictionary::ContainsValue(const Value& value) {
      throw UnsupportedOperationException();
    }


   std::set<Dictionary::Entry> FilterImpl::ServiceReferenceDictionary::EntrySet() const {
      throw UnsupportedOperationException();
    }

   std::set<Dictionary::Key> FilterImpl::ServiceReferenceDictionary::KeySet() const {
      throw UnsupportedOperationException();
    }

   std::vector<Dictionary::ConstValue> FilterImpl::ServiceReferenceDictionary::Values() const {
      throw UnsupportedOperationException();
    }

   std::vector<Dictionary::Value> FilterImpl::ServiceReferenceDictionary::Values() {
      throw UnsupportedOperationException();
    }


   Dictionary::Value FilterImpl::ServiceReferenceDictionary::Get(const Key& key) {
      throw UnsupportedOperationException();
    }


   Dictionary::Value FilterImpl::ServiceReferenceDictionary::Put(const Key& key, const Value& value) {
      throw UnsupportedOperationException();
    }

   void FilterImpl::ServiceReferenceDictionary::PutAll(const Dictionary& dict) {
      throw UnsupportedOperationException();
    }



   Dictionary::Value& FilterImpl::ServiceReferenceDictionary::operator[](const Key& key) {
      throw UnsupportedOperationException();
    }


   Dictionary::Value FilterImpl::ServiceReferenceDictionary::Remove(const Key& key) {
      throw UnsupportedOperationException();
    }

   std::size_t FilterImpl::ServiceReferenceDictionary::Size() const {
      throw UnsupportedOperationException();
    }


    bool FilterImpl::ServiceReferenceDictionary::IsEmpty() const {
      throw UnsupportedOperationException();
    }

}
}
}
