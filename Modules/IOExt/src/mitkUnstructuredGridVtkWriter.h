/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridVtkWriter_h
#define mitkUnstructuredGridVtkWriter_h

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <mitkUnstructuredGrid.h>

namespace mitk
{
  /**
   * \brief VTK-based writer for mitk::UnstructuredGrid.
   *
   * The mitk::UnstructuredGrid is written using the VTK-writer-type provided as the
   * template argument. If the mitk::UnstructuredGrid contains multiple points of
   * time, multiple files are written. The life-span (time-bounds) of each
   * each point of time is included in the filename according to the
   * following scheme:
   * &lt;filename&gt;_S&lt;timebounds[0]&gt;E&lt;timebounds[1]&gt;_T&lt;framenumber&gt;
   * (S=start, E=end, T=time).
   * Writing of multiple files according to a given filename pattern is not
   * yet supported.
   *
   * \tparam VTKWRITER The VTK writer type to use (e.g. vtkUnstructuredGridWriter).
   */
  template <class VTKWRITER>
  class UnstructuredGridVtkWriter : public mitk::FileWriterWithInformation
  {
  public:
    mitkClassMacro(UnstructuredGridVtkWriter, mitk::FileWriterWithInformation);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkWriterMacro;

    /** \brief Set the filename of the file to write. */
    mitkOverrideSetStringMacro(FileName);

    /** \brief Return the name of the file to be written to disk. */
    mitkOverrideGetStringMacro(FileName);

    /** \brief Set the file prefix. \warning Multiple write not (yet) supported. */
    mitkOverrideSetStringMacro(FilePrefix);

    /** \brief Return the file prefix. \warning Multiple write not (yet) supported. */
    mitkOverrideGetStringMacro(FilePrefix);

    /** \brief Set the file pattern. \warning Multiple write not (yet) supported. */
    mitkOverrideSetStringMacro(FilePattern);

    /** \brief Return the file pattern. \warning Multiple write not (yet) supported. */
    mitkOverrideGetStringMacro(FilePattern);

    using FileWriter::SetInput;

    /**
     * \brief Set the input data object to write.
     * \param input The BaseData (expected to be an UnstructuredGrid).
     */
    void SetInput(BaseData *input);

    /** \brief Return the input UnstructuredGrid. */
    const UnstructuredGrid *GetInput();

    /** \brief Return false if an error occurred during writing. */
    itkGetMacro(Success, bool);

    /** \brief Return the possible file extensions for this writer's data type. */
    std::vector<std::string> GetPossibleFileExtensions() override;

    std::string GetSupportedBaseData() const override { return UnstructuredGrid::GetStaticNameOfClass(); }
    // FileWriterWithInformation methods
    const char *GetDefaultFilename() override;
    const char *GetFileDialogPattern() override;
    const char *GetDefaultExtension() override;
    bool CanWriteBaseDataType(BaseData::Pointer data) override;
    void DoWrite(BaseData::Pointer data) override;

  protected:
    /**
     * Constructor.
     */
    UnstructuredGridVtkWriter();

    /**
     * Virtual destructor.
     */
    ~UnstructuredGridVtkWriter() override;

    void ExecuteWrite(VTKWRITER *vtkWriter);

    void GenerateData() override;

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    bool m_Success;
  };
}

#endif
