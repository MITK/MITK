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


#ifndef MITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197

namespace mitk
{

class RenderingManager;

/**
 * \brief Central manager for rendering requests.
 */
class RenderingManagerFactory
{
public:
  virtual ~RenderingManagerFactory() {};

  virtual RenderingManager *CreateRenderingManager() = 0;

protected:
  //##Documentation
  //## Constructor
  RenderingManagerFactory() {};

private:

};

} // namespace mitk



#endif
