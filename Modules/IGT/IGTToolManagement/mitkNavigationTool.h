/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-08-11 15:15:02 +0200 (Di, 11 Aug 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef NAVIGATIONTOOL_H_INCLUDED
#define NAVIGATIONTOOL_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief An object of this class represents a navigation tool in the view of the software.
  *        A few informations like an identifier, a toolname, a surface and a itk spatial
  *        object are stored in such an object. The classes NavigationToolReader and
  *        are availiable to write/read tools to/from the harddisc. If you need a collection
  *        of navigation tools the class NavigationToolStorage could be used.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationTool : public itk::Object
  {
  public:
    mitkClassMacro(NavigationTool,itk::Object);
    itkNewMacro(Self);
    
  protected:
    NavigationTool();
    ~NavigationTool();
  };
} // namespace mitk
#endif //NAVIGATIONTOOL