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
#include "mitkNavigationDataVisualizationByBaseDataTransformFilter.h"

#include "mitkDataStorage.h"


mitk::NavigationDataVisualizationByBaseDataTransformFilter::NavigationDataVisualizationByBaseDataTransformFilter()
{}


mitk::NavigationDataVisualizationByBaseDataTransformFilter::~NavigationDataVisualizationByBaseDataTransformFilter()
{}


const mitk::BaseData* mitk::NavigationDataVisualizationByBaseDataTransformFilter::GetBaseData(const NavigationData* nd) const
{
  if (nd == NULL)
    return NULL;

  RepresentationPointerMap::const_iterator iter = m_RepresentationList.find(nd);
  if (iter != m_RepresentationList.end())
    return (*iter).second;
  
  //else:
  return NULL;
}

bool mitk::NavigationDataVisualizationByBaseDataTransformFilter::SetBaseData(const NavigationData* nd, BaseData* data)
{
  if (nd == NULL || data == NULL)
    return false;

  //pair for returning the result
  std::pair<RepresentationPointerMap::iterator, bool> returnEl;
  
  //insert the given elements
  returnEl = m_RepresentationList.insert( RepresentationPointerMap::value_type(nd, data) );
  
  return returnEl.second;
}

void mitk::NavigationDataVisualizationByBaseDataTransformFilter::GenerateData()
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
        
    //check if the dada is valid
    if (!nd->IsDataValid())
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(nd); // copy all information from input to output
    const mitk::BaseData* data = this->GetBaseData(nd);
    if (data == NULL)
    {
      itkWarningMacro("Wrong BaseData associated with NavigationData!");
      return;
    }
    
    //get the transform from data
    mitk::AffineTransform3D::Pointer affineTransform = data->GetGeometry()->GetIndexToWorldTransform();
    if (affineTransform.IsNull())
    {
      //replace with mitk standard output
      itkWarningMacro("AffineTransform IndexToWorldTransform not initialized!");
      return;
    }

    //store the current scaling to set it after transformation
    mitk::Vector3D spacing = data->GetGeometry()->GetSpacing();
    //clear spacing of data to be able to set it again afterwards
    float scale[] = {1.0, 1.0, 1.0};
    data->GetGeometry()->SetSpacing(scale);

    /*now bring quaternion to affineTransform by using vnl_Quaternion*/
    affineTransform->SetIdentity();

    //calculate the transform from the quaternions
    static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

    mitk::NavigationData::OrientationType orientation = nd->GetOrientation();
    // convert mitk::Scalartype quaternion to double quaternion because of itk bug
    vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
    quatTransform->SetIdentity();
    quatTransform->SetRotation(doubleQuaternion);
    quatTransform->Modified();

    /* because of an itk bug, the transform can not be calculated with float datatype. 
    To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
    static AffineTransform3D::MatrixType m;
    mitk::TransferMatrix(quatTransform->GetMatrix(), m);
    affineTransform->SetMatrix(m);
    
    ///*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
    mitk::Vector3D pos;
    pos.Set_vnl_vector(nd->GetPosition().Get_vnl_vector());
    affineTransform->SetOffset(pos);
    affineTransform->Modified();
    
    //set the transform to data
    data->GetGeometry()->SetIndexToWorldTransform(affineTransform);

    //set the original spacing to keep scaling of the geometrical object
    data->GetGeometry()->SetSpacing(spacing);

    data->GetGeometry()->Modified();
    data->Modified();

    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
  }
}



