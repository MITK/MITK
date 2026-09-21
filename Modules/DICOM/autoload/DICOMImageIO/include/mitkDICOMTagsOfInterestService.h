/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagsOfInterestService_h
#define mitkDICOMTagsOfInterestService_h

#include <string>
#include <mutex>
#include <vector>
#include <map>
#include <mitkIDICOMTagsOfInterest.h>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief DICOM tags of interest service.
   *
   * This service allows you to manage the tags of interest (toi).
   * All registered toi will be extracted when loading dicom data and stored as properties in the corresponding
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

    /** Registers description and, on request, persistence for one published key
        shape. Separate from AddTagOfInterest because a functional-group-rooted
        tag of interest is scanned under its root but published under its
        frame-relative path, so both forms need a registration while only the
        root belongs in the scan set. */
    void RegisterDescriptionAndPersistence(const DICOMTagPath& tagPath, bool makePersistant);
    void UnregisterDescriptionAndPersistence(const DICOMTagPath& tagPath);

    /** Re-registers a published key that a removal just cleared, if any
        remaining tag of interest still publishes under it. */
    void RestoreKeyIfStillWanted(const DICOMTagPath& publishedKey);

    /** The registered tags, each mapped to the persistence request it was added
        with. The flag is kept because removing one functional-group root has to
        re-register the frame-relative key its surviving siblings still need, and
        can only do so faithfully if it knows what they asked for. */
    typedef std::map<DICOMTagPath, bool> InternalTagSetType;
    typedef std::lock_guard<std::mutex> MutexHolder;

    InternalTagSetType m_Tags;
    mutable std::mutex m_Lock;

    DICOMTagsOfInterestService(const DICOMTagsOfInterestService&);
    DICOMTagsOfInterestService& operator=(const DICOMTagsOfInterestService&);
  };
}

#endif
