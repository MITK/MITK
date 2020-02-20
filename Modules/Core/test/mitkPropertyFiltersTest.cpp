/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
#include <utility>
// MITK includes
#include <mitkCoreServices.h>
#include <mitkIPropertyFilters.h>
#include <mitkProperties.h>
#include <mitkPropertyFilter.h>
// VTK includes
#include <vtkDebugLeaks.h>

class mitkPropertyFiltersTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyFiltersTestSuite);
  MITK_TEST(GetPropertyFiltersService_Success);
  MITK_TEST(ApplyGlobalPropertyFilter_Success);
  MITK_TEST(ApplyRestrictedPropertyFilter_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::IPropertyFilters *m_PropertyFilters;
  typedef std::map<std::string, mitk::BaseProperty::Pointer> PropertyMap;
  typedef PropertyMap::const_iterator PropertyMapConstIterator;
  PropertyMap m_PropertyMap;
  mitk::PropertyFilter m_Filter;
  mitk::PropertyFilter m_RestrictedFilter;
  PropertyMap m_FilteredPropertyMap;
  PropertyMapConstIterator m_It1;
  PropertyMapConstIterator m_It2;
  PropertyMapConstIterator m_It3;

public:
  void setUp()
  {
    m_PropertyFilters = mitk::CoreServices::GetPropertyFilters();

    m_PropertyMap.insert(std::make_pair("propertyName1", mitk::BoolProperty::New().GetPointer()));
    m_PropertyMap.insert(std::make_pair("propertyName2", mitk::BoolProperty::New().GetPointer()));
    m_PropertyMap.insert(std::make_pair("propertyName3", mitk::BoolProperty::New().GetPointer()));

    m_Filter.AddEntry("propertyName1", mitk::PropertyFilter::Whitelist);
    m_Filter.AddEntry("propertyName2", mitk::PropertyFilter::Whitelist);

    m_RestrictedFilter.AddEntry("propertyName2", mitk::PropertyFilter::Blacklist);

    m_PropertyFilters->AddFilter(m_Filter);
    m_PropertyFilters->AddFilter(m_RestrictedFilter, "className");
  }
  void tearDown()
  {
    m_PropertyFilters = nullptr;
  }

  void GetPropertyFiltersService_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Get property filters service", m_PropertyFilters != nullptr);
  }

  void ApplyGlobalPropertyFilter_Success()
  {
    m_FilteredPropertyMap = m_PropertyFilters->ApplyFilter(m_PropertyMap);
    m_It1 = m_FilteredPropertyMap.find("propertyName1");
    m_It2 = m_FilteredPropertyMap.find("propertyName2");
    m_It3 = m_FilteredPropertyMap.find("propertyName3");
    
    CPPUNIT_ASSERT_MESSAGE("Apply global property filter",
      m_It1 != m_FilteredPropertyMap.end() && m_It2 != m_FilteredPropertyMap.end() && m_It3 == m_FilteredPropertyMap.end());
  }

  void ApplyRestrictedPropertyFilter_Success()
  {
    m_FilteredPropertyMap = m_PropertyFilters->ApplyFilter(m_PropertyMap, "className");
    m_It1 = m_FilteredPropertyMap.find("propertyName1");
    m_It2 = m_FilteredPropertyMap.find("propertyName2");
    m_It3 = m_FilteredPropertyMap.find("propertyName3");
    
    CPPUNIT_ASSERT_MESSAGE("Apply restricted property filter (also respects global filter)",
      m_It1 != m_FilteredPropertyMap.end() && m_It2 == m_FilteredPropertyMap.end() && m_It3 == m_FilteredPropertyMap.end());
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkPropertyFilters)
