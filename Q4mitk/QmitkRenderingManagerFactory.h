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


#ifndef QMITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197
#define QMITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197

#include "mitkRenderingManagerFactory.h"

/**
 * \brief Qt specific implementation of mitk::RenderingManagerFactory.
 *
 * This class create QmitkRenderingManager instances via
 * #CreateRenderingManager.
 *
 * A static instance of QmitkRenderingManagerFactory is created in
 * QmitkRenderWindow, forcing the usage of QmitkRenderingManager for the Qt
 * platform.
 * \ingroup Renderer
 */
class QmitkRenderingManagerFactory : public mitk::RenderingManagerFactory
{
public:

  QmitkRenderingManagerFactory();

  ~QmitkRenderingManagerFactory();

  virtual mitk::RenderingManager::Pointer CreateRenderingManager() const;

private:

};


#endif
