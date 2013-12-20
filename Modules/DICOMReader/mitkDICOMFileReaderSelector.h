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

#ifndef mitkDICOMFileReaderSelector_h
#define mitkDICOMFileReaderSelector_h

#include "mitkDICOMFileReader.h"

namespace mitk
{

/**
  \brief Simple best-reader selection.

  \ingroup DICOMReaderModule
*/
class DICOMReader_EXPORT DICOMFileReaderSelector : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMFileReaderSelector, itk::LightObject )
    itkNewMacro( DICOMFileReaderSelector )

    void AddConfig(const std::string& xmlDescription);
    void AddConfigFile(const std::string& filename);

    void LoadBuiltIn3DConfigs();
    void LoadBuiltIn3DnTConfigs();

    void SetInputFiles(StringList filenames);
    const StringList& GetInputFiles() const;

    DICOMFileReader::Pointer GetFirstReaderWithMinimumNumberOfOutputImages();

  protected:

    DICOMFileReaderSelector();
    virtual ~DICOMFileReaderSelector();

    void AddConfigsFromResources(const std::string& path);

  private:

    StringList m_PossibleConfigurations;
    StringList m_InputFilenames;
    typedef std::list<DICOMFileReader::Pointer> ReaderList;
    ReaderList m_Readers;

 };

} // namespace

#endif // mitkDICOMFileReaderSelector_h
