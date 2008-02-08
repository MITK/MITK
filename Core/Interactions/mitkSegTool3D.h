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

#ifndef mitkSegTool3D_h_Included
#define mitkSegTool3D_h_Included

#include "mitkCommon.h"
#include "mitkTool.h"

namespace mitk
{

/**
  \brief Abstract base class for segmentation tools.

  \sa Tool
  \sa SegTool2D

  So far useless, but this would be the place to implement segmentation specific helper methods, that might be of use to
  all kind of 3D and 2D segmentation tools.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITK_CORE_EXPORT SegTool3D : public Tool
{
  public:
    
    mitkClassMacro(SegTool3D, Tool);

  protected:

    SegTool3D(); // purposely hidden
    SegTool3D(const char*); // purposely hidden
    virtual ~SegTool3D();
};

} // namespace

#endif


