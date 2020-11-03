/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTImageDetectionHelper.h"

#include "mitkCESTPropertyHelper.h"
#include "mitkImage.h"
#include "mitkDataNode.h"
#include "mitkNodePredicateFunction.h"

bool mitk::IsAnyCESTImage(const Image* cestImage)
{
  return IsCESTorWasabiImage(cestImage) || IsCESTT1Image(cestImage);
};

bool mitk::IsCESTorWasabiImage(const Image* cestImage)
{
  if (!cestImage) return false;

  auto prop = cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_OFFSETS().c_str());

  return prop.IsNotNull();
};

bool mitk::IsCESTT1Image(const Image* cestImage)
{
  if (!cestImage) return false;

  auto prop = cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_TREC().c_str());

  return prop.IsNotNull();
};

mitk::NodePredicateBase::Pointer mitk::CreateAnyCESTImageNodePredicate()
{
  auto cestCheck = [](const mitk::DataNode * node)
  {
    if (node)
    {
      return mitk::IsAnyCESTImage(dynamic_cast<const mitk::Image*>(node->GetData()));
    }
    return false;
  };

  return mitk::NodePredicateFunction::New(cestCheck).GetPointer();
};

mitk::NodePredicateBase::Pointer mitk::CreateCESTorWasabiImageNodePredicate()
{
  auto cestCheck = [](const mitk::DataNode * node)
  {
    if (node)
    {
      return mitk::IsCESTorWasabiImage(dynamic_cast<const mitk::Image*>(node->GetData()));
    }
    return false;
  };

  return mitk::NodePredicateFunction::New(cestCheck).GetPointer();
};

mitk::NodePredicateBase::Pointer mitk::CreateCESTT1ImageNodePredicate()
{
  auto cestCheck = [](const mitk::DataNode * node)
  {
    if (node)
    {
      return mitk::IsCESTT1Image(dynamic_cast<const mitk::Image*>(node->GetData()));
    }
    return false;
  };

  return mitk::NodePredicateFunction::New(cestCheck).GetPointer();
};
