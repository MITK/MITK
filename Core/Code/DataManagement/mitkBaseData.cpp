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


#include "mitkBaseData.h"
#include <itkObjectFactoryBase.h>


mitk::BaseData::BaseData() :
  m_RequestedRegionInitialized(false),
  m_SourceOutputIndexDuplicate(0), m_Initialized(true)
{
  m_TimeSlicedGeometry = TimeSlicedGeometry::New();
  m_PropertyList = PropertyList::New();
}

mitk::BaseData::BaseData( const BaseData &other ):
itk::DataObject(), mitk::OperationActor(),
m_RequestedRegionInitialized(other.m_RequestedRegionInitialized),
m_SourceOutputIndexDuplicate(other.m_SourceOutputIndexDuplicate),
m_Initialized(other.m_Initialized)
{
  m_TimeSlicedGeometry = dynamic_cast<mitk::TimeSlicedGeometry*>(other.m_TimeSlicedGeometry->Clone().GetPointer());
  m_PropertyList = other.m_PropertyList->Clone();
}

mitk::BaseData::~BaseData()
{

}

void mitk::BaseData::InitializeTimeSlicedGeometry(unsigned int timeSteps)
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->Initialize();

 if ( timeSteps > 1 )
 {
    mitk::ScalarType timeBounds[] = {0.0, 1.0};
    g3d->SetTimeBounds( timeBounds );
 }

  // The geometry is propagated automatically to the other items,
  // if EvenlyTimed is true...
  timeGeometry->InitializeEvenlyTimed( g3d.GetPointer(), timeSteps );
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

void mitk::BaseData::Expand( unsigned int timeSteps )
{
  if( m_TimeSlicedGeometry.IsNotNull() )
    m_TimeSlicedGeometry->ExpandToNumberOfTimeSteps( timeSteps );
}

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
    if ( timeSlicedGeometry.IsNotNull() )
      m_TimeSlicedGeometry = timeSlicedGeometry;
    else
    {
      timeSlicedGeometry = TimeSlicedGeometry::New();
      m_TimeSlicedGeometry = timeSlicedGeometry;
      timeSlicedGeometry->InitializeEvenlyTimed(aGeometry3D, 1);
    }
    Modified();
  }
  else if( m_TimeSlicedGeometry.IsNotNull() )
  {
    m_TimeSlicedGeometry = NULL;
    Modified();
  }
  return;
}

void mitk::BaseData::SetClonedGeometry(const Geometry3D* aGeometry3D)
{
  SetGeometry(static_cast<mitk::Geometry3D*>(aGeometry3D->Clone().GetPointer()));
}

void mitk::BaseData::SetClonedGeometry(const Geometry3D* aGeometry3D, unsigned int time)
{
  if (m_TimeSlicedGeometry)
  {
    m_TimeSlicedGeometry->SetGeometry3D(static_cast<mitk::Geometry3D*>(aGeometry3D->Clone().GetPointer()), time);
  }
}

bool mitk::BaseData::IsEmptyTimeStep(unsigned int) const
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
    if(IsEmptyTimeStep(t) == false)
      return false;
  }
  return true;
}

itk::SmartPointer<mitk::BaseDataSource> mitk::BaseData::GetSource() const
{
  return static_cast<mitk::BaseDataSource*>(Superclass::GetSource().GetPointer());
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

void mitk::BaseData::SetOrigin(const mitk::Point3D& origin)
{
  mitk::TimeSlicedGeometry* timeSlicedGeometry = GetTimeSlicedGeometry();

  assert(timeSlicedGeometry!=NULL);

  mitk::Geometry3D* geometry;

  unsigned int steps = timeSlicedGeometry->GetTimeSteps();

  for(unsigned int timestep = 0; timestep < steps; ++timestep)
  {
    geometry = GetGeometry(timestep);
    if(geometry != NULL)
    {
      geometry->SetOrigin(origin);
    }
    if(GetTimeSlicedGeometry()->GetEvenlyTimed())
    {
      GetTimeSlicedGeometry()->InitializeEvenlyTimed(geometry, steps);
      break;
    }
  }
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

void mitk::BaseData::Graft(const itk::DataObject*)
{
  itkExceptionMacro(<< "Graft not implemented for mitk::BaseData subclass " << this->GetNameOfClass())
}

void mitk::BaseData::CopyInformation( const itk::DataObject* data )
{
  const Self* bd = dynamic_cast<const Self*>(data);
  if (bd != NULL)
  {
    m_TimeSlicedGeometry = dynamic_cast<TimeSlicedGeometry*>(bd->GetTimeSlicedGeometry()->Clone().GetPointer());
    m_PropertyList = bd->GetPropertyList()->Clone();
  }
  else
  {
    // pointer could not be cast back down; this can be the case if your filters input
    // and output objects differ in type; then you have to write your own GenerateOutputInformation method
    itkExceptionMacro(<< "mitk::BaseData::CopyInformation() cannot cast "
      << typeid(data).name() << " to "
      << typeid(Self*).name() );
  }

}

bool mitk::BaseData::IsInitialized() const
{
  return m_Initialized;
}

void mitk::BaseData::Clear()
{
  this->ClearData();
  this->InitializeEmpty();
}

void mitk::BaseData::ClearData()
{
  if(m_Initialized)
  {
    ReleaseData();
    m_Initialized = false;
  }
}

void mitk::BaseData::ExecuteOperation(mitk::Operation* /*operation*/)
{
  //empty by default. override if needed!
}

void mitk::BaseData::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << std::endl;
  os << indent << " TimeSlicedGeometry: ";
  if(GetTimeSlicedGeometry() == NULL)
    os << "NULL" << std::endl;
  else
    GetTimeSlicedGeometry()->Print(os, indent);
}

