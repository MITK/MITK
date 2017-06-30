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

#include "QmitkMappingJob.h"

// Mitk
#include <mitkImageAccessByItk.h>
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMatchPointPropertyTags.h>
#include <mitkPointSetMappingHelper.h>
#include <mitkProperties.h>

// Qt
#include <QThreadPool>

#include <mapEvents.h>

QmitkMappingJobSettings::QmitkMappingJobSettings()
{
  m_doGeometryRefinement = false;
  m_MappedName = "";
  m_allowUndefPixels = true;
  m_paddingValue = 0;
  m_allowUnregPixels = true;
  m_errorValue = 0;
  m_InterpolatorType = mitk::ImageMappingInterpolator::Linear;
};

const mitk::Image *QmitkMappingJob::GetInputDataAsImage() const
{
  return dynamic_cast<const mitk::Image *>(m_spInputData.GetPointer());
};

const mitk::PointSet *QmitkMappingJob::GetInputDataAsPointSet() const
{
  return dynamic_cast<const mitk::PointSet *>(m_spInputData.GetPointer());
};

const map::core::RegistrationBase *QmitkMappingJob::GetRegistration() const
{
  const mitk::MAPRegistrationWrapper *wrapper =
    dynamic_cast<const mitk::MAPRegistrationWrapper *>(m_spRegNode->GetData());

  return dynamic_cast<const map::core::RegistrationBase *>(wrapper->GetRegistration());
};

void QmitkMappingJob::OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event)
{
  const map::events::AnyMatchPointEvent *pMAPEvent = dynamic_cast<const map::events::AnyMatchPointEvent *>(&event);

  if (pMAPEvent)
  {
    emit AlgorithmInfo(QString::fromStdString(pMAPEvent->getComment()));
  }
}

QmitkMappingJob::QmitkMappingJob()
{
  m_spRefGeometry = nullptr;

  m_spCommand = ::itk::MemberCommand<QmitkMappingJob>::New();
  m_spCommand->SetCallbackFunction(this, &QmitkMappingJob::OnMapAlgorithmEvent);
};

QmitkMappingJob::~QmitkMappingJob(){};

void QmitkMappingJob::run()
{
  const mitk::Image *inputImage = this->GetInputDataAsImage();
  const mitk::PointSet *inputSet = this->GetInputDataAsPointSet();
  m_spMappedData = nullptr;

  if (m_doGeometryRefinement)
  {
    try
    {
      mitk::Image::Pointer spResultImage = nullptr;

      if (inputImage)
      {
        spResultImage = mitk::ImageMappingHelper::refineGeometry(inputImage, this->GetRegistration(), true);
      }

      m_spMappedData = spResultImage;

      if (spResultImage.IsNotNull())
      {
        emit MapResultIsAvailable(spResultImage.GetPointer(), this);
      }
      else
      {
        emit Error(QString("Error when when refining image geometry."));
      }
    }
    catch (std::exception &e)
    {
      emit Error(QString("Error when refining image geometry. Error description: ") + QString::fromLatin1(e.what()));
    }
    catch (...)
    {
      emit Error(QString("Unknown error when refining image geometry."));
    }
  }
  else
  {
    try
    {
      mitk::BaseData::Pointer spResultData = nullptr;

      if (inputImage)
      {
        spResultData = mitk::ImageMappingHelper::map(this->GetInputDataAsImage(),
                                                     this->GetRegistration(),
                                                     !(this->m_allowUndefPixels),
                                                     this->m_paddingValue,
                                                     this->m_spRefGeometry,
                                                     !(this->m_allowUnregPixels),
                                                     this->m_errorValue,
                                                     this->m_InterpolatorType)
                         .GetPointer();
      }
      else if (inputSet)
      {
        mitk::PointSet::PointDataType errorValue;
        errorValue.id = -1;
        errorValue.pointSpec = mitk::PTUNDEFINED;
        errorValue.selected = false;
        spResultData = mitk::PointSetMappingHelper::map(inputSet, this->GetRegistration(), -1, false, errorValue);
      }

      if (spResultData.IsNotNull())
      {
        emit MapResultIsAvailable(spResultData, this);
      }
      else
      {
        emit Error(QString("Error when mapping input data to result."));
      }

      m_spMappedData = spResultData;
    }
    catch (std::exception &e)
    {
      emit Error(QString("Error when mapping data. Error description: ") + QString::fromLatin1(e.what()));
    }
    catch (...)
    {
      emit Error(QString("Unkown error when mapping data."));
    }
  }
};
