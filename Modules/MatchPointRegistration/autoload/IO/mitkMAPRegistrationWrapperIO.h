/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMAPRegistrationWrapperIO_h
#define mitkMAPRegistrationWrapperIO_h

#include <mitkAbstractFileIO.h>

#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
  /**
   * \brief Provides IO capability for MatchPoint registration wrappers.
   *
   * Reads and writes mitk::MAPRegistrationWrapper objects to/from
   * MatchPoint registration files.
   */
  class MAPRegistrationWrapperIO : public AbstractFileIO
  {
  public:
    /** \brief Default constructor. Registers reader/writer for the MAP registration MIME type. */
    MAPRegistrationWrapperIO();

    // -------------- AbstractFileReader -------------
    using AbstractFileReader::Read;

    /** \brief Return the confidence level for reading the given file as a registration wrapper. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /** \brief Write the registration wrapper to the configured output location. */
    void Write() override;

    /** \brief Return the confidence level for writing the given data as a registration wrapper. */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Perform the actual reading of registration wrapper data.
     * \return A vector of loaded BaseData objects (MAPRegistrationWrappers).
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    MAPRegistrationWrapperIO(const MAPRegistrationWrapperIO& other);
    MAPRegistrationWrapperIO* IOClone() const override;
  };


} // end of namespace mitk

#endif
