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


#ifndef MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkCommon.h"
#include <vector>
#include <itkObjectFactory.h>

namespace mitk
{

class RenderWindow;
class RenderingManager;
class RenderingManagerFactory;

/**
 * \brief Central manager for rendering requests.
 */
class RenderingManager : public itk::Object
{
public:
  mitkClassMacro( RenderingManager, itk::Object );

  static Pointer New();

  static void SetFactory( RenderingManagerFactory *factory );

  static Pointer GetInstance();

  void AddRenderWindow( RenderWindow *renderWindow );
  void RemoveRenderWindow( RenderWindow *renderWindow );

  void RequestUpdate( RenderWindow *renderWindow );
  void ForceImmediateUpdate( RenderWindow *renderWindow );

  void RequestUpdateAll();
  void ForceImmediateUpdateAll();

  void SetMinimumInterval( int msec );
  int GetMinimumInterval() const;

  //##Documentation
  //## Destructor
  ~RenderingManager();

  virtual void UpdateCallback();

protected:
  //##Documentation
  //## Constructor
  RenderingManager();

  virtual void RestartTimer() = 0;

  virtual void StopTimer() = 0;

  bool m_UpdatePending;
  int m_Interval;

private:
  //##Documentation
  typedef std::map< RenderWindow *, bool > RenderWindowList;

  RenderWindowList m_RenderWindowList;

  static RenderingManager::Pointer m_Instance;
  static RenderingManagerFactory *m_RenderingManagerFactory;

};

} // namespace mitk



#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
