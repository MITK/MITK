/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkWriter_h
#define mitkSurfaceVtkWriter_h

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
   * \brief VTK-based writer for mitk::Surface objects.
   *
   * Writes mitk::Surface data using the VTK writer type specified as the template
   * argument (e.g., vtkSTLWriter, vtkPolyDataWriter, vtkXMLPolyDataWriter).
   *
   * If the mitk::Surface contains multiple time steps, a separate file is written
   * for each time step. The time bounds are encoded in the filename using the scheme:
   * \c \<filename\>_S\<start\>E\<end\>_T\<framenumber\> (S=start, E=end, T=time).
   *
   * \note Writing of multiple files according to a given filename pattern is not
   *       yet supported.
   *
   * \tparam VTKWRITER The VTK writer class to use for output (e.g., vtkSTLWriter,
   *         vtkPolyDataWriter, vtkXMLPolyDataWriter).
   *
   * \ingroup MitkLegacyIOModule
   * \sa mitk::SurfaceVtkWriterFactory, mitk::Surface, mitk::FileWriterWithInformation
   */
  template <class VTKWRITER>
  class MITKLEGACYIO_EXPORT SurfaceVtkWriter : public mitk::FileWriterWithInformation
  {
  public:
    mitkClassMacro(SurfaceVtkWriter, mitk::FileWriter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkWriterMacro;

    /** \brief The VTK writer type used for output. */
    typedef VTKWRITER VtkWriterType;

    /** \brief Set the output file name. */
    mitkOverrideSetStringMacro(FileName);

    /**
     * \brief Get the filename of the file to be written.
     * \return The output file name.
     */
    mitkOverrideGetStringMacro(FileName);

    /**
     * \brief Explicitly set the extension to be added to the filename.
     * \param[in] _arg The extension including a "." (e.g., ".vtk").
     *
     * Partial template specialization is used for some VTK writer types
     * to set a default extension.
     */
    itkSetStringMacro(Extension);

    /**
     * \brief Get the extension to be added to the filename.
     * \return The file extension (e.g., ".vtk").
     */
    itkGetStringMacro(Extension);

    /**
     * \brief Reset the extension to the default for the current VTK writer type.
     *
     * Partial template specialization is used for some VTK writer types
     * to define the default extension (e.g., ".stl" for vtkSTLWriter,
     * ".vtp" for vtkXMLPolyDataWriter).
     */
    void SetDefaultExtension();

    /**
     * \brief Set the file prefix for multi-file writing.
     * \warning Multiple file writing is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePrefix);

    /**
     * \brief Get the file prefix.
     * \warning Multiple file writing is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePrefix);

    /**
     * \brief Set the file pattern for multi-file writing.
     * \warning Multiple file writing is not yet supported.
     */
    mitkOverrideSetStringMacro(FilePattern);

    /**
     * \brief Get the file pattern.
     * \warning Multiple file writing is not yet supported.
     */
    mitkOverrideGetStringMacro(FilePattern);

    /**
     * \brief Set the 0th input surface for the writer.
     * \param[in] input The mitk::Surface to write.
     */
    void SetInput(mitk::Surface *input);

    /**
     * \brief Get the 0th input surface.
     * \return Pointer to the input mitk::Surface, or \c nullptr if not set.
     */
    const mitk::Surface *GetInput();

    /**
     * \brief Return the currently configured file extension.
     * \return The file extension string.
     */
    std::string GetFileExtension() override;

    /**
     * \brief Check whether the writer can write data from the given DataNode.
     * \param[in] node The DataNode to check. The node's data must be a mitk::Surface.
     * \return \c true if the node contains a mitk::Surface; \c false otherwise.
     */
    bool CanWriteDataType(DataNode * node) override;

    /**
     * \brief Return the MIME type of the file to be written.
     * \return The MIME type string.
     */
    std::string GetWritenMIMEType() override;

    using Superclass::SetInput;

    /**
     * \brief Set a DataNode as input. Extracts the mitk::Surface from the node.
     * \param[in] node The DataNode containing the surface to write.
     */
    virtual void SetInput(DataNode * node);

    /**
     * \brief Get the internal VTK writer instance.
     * \return Pointer to the VTK writer used for output.
     */
    VtkWriterType *GetVtkWriter() { return m_VtkWriter; }

    /**
     * \brief Return the list of possible file extensions for surface data.
     * \return A vector of supported file extension strings.
     */
    std::vector<std::string> GetPossibleFileExtensions() override;

    /**
     * \brief Return the class name of the supported base data type.
     * \return The static class name of mitk::Surface.
     */
    std::string GetSupportedBaseData() const override { return Surface::GetStaticNameOfClass(); }

    /**
     * \brief Return the default filename for file save dialogs.
     * \return A default filename string.
     */
    const char *GetDefaultFilename() override;

    /**
     * \brief Return the file dialog filter pattern.
     * \return A filter pattern string for file dialogs.
     */
    const char *GetFileDialogPattern() override;

    /**
     * \brief Return the default file extension.
     * \return The default extension string.
     */
    const char *GetDefaultExtension() override;

    /**
     * \brief Check whether this writer can write the given BaseData type.
     * \param[in] data The data object to check.
     * \return \c true if the data is a mitk::Surface; \c false otherwise.
     */
    bool CanWriteBaseDataType(BaseData::Pointer data) override;

    /**
     * \brief Write the given BaseData object to file.
     * \param[in] data The data to write (must be a mitk::Surface).
     */
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

#endif
