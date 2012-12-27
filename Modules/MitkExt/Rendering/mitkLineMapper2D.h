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


#ifndef MITKLINEMAPPER2D_H_HEADER_INCLUDED
#define MITKLINEMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkPointSetGLMapper2D.h"

namespace mitk {

class BaseRenderer;
class PointSet;

//##Documentation
//## @brief OpenGL-based mapper to display Lines
//##
//## uses the information from the PointSet to build up the lines.
//## Only lines in 2D in one Slice are drawn, not between slices!
//## @ingroup Mapper
class MitkExt_EXPORT LineMapper2D : public PointSetGLMapper2D
{
public:
  mitkClassMacro(LineMapper2D, PointSetGLMapper2D);
  itkNewMacro(Self);

  //##Documentation
  //## @brief Get the PointDataList to map
  const PointSet * GetInput(void);

  virtual void Paint(mitk::BaseRenderer * renderer);

protected:
  LineMapper2D();

  virtual ~LineMapper2D();
};

} // namespace mitk

#endif /* MITKLINEMAPPER2D_H_HEADER_INCLUDED */
