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


#ifndef BASEVTKMAPPER2D_H_HEADER_INCLUDED
#define BASEVTKMAPPER2D_H_HEADER_INCLUDED

#include "mitkMapper2D.h"

class vtkProp;

namespace mitk {

  //##Documentation
  //## @brief Base class of all vtk-based 2D-Mappers
  //##
  //## Those must implement the abstract
  //## method vtkProp* GetProp().
  //## @ingroup Mapper
  class MITK_CORE_EXPORT VtkMapper2D : public Mapper2D
  {
  public:
    mitkClassMacro(VtkMapper2D,Mapper2D);
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) = 0;

    /**
     * \brief Returns whether this is an vtk-based mapper
     */
    virtual bool IsVtkBased() const { return true; }

  protected:
    VtkMapper2D();

    virtual ~VtkMapper2D();

  };

} // namespace mitk



#endif /* BASEVTKMAPPER2D_H_HEADER_INCLUDED */
