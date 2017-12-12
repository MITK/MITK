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

#include "mapRegistration.h"

#include "mitkPointSetMappingHelper.h"
#include "mitkRegistrationHelper.h"


#include "mapPointSetMappingTask.h"

::map::core::continuous::Elements<3>::InternalPointSetType::Pointer mitk::PointSetMappingHelper::ConvertPointSetMITKtoMAP(const mitk::PointSet::DataType* mitkSet)
{
  if (! mitkSet) mapDefaultExceptionStaticMacro(<< "Error, cannot convert point set. Passed mitk point set is null.");

  ::map::core::continuous::Elements<3>::InternalPointSetType::Pointer mapSet = ::map::core::continuous::Elements<3>::InternalPointSetType::New();
  ::map::core::continuous::Elements<3>::InternalPointSetType::PointsContainer::Pointer mapContainer = ::map::core::continuous::Elements<3>::InternalPointSetType::PointsContainer::New();
  ::map::core::continuous::Elements<3>::InternalPointSetType::PointDataContainer::Pointer mapDataContainer = ::map::core::continuous::Elements<3>::InternalPointSetType::PointDataContainer::New();
  mapSet->SetPoints(mapContainer);
  mapSet->SetPointData(mapDataContainer);

  unsigned int pointCount = mitkSet->GetNumberOfPoints();

  for (unsigned int pointId = 0; pointId < pointCount; ++pointId)
  {
    mapSet->SetPoint(pointId, mitkSet->GetPoint(pointId));

    mitk::PointSet::PointDataType data;
    if (mitkSet->GetPointData(pointId,&data))
    {
      mapSet->SetPointData(pointId,data.id);
    }
  }

  return mapSet;
}

::mitk::PointSet::Pointer
  mitk::PointSetMappingHelper::map(const ::mitk::PointSet* input, const mitk::PointSetMappingHelper::RegistrationType* registration, int timeStep,
  bool throwOnMappingError, const ::mitk::PointSet::PointDataType& errorPointValue)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map point set. Passed registration wrapper pointer is nullptr.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map point set. Passed point set pointer is nullptr.";
  }
  if (static_cast<int>(input->GetTimeSteps())<=timeStep && timeStep>=0)
  {
    mitkThrow() << "Cannot set point set. Selected time step is larger then mitk point set. MITK time step count: "<<input->GetTimeSteps()<<"; selected time step: "<<timeStep;
  }

  ::mitk::PointSet::Pointer result = input->Clone();

  typedef ::map::core::continuous::Elements<3>::InternalPointSetType MAPPointSetType;
  typedef ::map::core::Registration<3,3> ConcreteRegistrationType;
  const ConcreteRegistrationType* castedReg = dynamic_cast<const ConcreteRegistrationType*>(registration);
  if (!castedReg)
  {
    mitkThrow() <<"Moving and/or fixed dimension of the registration is not 3. Cannot map point 3D set.";
  }

  typedef ::map::core::PointSetMappingTask<ConcreteRegistrationType, MAPPointSetType, MAPPointSetType> MappingTaskType;
  MappingTaskType::ErrorPointValueType internalErrorValue = itk::NumericTraits<MappingTaskType::ErrorPointValueType>::NonpositiveMin();
  MappingTaskType::Pointer spTask = MappingTaskType::New();
  spTask->setRegistration(castedReg);
  spTask->setThrowOnMappingError(throwOnMappingError);
  spTask->setErrorPointValue(internalErrorValue);

  unsigned int timePos = timeStep;
  unsigned int timeEndPos = timeStep+1;
  if (timeStep < 0)
  {
    timePos = 0;
    timeEndPos = input->GetTimeSteps();
  }

  while (timePos<timeEndPos)
  {
    MAPPointSetType::Pointer inputTempSet = ConvertPointSetMITKtoMAP(input->GetPointSet(timePos));
    spTask->setInputPointSet(inputTempSet);
    spTask->execute();

    MAPPointSetType::Pointer mappedSet = spTask->getResultPointSet();

    unsigned int pointCount = input->GetSize(timePos);

    for (unsigned int pointId = 0; pointId < pointCount; ++pointId)
    {
      result->SetPoint(pointId, mappedSet->GetPoint(pointId), timePos);
      bool invalid = true;
      MAPPointSetType::PixelType mappedData;
      if (mappedSet->GetPointData(pointId,&mappedData))
      {
        invalid = mappedData == internalErrorValue;
      }

      if (invalid)
      {
        result->GetPointSet(timePos)->GetPointData()->SetElement(pointId,errorPointValue);
      }
      else
      {
        result->GetPointSet(timePos)->GetPointData()->SetElement(pointId,input->GetPointSet(timePos)->GetPointData()->GetElement(pointId));
      }
    }

    ++timePos;
  }

  return result;
}

::mitk::PointSet::Pointer
  mitk::PointSetMappingHelper::map(const ::mitk::PointSet* input, const MITKRegistrationType* registration, int timeStep,
  bool throwOnMappingError, const ::mitk::PointSet::PointDataType& errorPointValue)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map point set. Passed registration wrapper pointer is nullptr.";
  }
  if (!registration->GetRegistration())
  {
    mitkThrow() << "Cannot map point set. Passed registration wrapper containes no registration.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map point set. Passed point set pointer is nullptr.";
  }

  ::mitk::PointSet::Pointer result = map(input, registration->GetRegistration(), timeStep, throwOnMappingError, errorPointValue);
  return result;
}
