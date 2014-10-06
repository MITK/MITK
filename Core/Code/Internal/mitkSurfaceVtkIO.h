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


#ifndef _MITK_SURFACE_VTK_IO_H_
#define _MITK_SURFACE_VTK_IO_H_

#include <mitkAbstractFileIO.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk
{

/**
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
 * @ingroup Process
 */
class SurfaceVtkIO : public mitk::AbstractFileIO
{
public:

  SurfaceVtkIO(const std::string& baseDataType, const CustomMimeType& mimeType,
               const std::string& description);

  virtual ConfidenceLevel GetWriterConfidenceLevel() const;

protected:

  vtkSmartPointer<vtkPolyData> GetPolyData(unsigned int t, std::string& fileName);

};

}

#endif //_MITK_SURFACE_VTK_IO_H_
