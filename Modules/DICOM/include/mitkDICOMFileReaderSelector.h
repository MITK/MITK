/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFileReaderSelector_h
#define mitkDICOMFileReaderSelector_h

#include "mitkDICOMFileReader.h"

#include <usModuleResource.h>

namespace mitk
{

/**
  \ingroup DICOMModule
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
  @remark If you use LoadBuiltIn3DConfigs() and LoadBuiltIn3DnTConfigs() you must
  ensure that the MitkDICOM module (and therefore its resources) is properly
  loaded. If the module is not available these methods will do nothing.
  This can especially lead to problems if these methods are used in the scope
  of another module's activator.
*/
class MITKDICOM_EXPORT DICOMFileReaderSelector : public itk::LightObject
{
  public:

    typedef std::list<DICOMFileReader::Pointer> ReaderList;

    mitkClassMacroItkParent( DICOMFileReaderSelector, itk::LightObject );
    itkNewMacro( DICOMFileReaderSelector );

    /// \brief Add a configuration as expected by DICOMReaderConfigurator.
    /// Configs can only be reset by instantiating a new DICOMFileReaderSelector.
    void AddConfig(const std::string& xmlDescription);
    /// \brief Add a configuration as expected by DICOMReaderConfigurator.
    /// Configs can only be reset by instantiating a new DICOMFileReaderSelector.
    void AddConfigFile(const std::string& filename);
    /// \brief Add a configuration that is stored in the passed us::ModuleResourse.
    /// Configs can only be reset by instantiating a new DICOMFileReaderSelector.
    void AddConfigFromResource(us::ModuleResource& resource);

    /// \brief Add a whole pre-configured reader to the selection process.
    void AddFileReaderCanditate(DICOMFileReader::Pointer reader);

    /// \brief Load 3D image creating configurations from the MITK module system (see us::Module::FindResources()).
    /// For a default set of configurations, look into the directory Resources of the DICOM module.
    /// @remark If you use this function you must ensure that the MitkDICOM module(and therefore its resources)
    /// is properly loaded. If the module is not available this function will do nothing.
    /// This can especially lead to problem if this function is used in the scope
    /// of another module's activator.
    void LoadBuiltIn3DConfigs();

    /// \brief Load 3D+t image creating configurations from the MITK module system (see us::Module::FindResources()).
    /// For a default set of configurations, look into the directory Resources of the DICOM module.
    /// @remark If you use this function you must ensure that the MitkDICOM module(and therefore its resources)
    /// is properly loaded. If the module is not available this function will do nothing.
    /// This can especially lead to problem if this function is used in the scope
    /// of another module's activator.
    void LoadBuiltIn3DnTConfigs();

    /// \brief Return all the DICOMFileReader%s that are currently used for selection by this class.
    /// The readers returned by this method depend on what config files have been added earlier
    /// (or which of the built-in readers have been loaded)
    ReaderList GetAllConfiguredReaders() const;

    /// Input files
    void SetInputFiles(StringList filenames);
    /// Input files
    const StringList& GetInputFiles() const;

    /// Execute the analysis and selection process. The first reader with a minimal number of outputs will be returned.
    DICOMFileReader::Pointer GetFirstReaderWithMinimumNumberOfOutputImages();

  protected:

    DICOMFileReaderSelector();
    ~DICOMFileReaderSelector() override;

    void AddConfigsFromResources(const std::string& path);
    void AddConfigFromResource(const std::string& resourcename);

  private:

    StringList m_PossibleConfigurations;
    StringList m_InputFilenames;
    ReaderList m_Readers;

 };

} // namespace

#endif
