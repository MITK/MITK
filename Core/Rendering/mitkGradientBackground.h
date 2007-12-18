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

#ifndef _vtk_Gradient_Background_h_
#define _vtk_Gradient_Background_h_


#include <mitkBaseData.h>

class vtkRenderer;
class vtkMapper;
class vtkActor;
class vtkPolyDataMapper;
class vtkLookupTable;
class vtkPolyData;
class vtkRenderWindow;

namespace mitk {

class RenderWindow;  
/**
 * Displays a color gradient in the background
 * of a vtkRenderWindow.
 * The gradient ist faked by displaying a non-interactable
 * smoothly shaded plane in a separate layer behind the
 * scene. After setting the renderwindow, the gradient may be 
 * activated by calling Enable()
 */
class GradientBackground : public BaseData
{
public:

  mitkClassMacro( GradientBackground, BaseData );
  
  itkNewMacro( Self );

  /**
   * Sets the renderwindow, in which the gradient background
   * will be shown. Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( vtkRenderWindow* renderWindow );

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the gradient background
   */
  virtual vtkRenderWindow* GetRenderWindow();

  /**
   * Returns the renderer responsible for
   * rendering the color gradient into the
   * vtkRenderWindow
   */
  virtual vtkRenderer* GetVtkRenderer();

  /**
   * Returns the actor associated with the color gradient
   */
  virtual vtkActor* GetActor();

  /**
   * Returns the mapper associated with the color
   * gradient.
   */
  virtual vtkPolyDataMapper* GetMapper();

  /**
   * Sets the gradient colors. The gradient
   * will smoothly fade from color1 to color2
   */
  virtual void SetGradientColors( double r1, double g1, double b1, double r2, double g2, double b2);
  virtual void SetUpperColor(double r, double g, double b );
  virtual void SetLowerColor(double r, double g, double b );

  /**
   * Enables drawing of the color gradient background.
   * If you want to disable it, call the Disable() function.
   */
  virtual void Enable();

  /**
   * Disables drawing of the color gradient background.
   * If you want to enable it, call the Enable() function.
   */
  virtual void Disable();
  
  /**
   * Checks, if the gradient background is currently
   * enabled (visible)
   */
  virtual bool IsEnabled();
  
  /**
   * Empty implementation, since the GradientBackground doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  /**
   * Empty implementation, since the GradientBackground doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  /**
   * Empty implementation, since the GradientBackground doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion();
  
  /**
   * Empty implementation, since the GradientBackground doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion(itk::DataObject*);
  
protected:

  /**
   * Constructor
   */
  GradientBackground();

  /**
   * Destructor
   */
  ~GradientBackground();

  vtkRenderWindow* m_RenderWindow;

  vtkRenderer* m_Renderer;

  vtkActor* m_Actor;

  vtkPolyDataMapper* m_Mapper;

  vtkLookupTable* m_Lut;

  vtkPolyData* m_Plane;
};

} //end of namespace mitk
#endif

