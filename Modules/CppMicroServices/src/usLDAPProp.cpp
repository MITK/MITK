/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usLDAPProp.h>

#include <stdexcept>

namespace us {

LDAPPropExpr::LDAPPropExpr(const std::string& expr)
  : m_ldapExpr(expr)
{}

LDAPPropExpr& LDAPPropExpr::operator!()
{
  if (m_ldapExpr.empty()) return *this;

  m_ldapExpr = "(!" + m_ldapExpr + ")";
  return *this;
}

LDAPPropExpr::operator std::string() const
{
  return m_ldapExpr;
}

bool LDAPPropExpr::IsNull() const
{
  return m_ldapExpr.empty();
}


LDAPProp::LDAPProp(const std::string& property)
  : m_property(property)
{
  if (m_property.empty()) throw std::invalid_argument("property must not be empty");
}

LDAPPropExpr LDAPProp::operator==(const std::string& s) const
{
  if (s.empty()) return LDAPPropExpr(s);
  return LDAPPropExpr("(" + m_property + "=" + s + ")");
}

LDAPPropExpr LDAPProp::operator==(const us::Any& any) const
{
  return operator==(any.ToString());
}

LDAPProp::operator LDAPPropExpr () const
{
  return LDAPPropExpr("(" + m_property + "=*)");
}

LDAPPropExpr LDAPProp::operator!() const
{
  return LDAPPropExpr("(!(" + m_property + "=*))");
}

LDAPPropExpr LDAPProp::operator!=(const std::string& s) const
{
  if (s.empty()) return LDAPPropExpr(s);
  return LDAPPropExpr("(!(" + m_property + "=" + s + "))");
}

LDAPPropExpr LDAPProp::operator!=(const us::Any& any) const
{
  return operator!=(any.ToString());
}

LDAPPropExpr LDAPProp::operator>=(const std::string& s) const
{
  if (s.empty()) return LDAPPropExpr(s);
  return LDAPPropExpr("(" + m_property + ">=" + s + ")");
}

LDAPPropExpr LDAPProp::operator>=(const us::Any& any) const
{
  return operator>=(any.ToString());
}

LDAPPropExpr LDAPProp::operator<=(const std::string& s) const
{
  if (s.empty()) return LDAPPropExpr(s);
  return LDAPPropExpr("(" + m_property + "<=" + s + ")");
}

LDAPPropExpr LDAPProp::operator<=(const us::Any& any) const
{
  return operator<=(any.ToString());
}

LDAPPropExpr LDAPProp::Approx(const std::string& s) const
{
  if (s.empty()) return LDAPPropExpr(s);
  return LDAPPropExpr("(" + m_property + "~=" + s + ")");
}

LDAPPropExpr LDAPProp::Approx(const us::Any& any) const
{
  return Approx(any.ToString());
}

}

us::LDAPPropExpr operator&&(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right)
{
  if (left.IsNull()) return right;
  if (right.IsNull()) return left;
  return us::LDAPPropExpr("(&" + static_cast<std::string>(left) + static_cast<std::string>(right) + ")");
}

us::LDAPPropExpr operator||(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right)
{
  if (left.IsNull()) return right;
  if (right.IsNull()) return left;
  return us::LDAPPropExpr("(|" + static_cast<std::string>(left) + static_cast<std::string>(right) + ")");
}
