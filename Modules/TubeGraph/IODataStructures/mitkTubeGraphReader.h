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

#ifndef __mitkTubeGraphReader_h
#define __mitkTubeGraphReader_h

#include "mitkCommon.h"
#include "mitkFileReader.h"

#include "mitkTubeGraph.h"

namespace mitk
{
  /** \brief The reader for tube graph structure files .tsf
  */

  class TubeGraphReader : public FileReader, public BaseDataSource
  {
  public:

    typedef TubeGraph OutputType;

    mitkClassMacro( TubeGraphReader, BaseDataSource );
    itkNewMacro(Self);

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

#endif // __mitkTubeGraphReader_h
