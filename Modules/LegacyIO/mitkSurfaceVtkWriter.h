/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_SURFACE_VTK_WRITER__H_
#define _MITK_SURFACE_VTK_WRITER__H_

#include <iomanip>

#include <vtkPolyDataWriter.h>
#include <vtkSTLWriter.h>
#include <vtkXMLPolyDataWriter.h>

#include <MitkLegacyIOExports.h>
#include <itkProcessObject.h>
#include <mitkFileWriterWithInformation.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>

#include <vtkSmartPointer.h>

class vtkTransformPolyDataFilter;

namespace mitk
{
  /**
   * @brief VTK-based writer for mitk::Surface
   *
   * The mitk::Surface is written using the VTK-writer-type provided as the
   * template argument. If the mitk::Surface contains multiple points of
   * time, multiple files are written. The life-span (time-bounds) of each
   * each point of time is included in the filename according to the
   * following scheme:
   * &lt;filename&gt;_S&lt;timebounds[0]&gt;E&lt;timebounds[1]&gt;_T&lt;framenumber&gt;
   * (S=start, E=end, T=time).
   * Writing of multiple files according to a given filename pattern is not
   * yet supported.
   * @ingroup MitkLegacyIOModule
   *
   * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
  */
  template <class VTKWRITER>
  class MITKLEGACYIO_EXPORT SurfaceVtkWriter : public mitk::FileWriterWithInformation
  {
  public:
    mitkClassMacro(SurfaceVtkWriter, mitk::FileWriter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkWriterMacro;

    typedef VTKWRITER VtkWriterType;

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
     * \brief Explicitly set the extension to be added to the filename.
     * @param _arg to be added to the filename, including a "."
     * (e.g., ".vtk").
     *
     * Partial template specialization is used for some vtk-writer types
     * to set a default extension.
     */
    itkSetStringMacro(Extension);

    /**
     * \brief Get the extension to be added to the filename.
     * @returns the extension to be added to the filename (e.g.,
     * ".vtk").
     */
    itkGetStringMacro(Extension);

    /**
     * \brief Set the extension to be added to the filename to the default
     *
     * Partial template specialization is used for some vtk-writer types
     * to define the default extension.
     */
    void SetDefaultExtension();

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

    /**
     * Sets the 0'th input object for the filter.
     * @param input the first input for the filter.
     */
    void SetInput(mitk::Surface *input);

    /**
     * @returns the 0'th input object of the filter.
     */
    const mitk::Surface *GetInput();

    /**
    * @brief Return the extension to be added to the filename.
    */
    std::string GetFileExtension() override;

    /**
    * @brief Check if the Writer can write the Content of the DataTreenode.
    */
    bool CanWriteDataType(DataNode *) override;

    /**
    * @brief Return the MimeType of the saved File.
    */
    std::string GetWritenMIMEType() override;

    using Superclass::SetInput;
    /**
    * @brief Set the DataTreenode as Input. Important: The Writer always have a SetInput-Function.
    */
    virtual void SetInput(DataNode *);

    VtkWriterType *GetVtkWriter() { return m_VtkWriter; }
    /**
    * @brief Return the possible file extensions for the data type associated with the writer
    */
    std::vector<std::string> GetPossibleFileExtensions() override;

    std::string GetSupportedBaseData() const override { return Surface::GetStaticNameOfClass(); }
    const char *GetDefaultFilename() override;
    const char *GetFileDialogPattern() override;
    const char *GetDefaultExtension() override;
    bool CanWriteBaseDataType(BaseData::Pointer data) override;
    void DoWrite(BaseData::Pointer data) override;

  protected:
    /**
     * Constructor.
     */
    SurfaceVtkWriter();

    /**
     * Virtual destructor.
     */
    ~SurfaceVtkWriter() override;

    void GenerateData() override;

    void ExecuteWrite(VtkWriterType *vtkWriter);

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::string m_Extension;

    std::string m_MimeType;

    vtkSmartPointer<VtkWriterType> m_VtkWriter;

    bool m_WriterWriteHasReturnValue;
  };

#ifndef MitkLegacyIO_EXPORTS
  extern template class SurfaceVtkWriter<vtkSTLWriter>;
  extern template class SurfaceVtkWriter<vtkPolyDataWriter>;
  extern template class SurfaceVtkWriter<vtkXMLPolyDataWriter>;
#endif
}

#endif //_MITK_SURFACE_VTK_WRITER__H_
