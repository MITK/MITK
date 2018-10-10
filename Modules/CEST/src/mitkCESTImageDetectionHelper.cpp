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

#include "mitkCESTImageDetectionHelper.h"

#include "mitkCustomTagParser.h"
#include "mitkImage.h"
#include "mitkDataNode.h"
#include "mitkNodePredicateFunction.h"

bool mitk::IsAnyCESTImage(const Image* cestImage)
{
  if (!cestImage) return false;

  auto prop = cestImage->GetProperty(mitk::CEST_PROPERTY_NAME_TOTALSCANTIME().c_str());
 
  return prop.IsNotNull();
};

bool mitk::IsCESTorWasabiImage(const Image* cestImage)
{
  if (!cestImage) return false;

  return IsAnyCESTImage(cestImage) && !IsCESTT1Image(cestImage);
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
