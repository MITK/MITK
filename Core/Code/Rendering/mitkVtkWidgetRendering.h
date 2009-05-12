/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _vtk_Widget_Rendering_h_
#define _vtk_Widget_Rendering_h_


#include <mitkBaseData.h>

class vtkRenderer;
class vtkRenderWindow;
class vtkInteractorObserver;

namespace mitk {

class RenderWindow;  

/**
 * \brief Mechanism for rendering a vtkWidget in the foreground of a RenderWindow.
 *
 * To use this class, specify the vtkRenderWindow of the window into which the
 * vtkWidget shall be placed, and set the vtkWidget using SetVtkWidget().
 * After enabling the vtkWidget and calling Enable() of this class, the widget
 * should be rendered.
 *
 * Note: this class only provides a basic mechanism for adding widget; all widget
 * configuration such as placement, size, and en-/disabling of interaction
 * mechanisms need to be done in the vtkWidget object.
 */
class MITK_CORE_EXPORT VtkWidgetRendering : public BaseData
{
public:

  mitkClassMacro( VtkWidgetRendering, BaseData );
  
  itkNewMacro( Self );

 
  
  /**
   * Sets the renderwindow, in which the widget
   * will be shown. Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( vtkRenderWindow* renderWindow );

  /**
   * Enables drawing of the widget.
   * If you want to disable it, call the Disable() function.
   */
  virtual void Enable();

  /**
   * Disables drawing of the widget.
   * If you want to enable it, call the Enable() function.
   */
  virtual void Disable();
  
  /**
   * Checks, if the widget is currently
   * enabled (visible)
   */
  virtual bool IsEnabled();
   
  /**
   * Empty implementation, since the VtkWidgetRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  /**
   * Empty implementation, since the VtkWidgetRendering doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  /**
   * Empty implementation, since the VtkWidgetRendering doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion();
  
  /**
   * Empty implementation, since the VtkWidgetRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion(itk::DataObject*);

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the widget
   */
  virtual vtkRenderWindow* GetRenderWindow();

  /**
   * Returns the renderer responsible for
   * rendering the widget into the
   * vtkRenderWindow
   */
  virtual vtkRenderer* GetVtkRenderer();


  /** Set the vtkWidget to be rendered */
  void SetVtkWidget( vtkInteractorObserver *widget );

  /** Get the vtkWidget to be rendered */
  vtkInteractorObserver *GetVtkWidget() const;

protected:
  /**
   * Constructor
   */
  VtkWidgetRendering();

  /**
   * Destructor
   */
  ~VtkWidgetRendering();

  vtkRenderWindow*    m_RenderWindow;
  vtkRenderer*        m_Renderer;

  vtkInteractorObserver *m_VtkWidget;

  bool                m_IsEnabled;
};

} //end of namespace mitk
#endif
