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

#include "mitkLDAPFilter.h"

#include "mitkLDAPExpr_p.h"
#include "mitkServiceReferencePrivate.h"

namespace mitk {

class LDAPFilterData : public SharedData
{
public:

  LDAPFilterData()
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
  : d(0)
{
}

LDAPFilter::LDAPFilter(const std::string& filter)
  : d(0)
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

LDAPFilter::operator bool() const
{
  return d;
}

bool LDAPFilter::Match(const ServiceReference& reference) const
{
  return d->ldapExpr.Evaluate(reference.d->GetProperties(), true);
}

bool LDAPFilter::Match(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(dictionary, false);
}

bool LDAPFilter::MatchCase(const ServiceProperties& dictionary) const
{
  return d->ldapExpr.Evaluate(dictionary, true);
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

std::ostream& operator<<(std::ostream& os, const mitk::LDAPFilter& filter)
{
  return os << filter.ToString();
}
