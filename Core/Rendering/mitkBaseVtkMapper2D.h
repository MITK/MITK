/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
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

#ifndef BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5
#define BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include <vtkProp.h>
#include "mitkBaseRenderer.h"

namespace mitk {

//##ModelId=3E32C60F00E6
//##Documentation
//## @brief Base class of all vtk-based 2D-Mappers
//## @ingroup Mapper
//## Those must implement the abstract
//## method vtkProp* GetProp().
class BaseVtkMapper2D : public Mapper2D
{
  public:
    //##ModelId=3E3AA40002D4
    virtual vtkProp* GetProp() = 0;
    //##ModelId=3EF19F880369
    //##Documentation
    //## @brief Called by BaseRenderer when an update is required
    virtual void Update(mitk::BaseRenderer* renderer) = 0;


  protected:
    //##ModelId=3E3AE0060159
    BaseVtkMapper2D();

    //##ModelId=3E3AE0060177
    virtual ~BaseVtkMapper2D();



};

} // namespace mitk



#endif /* BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5 */
