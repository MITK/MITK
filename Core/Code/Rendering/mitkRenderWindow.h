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


#ifndef MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF
#define MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF

#include <MitkExports.h>

#include "mitkRenderWindowBase.h"

namespace mitk
{

class vtkEventProvider;

/**
 * \brief mitkRenderWindow integrates the MITK rendering mechanism into VTK and
 * is NOT QT dependent
 *
 *
 * \ingroup Renderer
 */
class MITK_CORE_EXPORT RenderWindow: public mitk::RenderWindowBase, public itk::Object
{


public:
  mitkClassMacro(RenderWindow, itk::Object);
  itkNewMacro(Self);
  mitkNewMacro1Param(Self, vtkRenderWindow*);
  mitkNewMacro2Param(Self, vtkRenderWindow*, const char*);
  mitkNewMacro3Param(Self, vtkRenderWindow*, const char*, mitk::RenderingManager*);


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

  /**
  * \brief Convenience method to set the size of an mitkRenderWindow.
  *
  * This method sets the size of the vtkRenderWindow and tells the
  * rendering that the size has changed -> adapts displayGeometry, etc.
  */
  void SetSize( int width, int height );

  /**
  * \brief Initializes the mitkVtkEventProvider to listen to the
  * currently used vtkInteractorStyle.
  *
  * This method makes sure that the internal mitkVtkEventProvider
  * listens to the correct vtkInteractorStyle.
  * This makes sure that VTK-Events are correctly translated into
  * MITK-Events.
  *
  * \warn This method needs to be called MANUALLY as soon as the MapperID
  * for this RenderWindow is changed or the vtkInteractorStyle is modified
  * somehow else!
  */
  void ReinitEventProvider();


protected:
    RenderWindow(vtkRenderWindow * existingRenderWindow = NULL , const char* name = "unnamed renderer", mitk::RenderingManager* rm = NULL );


    void ResetView();
    vtkRenderWindow *               m_vtkRenderWindow;
    vtkRenderWindowInteractor *     m_vtkRenderWindowInteractor;

    vtkEventProvider *              m_vtkMitkEventProvider;


private:


};

} //namespace

#endif /* MITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66ASDF */
