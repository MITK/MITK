/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkBaseDataEqual.h"

#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkPointSet.h"

#include "usServiceProperties.h"
#include "usModuleContext.h"
#include "usGetModuleContext.h"

bool mitk::BaseDataEqual::AreEqual(const BaseData* left, const BaseData* right, double eps, bool verbose)
{
  // Do basic tests that are valid for all types here.
  // Let specializations only implement a meaningful
  // comparison of two non-null objects of same type
  if (left == nullptr && right == nullptr)
    return true;

  if (AreSameClasses(left, right, verbose))
  {
    return InternalAreEqual(*left, *right, eps, verbose);
  }

  return false;
}

bool mitk::BaseDataEqual::AreSameClasses(const BaseData* left, const BaseData* right, bool verbose)
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
  if ( left->GetNameOfClass() != right->GetNameOfClass() )
  {
    if ( verbose )
      MITK_WARN << "Mismatch: Left data is '" << left->GetNameOfClass() << "', "
                << "right data is '" << right->GetNameOfClass() << "'";
    return false;
  }

  return true;
}

void mitk::BaseDataEqual::RegisterCoreEquals()
{
  static bool comparatorsCreated = false;
  if (!comparatorsCreated)
  {
    static BaseDataEqualT<Image> imageEqual;
    us::ServiceProperties imageProperties;
    imageProperties["basedata"] = std::string(Image::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataEqual>(&imageEqual, imageProperties);

    static BaseDataEqualT<Surface> surfaceEqual;
    us::ServiceProperties surfaceProperties;
    surfaceProperties["basedata"] =std::string(Surface::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataEqual>(&surfaceEqual, surfaceProperties);

    static BaseDataEqualT<PointSet> pointsetEqual;
    us::ServiceProperties pointsetProperties;
    pointsetProperties["basedata"] = std::string(PointSet::GetStaticNameOfClass());
    us::GetModuleContext()->RegisterService<BaseDataEqual>(&pointsetEqual, pointsetProperties);

    comparatorsCreated = true;
  }
}
