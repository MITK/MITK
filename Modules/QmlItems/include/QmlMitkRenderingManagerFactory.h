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

#ifndef __QmlMitkRenderingManagerFactory_h
#define __QmlMitkRenderingManagerFactory_h

#include "mitkRenderingManagerFactory.h"

/**
 * \ingroup QmlMitkModule
 * \brief Qt specific implementation of mitk::RenderingManagerFactory.
 *
 * This class create QmlMitkRenderingManager instances via
 * CreateRenderingManager().
 *
 */
class  QmlMitkRenderingManagerFactory : public mitk::RenderingManagerFactory
{
public:

  QmlMitkRenderingManagerFactory();
  ~QmlMitkRenderingManagerFactory();

  virtual mitk::RenderingManager::Pointer CreateRenderingManager() const;
};


#endif
