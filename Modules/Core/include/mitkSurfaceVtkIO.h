/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkIO_h
#define mitkSurfaceVtkIO_h

#include <mitkAbstractFileIO.h>
#include <MitkCoreExports.h>

#include <vtkSmartPointer.h>

class vtkPolyData;

namespace mitk
{
  /**
   * \brief Base class for surface readers and writers of file formats that hold
   * the polygonal data of a single time step.
   *
   * A surface with several time steps is written as one file per time step. The
   * file names carry the time bounds of the step in ms and its index:
   * \c &lt;name&gt;_S&lt;start&gt;_E&lt;end&gt;_T&lt;index&gt;.&lt;extension&gt;
   * Output streams only receive the first time step.
   *
   * Reading any file of such a series reads all files of the series in its
   * directory and returns a single surface with all time steps. GetReadFiles()
   * lists these files, so that IOUtil skips the other files of the series when
   * several of them were selected for loading. File names are compared
   * case-insensitively, and a single file whose name follows the pattern is
   * read on its own. Writing a series refuses files of an earlier series of the
   * same name that it would not overwrite, as they would be read along.
   *
   * The geometry of each time step is baked into the written points, so that
   * other applications show the surface at the right place. Read surfaces
   * therefore always have an identity geometry, with their points in world
   * coordinates.
   *
   * Subclasses read and write the polygonal data of a single file.
   */
  class MITKCORE_EXPORT SurfaceVtkIO : public AbstractFileIO
  {
  public:
    /**
     * \brief PartiallySupported for surfaces that have a time, as several time
     * steps are split into several files and a single time step loses its time.
     */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

    void Write() override;

  protected:
    SurfaceVtkIO(const CustomMimeType &mimeType, const std::string &description);

    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    /** \brief Name the node after the series rather than after the file of a single time step. */
    void SetDefaultDataNodeProperties(DataNode *node, const std::string &filePath) override;

    /**
     * \brief Read the polygonal data of a single file.
     * \throw mitk::Exception if the file cannot be read.
     */
    virtual vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &fileName) = 0;

    /**
     * \brief Write the polygonal data of a single time step to a file.
     * \throw mitk::Exception if the file cannot be written.
     */
    virtual void WritePolyData(vtkPolyData *polyData, const std::string &fileName) = 0;

    /** \brief Report how far reading the current file has got, from 0 to 1. */
    void ReportReadProgress(float progress);

    /** \brief Report how far writing the current file has got, from 0 to 1. */
    void ReportWriteProgress(float progress);

  private:
    unsigned int m_CurrentFile = 0;
    unsigned int m_NumberOfFiles = 1;
  };
}

#endif
