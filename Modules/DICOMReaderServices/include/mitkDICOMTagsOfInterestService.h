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

#ifndef mitkDICOMTagsOfInterestService_h
#define mitkDICOMTagsOfInterestService_h

#include <string>
#include <vector>
#include <set>
#include <mitkIDICOMTagsOfInterest.h>

#include <itkFastMutexLock.h>
#include <itkMutexLockHolder.h>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief DICOM tags of interest service.
   *
   * This service allows you to manage the tags of interest (toi).
   * All registred toi will be extracted when loading dicom data and stored as properties in the corresponding
   * base data object. In addition the service can (if available) use IPropertyPersistance and IPropertyDescriptions
   * to ensure that the tags of interests are also persisted and have a human readable descriptions.
   */
  class  DICOMTagsOfInterestService: public IDICOMTagsOfInterest
  {
  public:
    DICOMTagsOfInterestService();
    ~DICOMTagsOfInterestService() override;

    void AddTagOfInterest(const DICOMTagPath& tag, bool makePersistant = true) override;

    DICOMTagPathMapType GetTagsOfInterest() const override;

    bool HasTag(const DICOMTagPath& tag) const override;

    void RemoveTag(const DICOMTagPath& tag) override;

    void RemoveAllTags() override;

  private:

    typedef std::set<DICOMTagPath> InternalTagSetType;
    typedef itk::MutexLockHolder<itk::SimpleFastMutexLock> MutexHolder;

    InternalTagSetType m_Tags;
    mutable itk::SimpleFastMutexLock m_Lock;

    DICOMTagsOfInterestService(const DICOMTagsOfInterestService&);
    DICOMTagsOfInterestService& operator=(const DICOMTagsOfInterestService&);
  };
}

#endif
