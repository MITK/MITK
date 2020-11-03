/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGeometryData.h"
#include "mitkImage.h"
#include "mitkPointSet.h"
#include "mitkSurface.h"

// this include after all specific type includes! (for mitk::Equal)
#include "mitkBaseDataCompare.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usServiceProperties.h"

bool mitk::BaseDataCompare::AreEqual(const BaseData *left, const BaseData *right, ScalarType eps, bool verbose)
{
  // Do basic tests that are valid for all types here.
  // Let specializations only implement a meaningful
  // comparison of two non-null objects of same type
  if (left == nullptr && right == nullptr)
    return true;

  if (AreSameClasses(left, right, verbose))
  {
    if (left->GetUID() != right->GetUID())
    {
      return false;
    }
    return InternalAreEqual(*left, *right, eps, verbose);
  }

  return false;
}

bool mitk::BaseDataCompare::AreSameClasses(const BaseData *left, const BaseData *right, bool verbose)
{
  if (left == nullptr && right == nullptr)
    return true;

  if (left == nullptr && right != nullptr)
  {
    if (verbose)
      MITK_WARN << "Left data is nullptr, right data is not (type " << right->GetNameOfClass() << ")";
    return false;
  }

  if (left != nullptr && right == nullptr)
  {
    if (verbose)
      MITK_WARN << "Right data is nullptr, left data is not (type " << left->GetNameOfClass() << ")";
    return false;
  }

  // two real BaseData objects, need to really compare
  if (left->GetNameOfClass() != right->GetNameOfClass())
  {
    if (verbose)
      MITK_WARN << "Mismatch: Left data is '" << left->GetNameOfClass() << "', "
                << "right data is '" << right->GetNameOfClass() << "'";
    return false;
  }

  return true;
}

void mitk::BaseDataCompare::RegisterCoreEquals()
{
  static bool comparatorsCreated = false;
  if (!comparatorsCreated)
  {
    static BaseDataCompareT<Image> imageEqual;
    us::ServiceProperties imageProperties;
    imageProperties["basedata"] = std::string(Image::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataCompare>(&imageEqual, imageProperties);

    static BaseDataCompareTNonConst<Surface> surfaceEqual;
    us::ServiceProperties surfaceProperties;
    surfaceProperties["basedata"] = std::string(Surface::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataCompare>(&surfaceEqual, surfaceProperties);

    static BaseDataCompareT<PointSet> pointsetEqual;
    us::ServiceProperties pointsetProperties;
    pointsetProperties["basedata"] = std::string(PointSet::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataCompare>(&pointsetEqual, pointsetProperties);

    static BaseDataCompareT<GeometryData> geometryDataEqual;
    us::ServiceProperties geometryDataProperties;
    geometryDataProperties["basedata"] = std::string(GeometryData::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataCompare>(&geometryDataEqual, geometryDataProperties);

    comparatorsCreated = true;
  }
}
