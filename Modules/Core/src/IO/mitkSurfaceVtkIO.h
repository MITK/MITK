/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_SURFACE_VTK_IO_H_
#define _MITK_SURFACE_VTK_IO_H_

#include <mitkAbstractFileIO.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * @internal
   * @brief VTK-based reader and writer for mitk::Surface
   *
   * If the mitk::Surface contains multiple points of
   * time, multiple files are written. The life-span (time-bounds) of each
   * each point of time is included in the filename according to the
   * following scheme:
   * &lt;filename&gt;_S&lt;timebounds[0]&gt;E&lt;timebounds[1]&gt;_T&lt;framenumber&gt;
   * (S=start, E=end, T=time).
   * Writing of multiple files according to a given filename pattern is not
   * yet supported.
   */
  class SurfaceVtkIO : public mitk::AbstractFileIO
  {
  public:
    SurfaceVtkIO(const std::string &baseDataType, const CustomMimeType &mimeType, const std::string &description);

    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    vtkSmartPointer<vtkPolyData> GetPolyData(unsigned int t, std::string &fileName);
  };
}

#endif //_MITK_SURFACE_VTK_IO_H_
