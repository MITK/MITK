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

//##ModelId=3E3AA2F60390
//##Documentation
//## @brief Base class of all mappers for 3D display
//## @ingroup Mapper
class Mapper3D : public Mapper
{
  public:
    mitkClassMacro(Mapper3D,Mapper);

  protected:
    //##ModelId=3E3AE1C5010C
    Mapper3D();

    //##ModelId=3E3AE1C5012A
    virtual ~Mapper3D();

};

} // namespace mitk



#endif /* MAPPER3D_H_HEADER_INCLUDED_C1C517B9 */
