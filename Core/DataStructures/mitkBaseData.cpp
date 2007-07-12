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
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>
#include <itkObjectFactoryBase.h>
#include <itkSmartPointerForwardReference.txx>

//template class itk::SmartPointerForwardReference<mitk::BaseProcess>;

#define MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED

const std::string mitk::BaseData::XML_NODE_NAME = "data";

//##ModelId=3E3FE04202B9
mitk::BaseData::BaseData() : 
  m_RequestedRegionInitialized(false), m_SmartSourcePointer(NULL), 
  m_SourceOutputIndexDuplicate(0), m_Initialized(true), 
  m_Unregistering(false), m_CalculatingExternalReferenceCount(false), 
  m_ExternalReferenceCount(-1)
{
  m_TimeSlicedGeometry = TimeSlicedGeometry::New();
  m_PropertyList = PropertyList::New(); 
}

//##ModelId=3E3FE042031D
mitk::BaseData::~BaseData() 
{
  m_SmartSourcePointer = NULL;
}

void mitk::BaseData::UpdateOutputInformation()
{
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  if(m_TimeSlicedGeometry.IsNotNull())
    m_TimeSlicedGeometry->UpdateInformation();
}

const mitk::TimeSlicedGeometry* mitk::BaseData::GetUpdatedTimeSlicedGeometry()
{
  SetRequestedRegionToLargestPossibleRegion();

  UpdateOutputInformation();

  return GetTimeSlicedGeometry();
}

//##ModelId=3DCBE2BA0139
const mitk::Geometry3D* mitk::BaseData::GetUpdatedGeometry(int t)
{
  SetRequestedRegionToLargestPossibleRegion();

  UpdateOutputInformation();

  return GetGeometry(t);
}

void mitk::BaseData::SetGeometry(Geometry3D* aGeometry3D)
{
  if(aGeometry3D!=NULL)
  {
    TimeSlicedGeometry::Pointer timeSlicedGeometry = dynamic_cast<TimeSlicedGeometry*>(aGeometry3D);
    if(timeSlicedGeometry.IsNull())
    {
      if((m_TimeSlicedGeometry.IsNotNull()) && (m_TimeSlicedGeometry->GetGeometry3D(0)==aGeometry3D) && m_TimeSlicedGeometry->GetTimeSteps()==1)
        return;
      timeSlicedGeometry = TimeSlicedGeometry::New();
      m_TimeSlicedGeometry = timeSlicedGeometry;   
      timeSlicedGeometry->InitializeEvenlyTimed(aGeometry3D, 1);
    }
    else
    {
      if(aGeometry3D == m_TimeSlicedGeometry)
        return;
    }
    m_TimeSlicedGeometry = timeSlicedGeometry;
  }
  else
  {
    if(m_TimeSlicedGeometry.IsNull())
      return;
    m_TimeSlicedGeometry = NULL;
  }
  Modified();
}

void mitk::BaseData::SetClonedGeometry(const Geometry3D* aGeometry3D)
{
  SetGeometry(static_cast<mitk::Geometry3D*>(aGeometry3D->Clone().GetPointer()));
}

bool mitk::BaseData::IsEmpty(int) const
{
  return IsInitialized() == false;
}

bool mitk::BaseData::IsEmpty() const
{
  if(IsInitialized() == false)
    return true;
  const TimeSlicedGeometry* timeGeometry = const_cast<BaseData*>(this)->GetUpdatedTimeSlicedGeometry();
  if(timeGeometry == NULL)
    return true;
  unsigned int timeSteps = timeGeometry->GetTimeSteps();
  for ( unsigned int t = 0 ; t < timeSteps ; ++t )
  {
    if(IsEmpty(t) == false)
      return false;
  }
  return true;
}

itk::SmartPointerForwardReference<mitk::BaseProcess> mitk::BaseData::GetSource() const
{
  return static_cast<mitk::BaseProcess*>(Superclass::GetSource().GetPointer());
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
      // the order of the following boolean statement is important:
      // this->GetSource() returns a SmartPointerForwardReference,
      // which increases and afterwards decreases the reference count,
      // which may result in an ExternalReferenceCount of 0, causing
      // BaseProcess::UnRegister() to destroy us (also we already
      // about to do that).
      if((this->m_SmartSourcePointer->GetExternalReferenceCount()==0) || (this->GetSource()==NULL))
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

unsigned long mitk::BaseData::GetMTime() const
{
  unsigned long time = Superclass::GetMTime();
  if(m_TimeSlicedGeometry.IsNotNull())
  {
    if((time < m_TimeSlicedGeometry->GetMTime()))
    {
      Modified();
      return Superclass::GetMTime();
    }
    //unsigned long geometryTime = m_TimeSlicedGeometry->GetMTime();
    //if(time < geometryTime)
    //{
    //  return geometryTime;
    //}
  }
  return time;
}

bool mitk::BaseData::IsInitialized() const
{
  return m_Initialized;
}

void mitk::BaseData::Clear()
{
  if(m_Initialized)
  {
    m_Initialized = false;

    ReleaseData();
  }
}

void mitk::BaseData::ExecuteOperation(mitk::Operation* /*operation*/)
{
  //empty by default. override if needed!
}

void mitk::BaseData::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " TimeSlicedGeometry: ";
  if(GetTimeSlicedGeometry() == NULL)
    os << "NULL" << std::endl;
  else
    GetTimeSlicedGeometry()->Print(os, indent);
}

bool mitk::BaseData::WriteXMLData( XMLWriter& xmlWriter ) 
{
  if ( m_TimeSlicedGeometry )
    this->m_TimeSlicedGeometry->WriteXML( xmlWriter );

  return true;
}

bool mitk::BaseData::ReadXMLData( XMLReader& xmlReader ) 
{
  if ( xmlReader.Goto( Geometry3D::XML_NODE_NAME ) ) 
  {
    this->m_TimeSlicedGeometry = dynamic_cast<mitk::TimeSlicedGeometry*>( xmlReader.CreateObject().GetPointer() );
    if ( this->m_TimeSlicedGeometry.IsNotNull() ) this->m_TimeSlicedGeometry->ReadXMLData( xmlReader );
    xmlReader.GotoParent();
  }
  return true;
}

const std::string& mitk::BaseData::GetXMLNodeName() const
{ 
  return XML_NODE_NAME;
}
