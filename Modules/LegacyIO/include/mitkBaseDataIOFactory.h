/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseDataIOFactory_h
#define mitkBaseDataIOFactory_h

#include <MitkLegacyIOExports.h>

#include <mitkBaseData.h>

#include <itkObject.h>

namespace mitk
{
  /**
   * \brief Factory class for loading BaseData objects from files.
   *
   * Provides a static method to load mitk::BaseData objects from a given file path
   * using the registered IO factories. Internally delegates to mitk::IOUtil::Load().
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::IOUtil, mitk::FileReaderRegistry
   */
  class MITKLEGACYIO_EXPORT BaseDataIO : public itk::Object
  {
  public:
    /** \brief Standard ITK self type. */
    typedef BaseDataIO Self;
    /** \brief Standard ITK superclass type. */
    typedef itk::Object Superclass;
    /** \brief Smart pointer type. */
    typedef itk::SmartPointer<Self> Pointer;
    /** \brief Const smart pointer type. */
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** \brief Run-time type information (and related methods). */
    itkTypeMacro(BaseDataIO, Object);

    /**
     * \brief Load BaseData objects from a file.
     *
     * Creates the appropriate BaseData objects depending on the file type.
     * Internally delegates to mitk::IOUtil::Load(). The \p filePrefix,
     * \p filePattern, and \p series parameters are currently ignored.
     *
     * \param[in] path The absolute file path to load data from.
     * \param[in] filePrefix The file prefix (currently unused).
     * \param[in] filePattern The file pattern (currently unused).
     * \param[in] series Whether the file is part of a series (currently unused).
     * \return A vector of loaded BaseData objects.
     */
    static std::vector<mitk::BaseData::Pointer> LoadBaseDataFromFile(const std::string path,
                                                                     const std::string filePrefix,
                                                                     const std::string filePattern,
                                                                     bool series);

  protected:
    BaseDataIO();
    ~BaseDataIO() override;

  private:
    BaseDataIO(const Self &);     // purposely not implemented
    void operator=(const Self &); // purposely not implemented
  };

} // end namespace mitk

#endif
