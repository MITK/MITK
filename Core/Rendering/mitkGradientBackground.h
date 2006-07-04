#ifndef _vtk_Gradient_Background_h_
#define _vtk_Gradient_Background_h_


#include <vtkSystemIncludes.h>
#include <mitkBaseData.h>

class vtkRenderWindow;
class vtkRenderer;
class vtkMapper;
class vtkActor;
class vtkPolyDataMapper;
class vtkLookupTable;
class vtkPolyData;

namespace mitk
{

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
  virtual void SetVtkRenderWindow( vtkRenderWindow* renderWindow );

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the gradient background
   */
  virtual vtkRenderWindow* GetVtkRenderWindow();

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
  virtual void SetGradientColors( vtkFloatingPointType r1, vtkFloatingPointType g1, vtkFloatingPointType b1, vtkFloatingPointType r2, vtkFloatingPointType g2, vtkFloatingPointType b2);
  virtual void SetUpperColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b );
  virtual void SetLowerColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b );

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

