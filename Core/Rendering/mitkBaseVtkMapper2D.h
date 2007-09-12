/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
  class BaseVtkMapper2D : public Mapper2D
  {
  public:
    virtual vtkProp* GetProp() = 0;

  protected:
    BaseVtkMapper2D();

    virtual ~BaseVtkMapper2D();

  };

} // namespace mitk



#endif /* BASEVTKMAPPER2D_H_HEADER_INCLUDED */
