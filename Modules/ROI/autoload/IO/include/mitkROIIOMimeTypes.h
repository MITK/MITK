/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIIOMimeTypes_h
#define mitkROIIOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkROIIOExports.h>

namespace mitk
{
  /** \brief MIME type definitions for the MITK ROI file format. */
  namespace MitkROIIOMimeTypes
  {
    /**
     * \brief Custom MIME type for MITK ROI files.
     *
     * Identifies MITK ROI (region of interest) files by checking the file
     * extension and content.
     */
    class MITKROIIO_EXPORT MitkROIMimeType : public CustomMimeType
    {
    public:
      MitkROIMimeType();

      /**
       * \brief Check whether the given file path matches this MIME type.
       * \param path Path to the file to check.
       * \return \c true if the file is recognized as a MITK ROI file.
       */
      bool AppliesTo(const std::string& path) const override;

      /** \brief Create a copy of this MIME type object. */
      MitkROIMimeType* Clone() const override;
    };

    /** \brief Get the ROI MIME type instance. */
    MITKROIIO_EXPORT MitkROIMimeType ROI_MIMETYPE();

    /** \brief Get the name string for the ROI MIME type. */
    MITKROIIO_EXPORT std::string ROI_MIMETYPE_NAME();

    /**
     * \brief Get all MIME types defined in this module.
     * \return A vector of pointers to CustomMimeType objects. Caller takes ownership.
     */
    MITKROIIO_EXPORT std::vector<CustomMimeType*> Get();
  }
}

#endif
