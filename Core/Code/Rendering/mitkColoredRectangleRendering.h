/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-07-13 17:30:13 +0200 (Fr, 13 Jul 2007) $
Version:   $Revision: 11283 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _vtk_Colored_Rectangle_Rendering_h_
#define _vtk_Colored_Rectangle_Rendering_h_


#include <vtkSystemIncludes.h>
#include <mitkBaseData.h>

#include <vector>
#include <map>

class vtkRenderer;
class vtkRenderWindow;
class vtkCamera;
class vtkTextProperty;
class vtkTextActor;


namespace mitk {

/**
 * This is a simple class for rendering colored rectangles
 * at the boarders of vtkRenderWindows.
 * The rectangle rendering itself is performed by means of a
 * vtkProp (vtkMitkRectangleProp).
 * This class instantiates the vtkProp and a corresponding vtkRenderer instance. 
 */
class MITK_CORE_EXPORT ColoredRectangleRendering : public BaseData
{
public:

  mitkClassMacro( ColoredRectangleRendering, BaseData );
  itkNewMacro( Self );

  /**
   * Sets the renderwindow, in which colored rectangle boarders will be shown. 
   * Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( vtkRenderWindow* renderWindow );
  
  /**
   * Enables drawing of the colored rectangle.
   * If you want to disable it, call the Disable() function.
   */
  virtual void Enable(float col1, float col2, float col3);

  /**
   * Disables drawing of the colored rectangle.
   * If you want to enable it, call the Enable() function.
   */
  virtual void Disable();
  
  /**
   * Checks, if the text is currently
   * enabled (visible)
   */
  virtual bool IsEnabled();
   
  /**
   * Empty implementation, since the rectangle rendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  /**
   * Empty implementation, since the rectangle rendering doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  /**
   * Empty implementation, since the rectangle rendering doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion();
  
  /**
   * Empty implementation, since the rectangle rendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion(itk::DataObject*);

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the text
   */
  virtual vtkRenderWindow* GetRenderWindow();

  /**
   * Returns the renderer responsible for
   * rendering the text into the
   * vtkRenderWindow
   */
  virtual vtkRenderer* GetVtkRenderer();

   
protected:
  
  /**
   * Constructor
   */
  ColoredRectangleRendering();

  /**
   * Destructor
   */
  ~ColoredRectangleRendering();
  
  vtkRenderWindow*    m_RenderWindow;
  vtkRenderer*        m_RectangleRenderer;
  bool                m_IsEnabled;
 
};

} //end of namespace mitk
#endif
