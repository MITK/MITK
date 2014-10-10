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

#ifndef __mitkFiberBundleXReader_h
#define __mitkFiberBundleXReader_h

#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <mitkFiberBundleX.h>
#include <vtkSmartPointer.h>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

  /** \brief
  */

  class MitkFiberTracking_EXPORT FiberBundleXReader : public FileReader, public BaseDataSource
  {
  public:

    /** Types for the standardized TractContainer **/
    /* direct linked includes of mitkFiberBundleX DataStructure */

    typedef mitk::FiberBundleX OutputType;

    mitkClassMacro( FiberBundleXReader, BaseDataSource )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

    virtual void Update();

    BaseDataSource::DataObjectPointer MakeOutput(const DataObjectIdentifierType &name);
    BaseDataSource::DataObjectPointer MakeOutput( DataObjectPointerArraySizeType idx);

  protected:

    /** Does the real work. */
    virtual void GenerateData();
    virtual void GenerateOutputInformation();

    OutputType::Pointer m_OutputCache;

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#endif // __mitkFiberBundleXReader_h
