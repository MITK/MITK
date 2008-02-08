/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"

class vtkPlaneSource;
class vtkTransformPolyDataFilter;
class vtkDataSetToPolyDataFilter;

namespace mitk {

class Surface;
class Image;

//##Documentation
//## @brief Adapter for vtkProbeFilter, making it a 3D+t filter
//## 
//## @ingroup Process
class MITK_CORE_EXPORT ProbeFilter : public SurfaceSource
{
public:
  mitkClassMacro(ProbeFilter, SurfaceSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();
  virtual void GenerateInputRequestedRegion();
  virtual void GenerateData();

  const mitk::Surface *GetInput(void);
  const mitk::Image *GetSource(void);

  virtual void SetInput(const mitk::Surface *input);
  virtual void SetSource(const mitk::Image *source);

protected:
  ProbeFilter();

  virtual ~ProbeFilter();
};

} // namespace mitk

#endif /* MITKPROBEFILTER_H_HEADER_INCLUDED_C10B22CD */
