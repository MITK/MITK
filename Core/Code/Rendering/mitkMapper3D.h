/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MAPPER3D_H_HEADER_INCLUDED_C1C517B9
#define MAPPER3D_H_HEADER_INCLUDED_C1C517B9

#include "mitkMapper.h"

namespace mitk {

//##Documentation
//## @brief Base class of all mappers for 3D display
//## @ingroup Mapper
class MITK_CORE_EXPORT Mapper3D : public Mapper
{
  public:
    mitkClassMacro(Mapper3D,Mapper);

  protected:
    Mapper3D();

    virtual ~Mapper3D();

};

} // namespace mitk



#endif /* MAPPER3D_H_HEADER_INCLUDED_C1C517B9 */
