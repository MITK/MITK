/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOMetaInformationPropertyConstants_h
#define mitkIOMetaInformationPropertyConstants_h

#include <MitkCoreExports.h>

#include <mitkPropertyKeyPath.h>

namespace mitk
{
  /**
   * \ingroup IO
   * \brief Property key path constants for IO meta information.
   *
   * These constants define the property key paths used to store meta information
   * about how data was read, including the reader description, MITK version,
   * MIME type, and input location.
   */
  struct MITKCORE_EXPORT IOMetaInformationPropertyConstants
  {
    /** \brief Path to the property containing the name of the reader used. */
    static PropertyKeyPath READER_DESCRIPTION();

    /** \brief Path to the property containing the MITK version used to read the data. */
    static PropertyKeyPath READER_VERSION();

    /** \brief Path to the property containing the MIME name detected for reading. */
    static PropertyKeyPath READER_MIME_NAME();

    /** \brief Path to the property containing the MIME category detected for reading. */
    static PropertyKeyPath READER_MIME_CATEGORY();

    /** \brief Path to the property containing the input file location used to read the data. */
    static PropertyKeyPath READER_INPUTLOCATION();

    /** \brief Path to the property containing split reason information for the read volume. */
    static PropertyKeyPath VOLUME_SPLIT_REASON();

    /** \brief Path to the root node of reader option properties. */
    static PropertyKeyPath READER_OPTION_ROOT();

    /** \brief Path to any reader option property (wildcard). */
    static PropertyKeyPath READER_OPTIONS_ANY();
  };
}

#endif
