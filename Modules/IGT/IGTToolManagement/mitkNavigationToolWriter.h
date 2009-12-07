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

#ifndef NAVIGATIONTOOLWRITER_H_INCLUDED
#define NAVIGATIONTOOLWRITER_H_INCLUDED

#include <MitkIGTExports.h>

namespace mitk 
{
  /**Documentation
  * \brief This class offers methods to write objects of the class navigation tool permanently
  *        to the harddisk. The objects are saved in a special fileformat which can be read
  *        by the class NavigationToolReader to restore the object.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolWriter
  {

  };
} // namespace mitk
#endif //NAVIGATIONTOOLWRITER