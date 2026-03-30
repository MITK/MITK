/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIOAdapter_h
#define mitkIOAdapter_h

#include <mitkBaseProcess.h>

#include <itkObject.h>

namespace mitk
{
  /**
   * \brief Abstract adapter base class for IO process objects.
   *
   * \ingroup DeprecatedIO
   * \deprecatedSince{2014_10} Use mitk::IFileReader instead.
   *
   * \sa IOAdapter
   * \sa IFileReader
   */
  class IOAdapterBase : public itk::Object
  {
  public:
    /** Standard typedefs. */
    typedef IOAdapterBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /**
     * \brief Create an IO process object and return a pointer to it.
     *
     * \param filename The file name for the IO process object.
     * \param filePrefix The file prefix for the IO process object.
     * \param filePattern The file pattern for the IO process object.
     * \return A smart pointer to the created BaseDataSource.
     */
    virtual itk::SmartPointer<BaseDataSource> CreateIOProcessObject(const std::string filename,
                                                                    const std::string filePrefix,
                                                                    const std::string filePattern) = 0;

    /**
     * \brief Check whether this adapter can read the specified file.
     *
     * \param filename The file name to check.
     * \param filePrefix The file prefix to check.
     * \param filePattern The file pattern to check.
     * \return \c true if the file can be read, \c false otherwise.
     */
    virtual bool CanReadFile(const std::string filename,
                             const std::string filePrefix,
                             const std::string filePattern) = 0;

  protected:
    IOAdapterBase() {}
    ~IOAdapterBase() override {}
  private:
    IOAdapterBase(const Self &);  // purposely not implemented
    void operator=(const Self &); // purposely not implemented
  };

  /**
   * \brief Template adapter class for instantiation of IO process objects.
   *
   * Additionally this interface defines the function CanReadFile().
   * This interface allows the target (object) access to the adaptee (IO process object).
   *
   * \tparam T The IO process object type to adapt.
   *
   * \ingroup IO
   * \deprecatedSince{2014_10} Use mitk::IFileReader instead.
   *
   * \sa IOAdapterBase
   * \sa IFileReader
   */
  template <class T>
  class IOAdapter : public IOAdapterBase
  {
  public:
    /** Standard class typedefs. */
    typedef IOAdapter Self;
    typedef itk::SmartPointer<Self> Pointer;

    /** Methods from mitk::BaseProcess. */
    itkFactorylessNewMacro(Self);

    /** \copydoc IOAdapterBase::CreateIOProcessObject */
    mitk::BaseDataSource::Pointer CreateIOProcessObject(const std::string filename,
                                                        const std::string filePrefix,
                                                        const std::string filePattern) override
    {
      typename T::Pointer ioProcessObject = T::New();
      ioProcessObject->SetFileName(filename.c_str());
      ioProcessObject->SetFilePrefix(filePrefix.c_str());
      ioProcessObject->SetFilePattern(filePattern.c_str());
      return ioProcessObject.GetPointer();
    }

    /** \copydoc IOAdapterBase::CanReadFile */
    bool CanReadFile(const std::string filename,
                             const std::string filePrefix,
                             const std::string filePattern) override
    {
      return T::CanReadFile(filename, filePrefix, filePattern);
    }

  protected:
    IOAdapter() {}
    ~IOAdapter() override {}
  private:
    IOAdapter(const Self &);      // purposely not implemented
    void operator=(const Self &); // purposely not implemented
  };

} // end namespace mitk

#endif
