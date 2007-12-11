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


#ifndef MITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGERFACTORY_H_HEADER_INCLUDED_C135A197

#include "mitkRenderingManager.h"

namespace mitk
{

/**
 * \brief Factory interface for facilitating the usage of a platform-specific
 * mitk::RenderingManager instance.
 *
 * This class provides an interface for creating the required singleton
 * instance of RenderingManager. Concrete platform-specific subclasses should
 * be instantiated once during startup of the framework (e.g. as a static
 * instance). Their constructor then merely has to call
 * mitk::RenderingManager::SetFactory().
 *
 * \note Instead of using an external class for the manager
 * instantiation, the factory mechanism could be integrated into the
 * RenderingManager (and its subclasses) itself. However, this would make
 * the framework specific instantiation more complicated. Simply creating a
 * static instance somewhere would not work since RenderingManager derives from
 * itk::Object, which itself depends on the initialization of static members
 * (which is problematic since the order of static member initializations
 * cannot easily be controlled).
 * \ingroup Renderer
 */
class RenderingManagerFactory
{
public:
  virtual ~RenderingManagerFactory() {};

  /** \brief Factory method to create platform specific instances of
   * RenderingManager. */
  virtual RenderingManager::Pointer CreateRenderingManager() const = 0;

protected:
  RenderingManagerFactory()
  {
  };

private:

};

} // namespace mitk

#endif
