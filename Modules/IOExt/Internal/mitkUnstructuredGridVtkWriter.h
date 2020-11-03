/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_
#define _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_

#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>

#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include "mitkUnstructuredGrid.h"

namespace mitk
{
  /**
   * @brief VTK-based writer for mitk::UnstructuredGrid
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
  */
  template <class VTKWRITER>
  class UnstructuredGridVtkWriter : public mitk::FileWriterWithInformation
  {
  public:
    mitkClassMacro(UnstructuredGridVtkWriter, mitk::FileWriterWithInformation);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkWriterMacro;

    /**
     * Sets the filename of the file to write.
     * @param _arg the name of the file to write.
     */
    itkSetStringMacro(FileName);

    /**
     * @returns the name of the file to be written to disk.
     */
    itkGetStringMacro(FileName);

    /**
     * @warning multiple write not (yet) supported
     */
    itkSetStringMacro(FilePrefix);

    /**
     * @warning multiple write not (yet) supported
     */
    itkGetStringMacro(FilePrefix);

    /**
     * @warning multiple write not (yet) supported
     */
    itkSetStringMacro(FilePattern);

    /**
     * @warning multiple write not (yet) supported
     */
    itkGetStringMacro(FilePattern);

    using FileWriter::SetInput;

    /**
     * Sets the 0'th input object for the filter.
     * @param input the first input for the filter.
     */
    void SetInput(BaseData *input);

    /**
     * @returns the 0'th input object of the filter.
     */
    const UnstructuredGrid *GetInput();

    /**
     * Returns false if an error happened during writing
     */
    itkGetMacro(Success, bool);

    /**
    * @brief Return the possible file extensions for the data type associated with the writer
    */
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

#endif // _MITK_UNSTRUCTURED_GRID_VTK_WRITER__H_
