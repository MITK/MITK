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


#define _USE_MATH_DEFINES
#include "mitkTractographyForest.h"
#include <mitkExceptionMacro.h>
#include <mitkGeometry3D.h>

namespace mitk
{

TractographyForest::TractographyForest( std::shared_ptr< vigra::RandomForest<int> > forest )
{
  m_Forest = forest;
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  SetGeometry(geometry);
}

TractographyForest::~TractographyForest()
{

}

void TractographyForest::PredictProbabilities(vigra::MultiArray<2, float>& features, vigra::MultiArray<2, float>& probabilities) const
{
  m_Forest->predictProbabilities(features, probabilities);
}

int TractographyForest::GetNumFeatures() const
{
  if (!HasForest())
    mitkThrow() << "Forest is NULL";
  return m_Forest->feature_count();
}

int TractographyForest::GetNumTrees() const
{
  if (!HasForest())
    mitkThrow() << "Forest is NULL";
  return m_Forest->tree_count();
}

int TractographyForest::GetNumClasses() const
{
  if (!HasForest())
    mitkThrow() << "Forest is NULL";
  return m_Forest->class_count();
}

int TractographyForest::GetMaxTreeDepth() const
{
  if (!HasForest())
    mitkThrow() << "Forest is NULL";
  return m_Forest->ext_param_.max_tree_depth;
}

int TractographyForest::IndexToClassLabel(int idx) const
{
  if (!HasForest())
    mitkThrow() << "Forest is NULL";
  unsigned int classLabel = 0;
  m_Forest->ext_param_.to_classlabel(idx, classLabel);
  return classLabel;
}

bool TractographyForest::HasForest() const
{
  return m_Forest!=nullptr;
}

void TractographyForest::PrintSelf(std::ostream &os, itk::Indent indent) const
{
//  Superclass::PrintSelf(os, indent);
  os << indent << this->GetNameOfClass() << ":\n";
  os << indent << "Number of features: " << GetNumFeatures() << std::endl;
  os << indent << "Number of classes: " << GetNumClasses() << std::endl;
  os << indent << "Number of trees: " << GetNumTrees() << std::endl;
  os << indent << "Maximum tree depth: " << GetMaxTreeDepth() << std::endl;
}

/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */
void TractographyForest::UpdateOutputInformation()
{

}
void TractographyForest::SetRequestedRegionToLargestPossibleRegion()
{

}
bool TractographyForest::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool TractographyForest::VerifyRequestedRegion()
{
  return true;
}
void TractographyForest::SetRequestedRegion(const itk::DataObject* )
{

}

}
