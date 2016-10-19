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

#ifndef mitkIDICOMTagsOfInterest_h
#define mitkIDICOMTagsOfInterest_h

#include <mitkServiceInterface.h>
#include <string>
#include <vector>
#include <mitkDICOMTag.h>
#include <MitkDICOMReaderExports.h>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief Interface of DICOM tags of interest service.
   *
   * This service allows you to manage the tags of interest (toi).
   * All registred toi will be extracted when loading dicom data and stored as properties in the corresponding
   * base data object. In addition the service can (if available) use IPropertyPersistance and IPropertyAliases
   * to ensure that the tags of interests are also persisted and have a human readable alias.
   */
  class MITKDICOMREADER_EXPORT IDICOMTagsOfInterest
  {
  public:
    virtual ~IDICOMTagsOfInterest();

    typedef std::unordered_map<const char*, DICOMTag> DICOMTagMapType;

    /** \brief Add an tag to the TOI.
      * If the tag was already added it will be overwritten with the passed values.
      * \param[in] tag Tag that should be added.
      * \param[in] makePersistant Indicates if the tag should be made persistant if possible via the IPropertyPersistence service.
      */
    virtual void AddTagOfInterest(const DICOMTag& tag, bool makePersistant = true) = 0;

    /** Returns the map of all tags of interest. Key is the property name. Value is the DICOM tag.*/
    virtual DICOMTagMapType GetTagsOfInterest() const = 0;

    /** Indicates if the given tag is already a tag of interest.*/
    virtual bool HasTag(const DICOMTag& tag) const = 0;

    /** \brief Remove specific tag. If it not exists the function will do nothing.
      * \param[in] tag Tag that should be removed.
      */
    virtual void RemoveTag(const DICOMTag& tag) = 0;

    /** \brief Remove all tags.
      */
    virtual void RemoveAllTags() = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IDICOMTagsOfInterest, "org.mitk.IDICOMTagsOfInterest")

#endif
