/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkBaseData.h"
#include "mitkBaseProcess.h"

#define MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED

//##ModelId=3E3FE04202B9
mitk::BaseData::BaseData() : m_SmartSourcePointer(NULL), m_SourceOutputIndexDuplicate(0), m_Unregistering(false), m_CalculatingExternalReferenceCount(false), m_ExternalReferenceCount(-1)
{
  m_Geometry3D = Geometry3D::New();
  m_PropertyList = PropertyList::New();
  
}

//##ModelId=3E3FE042031D
mitk::BaseData::~BaseData() 
{
  m_SmartSourcePointer = NULL;
}

//##ModelId=3DCBE2BA0139
const mitk::Geometry3D* mitk::BaseData::GetUpdatedGeometry()
{
  SetRequestedRegionToLargestPossibleRegion();

  UpdateOutputInformation();

  return m_Geometry3D.GetPointer();
}

mitk::Geometry3D* mitk::BaseData::GetGeometry() const
{
  return m_Geometry3D.GetPointer();
}

void mitk::BaseData::SetGeometry(Geometry3D* aGeometry3D)
{
  m_Geometry3D = aGeometry3D;
}

itk::SmartPointerForwardReference<mitk::BaseProcess> mitk::BaseData::GetSource() const
{
  return m_SmartSourcePointer.GetPointer();
}

//##ModelId=3E8600DB02DC
int mitk::BaseData::GetExternalReferenceCount() const
{
  if(m_CalculatingExternalReferenceCount==false) //this is only needed because a smart-pointer to m_Outputs (private!!) must be created by calling GetOutputs.
  {
    m_CalculatingExternalReferenceCount = true;

    m_ExternalReferenceCount = -1;

    int realReferenceCount = GetReferenceCount();

    if(GetSource()==NULL) 
    {
      m_ExternalReferenceCount = realReferenceCount;
      m_CalculatingExternalReferenceCount = false;
      return m_ExternalReferenceCount;
    }

    mitk::BaseProcess::DataObjectPointerArray outputs = m_SmartSourcePointer->GetOutputs();

    unsigned int idx;
    for (idx = 0; idx < outputs.size(); ++idx)
    {
      //references of outputs that are not referenced from someone else (reference additional to the reference from this BaseProcess object) are interpreted as non-existent 
      if(outputs[idx]==this)
        --realReferenceCount;
    }
    m_ExternalReferenceCount = realReferenceCount;
    if(m_ExternalReferenceCount<0)
      m_ExternalReferenceCount=0;
    m_CalculatingExternalReferenceCount = false;
  }
  else
    return -1;
  return m_ExternalReferenceCount;
}

//##ModelId=3E8600DB0188
void mitk::BaseData::UnRegister() const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  if(GetReferenceCount()>1)
  {
    Superclass::UnRegister();
    if((m_Unregistering==false) && (m_SmartSourcePointer.IsNotNull()))
    {
      m_Unregistering=true;
      if(m_SmartSourcePointer->GetExternalReferenceCount()==0)
        m_SmartSourcePointer=NULL; // now the reference count is zero and this object has been destroyed; thus nothing may be done after this line!!
      else
        m_Unregistering=false;
    }
  }
  else
#endif
    Superclass::UnRegister(); // now the reference count is zero and this object has been destroyed; thus nothing may be done after this line!!
}

//##ModelId=3E8600DC0053
void mitk::BaseData::ConnectSource(itk::ProcessObject *arg, unsigned int idx) const
{
#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  itkDebugMacro( "connecting source  " << arg
    << ", source output index " << idx);

  if ( GetSource() != arg || m_SourceOutputIndexDuplicate != idx)
  {
    m_SmartSourcePointer = dynamic_cast<mitk::BaseProcess*>(arg);
    m_SourceOutputIndexDuplicate = idx;
    Modified();
  }
#endif
}



mitk::PropertyList::Pointer mitk::BaseData::GetPropertyList() const
{
	  return m_PropertyList;
}


mitk::BaseProperty::Pointer mitk::BaseData::GetProperty(const char *propertyKey) const
{
   return m_PropertyList->GetProperty(propertyKey);
}

void mitk::BaseData::SetProperty(const char *propertyKey,
                                     BaseProperty* propertyValue)
{
    m_PropertyList->SetProperty(propertyKey, propertyValue);
}

void mitk::BaseData::SetPropertyList(PropertyList *pList)
{
    m_PropertyList = pList;
}


void mitk::BaseData::ExecuteOperation(mitk::Operation* operation)
{
  //empty by default. override if needed!
}
