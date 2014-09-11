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
#include "mitkNavigationDataObjectVisualizationFilter.h"

#include "mitkDataStorage.h"


mitk::NavigationDataObjectVisualizationFilter::NavigationDataObjectVisualizationFilter()
: NavigationDataToNavigationDataFilter(),
m_RepresentationList(), m_TransformPosition(), m_TransformOrientation(), m_RotationMode(RotationStandard)
{
}


mitk::NavigationDataObjectVisualizationFilter::~NavigationDataObjectVisualizationFilter()
{
  m_RepresentationList.clear();
  m_OffsetList.clear();
}


const mitk::BaseData* mitk::NavigationDataObjectVisualizationFilter::GetRepresentationObject(unsigned int idx)
{
  RepresentationPointerMap::const_iterator iter = m_RepresentationList.find(idx);
  if (iter != m_RepresentationList.end())
    return iter->second;

  return NULL;
}

mitk::AffineTransform3D::Pointer mitk::NavigationDataObjectVisualizationFilter::GetOffset(int index)
{
  OffsetPointerMap::const_iterator iter = m_OffsetList.find(index);
  if (iter != m_OffsetList.end())
    return iter->second;
  return NULL;
}


void mitk::NavigationDataObjectVisualizationFilter::SetRepresentationObject(unsigned int idx, BaseData* data)
{
  m_RepresentationList[idx] = RepresentationPointer(data);
}

void mitk::NavigationDataObjectVisualizationFilter::SetOffset(int index, mitk::AffineTransform3D::Pointer offset)
{
m_OffsetList[index] = offset;
}


void mitk::NavigationDataObjectVisualizationFilter::SetRotationMode(RotationMode r)
{
  m_RotationMode = r;
}


void mitk::NavigationDataObjectVisualizationFilter::GenerateData()
{
  /*get each input, lookup the associated BaseData and transfer the data*/
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
  for (unsigned int index=0; index < inputs.size(); index++)
  {
    //get the needed variables
    const mitk::NavigationData* nd = this->GetInput(index);
    assert(nd);

    mitk::NavigationData* output = this->GetOutput(index);
    assert(output);

    //check if the data is valid
    if (!nd->IsDataValid())
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(nd); // copy all information from input to output
    const mitk::BaseData* data = this->GetRepresentationObject(index);
    if (data == NULL)
    {
      MITK_WARN << "No BaseData associated with input " << index;
      continue;
    }

    //get the transform from data
    mitk::AffineTransform3D::Pointer affineTransform = data->GetGeometry()->GetIndexToWorldTransform();
    if (affineTransform.IsNull())
    {
      MITK_WARN << "AffineTransform IndexToWorldTransform not initialized!";
      continue;
    }

    //check for offset
    mitk::AffineTransform3D::Pointer offset = this->GetOffset(index);

    //store the current scaling to set it after transformation
    mitk::Vector3D spacing = data->GetGeometry()->GetSpacing();
    //clear spacing of data to be able to set it again afterwards
    ScalarType scale[] = {1.0, 1.0, 1.0};
    data->GetGeometry()->SetSpacing(scale);

    /*now bring quaternion to affineTransform by using vnl_Quaternion*/
    affineTransform->SetIdentity();

    if (this->GetTransformOrientation(index) == true)
    {
      mitk::NavigationData::OrientationType orientation = nd->GetOrientation();

      /* because of an itk bug, the transform can not be calculated with float data type.
      To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
      static AffineTransform3D::MatrixType m;

      //convert quaternion to rotation matrix depending on the rotation mode
      if(m_RotationMode == RotationStandard)
        {
        //calculate the transform from the quaternions
        static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();
        // convert mitk::ScalarType quaternion to double quaternion because of itk bug
        vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
        quatTransform->SetIdentity();
        quatTransform->SetRotation(doubleQuaternion);
        quatTransform->Modified();
        mitk::TransferMatrix(quatTransform->GetMatrix(), m);
        }

      else if(m_RotationMode == RotationTransposed)
        {
        vnl_matrix_fixed<mitk::ScalarType,3,3> rot = orientation.rotation_matrix_transpose();
        for(int i=0; i<3; i++) for (int j=0; j<3; j++) m[i][j] = rot[i][j];
        }
      affineTransform->SetMatrix(m);

    }
    if (this->GetTransformPosition(index) == true)
    {
      ///*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
      mitk::Vector3D pos;
      pos.SetVnlVector(nd->GetPosition().GetVnlVector());
      affineTransform->SetOffset(pos);
    }
    affineTransform->Modified();


    //set the transform to data
    if(offset.IsNotNull()) //first use offset if there is one.
      {
      mitk::AffineTransform3D::Pointer overallTransform = mitk::AffineTransform3D::New();
      overallTransform->SetIdentity();
      overallTransform->Compose(offset);
      overallTransform->Compose(affineTransform);
      data->GetGeometry()->SetIndexToWorldTransform(overallTransform);
      }
    else
      {
      data->GetGeometry()->SetIndexToWorldTransform(affineTransform);
      }

    //set the original spacing to keep scaling of the geometrical object
    data->GetGeometry()->SetSpacing(spacing);
    data->GetGeometry()->Modified();
    data->Modified();
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
  }
}


void mitk::NavigationDataObjectVisualizationFilter::SetTransformPosition( unsigned int index, bool applyTransform )
{
  itkDebugMacro("setting TransformPosition for index " << index << " to " << applyTransform);
  BooleanInputMap::const_iterator it = this->m_TransformPosition.find(index);
  if ((it != this->m_TransformPosition.end()) && (it->second == applyTransform))
    return;

  this->m_TransformPosition[index] = applyTransform;
  this->Modified();
}


bool mitk::NavigationDataObjectVisualizationFilter::GetTransformPosition( unsigned int index ) const
{
  itkDebugMacro("returning TransformPosition for index " << index);
  BooleanInputMap::const_iterator it = this->m_TransformPosition.find(index);
  if (it != this->m_TransformPosition.end())
    return it->second;
  else
    return true; // default to true
}


void mitk::NavigationDataObjectVisualizationFilter::TransformPositionOn( unsigned int index )
{
  this->SetTransformPosition(index, true);
}


void mitk::NavigationDataObjectVisualizationFilter::TransformPositionOff( unsigned int index )
{
  this->SetTransformPosition(index, false);
}


void mitk::NavigationDataObjectVisualizationFilter::SetTransformOrientation( unsigned int index, bool applyTransform )
{
  itkDebugMacro("setting TransformOrientation for index " << index << " to " << applyTransform);
  BooleanInputMap::const_iterator it = this->m_TransformOrientation.find(index);
  if ((it != this->m_TransformOrientation.end()) && (it->second == applyTransform))
    return;

  this->m_TransformOrientation[index] = applyTransform;
  this->Modified(); \
}


bool mitk::NavigationDataObjectVisualizationFilter::GetTransformOrientation( unsigned int index ) const
{
  itkDebugMacro("returning TransformOrientation for index " << index);
  BooleanInputMap::const_iterator it = this->m_TransformOrientation.find(index);
  if (it != this->m_TransformOrientation.end())
    return it->second;
  else
    return true; // default to true
}


void mitk::NavigationDataObjectVisualizationFilter::TransformOrientationOn( unsigned int index )
{
  this->SetTransformOrientation(index, true);
}


void mitk::NavigationDataObjectVisualizationFilter::TransformOrientationOff( unsigned int index )
{
  this->SetTransformOrientation(index, false);
}
