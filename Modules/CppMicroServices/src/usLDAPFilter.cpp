/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usLDAPFilter.h>
#include "usLDAPExpr_p.h"
#include "usServicePropertiesImpl_p.h"
#include <usServiceReference.h>
#include "usServiceReferenceBasePrivate.h"

#include <stdexcept>

namespace us {

class LDAPFilterData : public SharedData
{
public:

  LDAPFilterData() : ldapExpr()
  {}

  LDAPFilterData(const std::string& filter)
    : ldapExpr(filter)
  {}

  LDAPFilterData(const LDAPFilterData& other)
    : SharedData(other), ldapExpr(other.ldapExpr)
  {}

  LDAPExpr ldapExpr;
};

LDAPFilter::LDAPFilter()
  : d(nullptr)
{
}

LDAPFilter::LDAPFilter(const std::string& filter)
  : d(nullptr)
{
  try
  {
    d = new LDAPFilterData(filter);
  }
  catch (const std::exception& e)
  {
    throw std::invalid_argument(e.what());
  }
}

LDAPFilter::LDAPFilter(const LDAPFilter& other)
  : d(other.d)
{
}

LDAPFilter::~LDAPFilter()
{
}

LDAPFilter::operator bool_type() const
{
  return d.ConstData() != nullptr ? &LDAPFilter::d : nullptr;
}

bool LDAPFilter::Match(const ServiceReferenceBase& reference) const
{
  return d->ldapExpr.Evaluate(reference.d->GetProperties(), true);
}

bool LDAPFilter::Match(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(ServicePropertiesImpl(dictionary), false);
}

bool LDAPFilter::MatchCase(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(ServicePropertiesImpl(dictionary), true);
}

std::string LDAPFilter::ToString() const
{
  return d->ldapExpr.ToString();
}

bool LDAPFilter::operator==(const LDAPFilter& other) const
{
  return d->ldapExpr.ToString() == other.d->ldapExpr.ToString();
}

LDAPFilter& LDAPFilter::operator=(const LDAPFilter& filter)
{
  d = filter.d;

  return *this;
}

}

using namespace us;

std::ostream& operator<<(std::ostream& os, const LDAPFilter& filter)
{
  return os << filter.ToString();
}
