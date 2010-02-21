/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-09 18:14:41 +0200 (Mo, 09 Jul 2007) $
Version:   $Revision: 11185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkNavigationDataObjectVisualizationFilter.h"

#include "mitkDataStorage.h"


mitk::NavigationDataObjectVisualizationFilter::NavigationDataObjectVisualizationFilter()
: NavigationDataToNavigationDataFilter(), 
m_RepresentationList(), m_TransformPosition(), m_TransformOrientation()
{
}


mitk::NavigationDataObjectVisualizationFilter::~NavigationDataObjectVisualizationFilter()
{
  m_RepresentationList.clear();
}


const mitk::BaseData* mitk::NavigationDataObjectVisualizationFilter::GetRepresentationObject(unsigned int idx)
{
  //if (idx >= this->GetNumberOfInputs())
  //  return NULL;
  
  //const NavigationData* nd = this->GetInput(idx);  
  //if (nd == NULL)
  //  return NULL;

  RepresentationPointerMap::const_iterator iter = m_RepresentationList.find(idx);
  if (iter != m_RepresentationList.end())
    return iter->second;

  return NULL;
}


void mitk::NavigationDataObjectVisualizationFilter::SetRepresentationObject(unsigned int idx, BaseData* data)
{
  //if (idx >= this->GetNumberOfInputs())
  //  return false;

  //const NavigationData* nd = this->GetInput(idx);

  //if (nd == NULL || data == NULL)
  //  return false;

  m_RepresentationList[idx] = RepresentationPointer(data);
  //std::pair<RepresentationPointerMap::iterator, bool> returnEl; //pair for returning the result
  //returnEl = m_RepresentationList.insert( RepresentationPointerMap::value_type(nd, data) ); //insert the given elements  
  //return returnEl.second; // return if insert was successful 
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
      itkWarningMacro("NavigationDataObjectVisualizationFilter: Wrong/No BaseData associated with input.");
      return;
    }
    
    //get the transform from data
    mitk::AffineTransform3D::Pointer affineTransform = data->GetGeometry()->GetIndexToWorldTransform();
    if (affineTransform.IsNull())
    {
      //replace with mitk standard output
      itkWarningMacro("NavigationDataObjectVisualizationFilter: AffineTransform IndexToWorldTransform not initialized!");
      return;
    }

    //store the current scaling to set it after transformation
    mitk::Vector3D spacing = data->GetGeometry()->GetSpacing();
    //clear spacing of data to be able to set it again afterwards
    float scale[] = {1.0, 1.0, 1.0};
    data->GetGeometry()->SetSpacing(scale);

    /*now bring quaternion to affineTransform by using vnl_Quaternion*/
    affineTransform->SetIdentity();


    if (this->GetTransformOrientation(index) == true)
    {
      //calculate the transform from the quaternions
      static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

      mitk::NavigationData::OrientationType orientation = nd->GetOrientation();
      // convert mitk::ScalarType quaternion to double quaternion because of itk bug
      vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
      quatTransform->SetIdentity();
      quatTransform->SetRotation(doubleQuaternion);
      quatTransform->Modified();

      /* because of an itk bug, the transform can not be calculated with float data type. 
      To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
      static AffineTransform3D::MatrixType m;
      mitk::TransferMatrix(quatTransform->GetMatrix(), m);
      affineTransform->SetMatrix(m);
    }
    if (this->GetTransformPosition(index) == true)
    {
      ///*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
      mitk::Vector3D pos;
      pos.Set_vnl_vector(nd->GetPosition().Get_vnl_vector());
      affineTransform->SetOffset(pos);    
    }
    affineTransform->Modified();
    //set the transform to data
    data->GetGeometry()->SetIndexToWorldTransform(affineTransform);    
    //set the original spacing to keep scaling of the geometrical object
    data->GetGeometry()->SetSpacing(spacing);
    data->GetGeometry()->TransferItkToVtkTransform(); // update VTK Transform for rendering too
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
  this->Modified(); \
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
