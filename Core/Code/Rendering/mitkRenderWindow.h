/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-06-30 15:12:34 +0200 (Mi, 30. Jun 2010) $
Version:   $Revision: 24176 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF
#define MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF

#include "mitkCommon.h"

#include "mitkRenderWindowBase.h"

namespace mitk
{

class vtkEventProvider;

/**
 * \brief mitkRenderWindow integrates the MITK rendering mechanism into VTK and
 * is NOT QT dependent
 *
 * Note: This class is currently experimental.
 *
 * \ingroup Renderer
 */
class MITK_CORE_EXPORT RenderWindow: public mitk::RenderWindowBase, public itk::Object
{
  

public:
  mitkClassMacro(RenderWindow, itk::Object);
  itkNewMacro(Self);
  
  virtual ~RenderWindow();

  virtual vtkRenderWindow* GetVtkRenderWindow();
  virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor();

  // Set Layout Index to define the Layout Type
  void SetLayoutIndex( unsigned int layoutIndex );
    
  // Get Layout Index to define the Layout Type
  unsigned int GetLayoutIndex();

  //MenuWidget need to update the Layout Design List when Layout had changed
  void LayoutDesignListChanged( int layoutDesignIndex );
  
  void FullScreenMode( bool state );

protected:
    RenderWindow();

    void ResetView();
  
    vtkRenderWindow *               m_vtkRenderWindow;
    vtkRenderWindowInteractor *     m_vtkRenderWindowInteractor;

    vtkEventProvider *              m_vtkMitkEventProvider;
   

private:
  
  
};

} //namespace

#endif /* MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF */
