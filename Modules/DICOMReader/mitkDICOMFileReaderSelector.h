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
  \ingroup DICOMReaderModule
  \brief Simple best-reader selection.

  This class implements a process of comparing different DICOMFileReader%s and selecting
  the reader with the minimal number of mitk::Image%s in its output.

  The code found in this class can
   - just be used to select a reader using this simple strategy
   - be taken as an example of how to use DICOMFileReader%s

  To create a selection of potential readers, the class makes use
  of mitk::DICOMReaderConfigurator, i.e. DICOMFileReaderSelector
  also expects the configuration files/strings to be in the format
  expected by mitk::DICOMReaderConfigurator.

  Two convenience methods load "default" configurations from
  compiled-in resources: LoadBuiltIn3DConfigs() and LoadBuiltIn3DnTConfigs().
*/
class DICOMReader_EXPORT DICOMFileReaderSelector : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMFileReaderSelector, itk::LightObject )
    itkNewMacro( DICOMFileReaderSelector )

    /// \brief Add a configuration as expected by DICOMReaderConfigurator.
    /// Configs can only be reset by instantiating a new DICOMFileReaderSelector.
    void AddConfig(const std::string& xmlDescription);
    /// \brief Add a configuration as expected by DICOMReaderConfigurator.
    /// Configs can only be reset by instantiating a new DICOMFileReaderSelector.
    void AddConfigFile(const std::string& filename);

    /// \brief Load 3D image creating configurations from the MITK module system (see \ref mitk::Module::FindResources).
    /// For a default set of configurations, look into the directory Resources of the DICOMReader module.
    void LoadBuiltIn3DConfigs();
    /// \brief Load 3D+t image creating configurations from the MITK module system (see \ref mitk::Module::FindResources).
    /// For a default set of configurations, look into the directory Resources of the DICOMReader module.
    void LoadBuiltIn3DnTConfigs();

    /// Input files
    void SetInputFiles(StringList filenames);
    /// Input files
    const StringList& GetInputFiles() const;

    /// Execute the analysis and selection process. The first reader with a minimal number of outputs will be returned.
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
