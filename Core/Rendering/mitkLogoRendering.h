#ifndef _vtk_Logo_Rendering_h_
#define _vtk_Logo_Rendering_h_


#include <vtkSystemIncludes.h>
#include <mitkBaseData.h>

class vtkRenderer;
class vtkMapper;
class vtkCamera;
class vtkImageActor;
class vtkImageMapper;
class vtkLookupTable;
class vtkPolyData;
class vtkPNGReader;

namespace mitk {

class RenderWindow;  
/**
 * Renders a company logo in the foreground
 * of a vtkRenderWindow.
 
 */
class LogoRendering : public BaseData
{
public:

  mitkClassMacro( LogoRendering, BaseData );
  
  itkNewMacro( Self );

  enum LogoPosition{ UpperLeft, UpperRight, LowerLeft, LowerRight, Middle };

  
  
  /**
   * Sets the renderwindow, in which the logo
   * will be shown. Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( mitk::RenderWindow* renderWindow );

  /**
   * Sets the source file for the logo. 
   */
  virtual void SetLogoSource(const char* filename);
  /**
   * Sets the opacity level of the logo.
   */
  virtual void SetOpacity(double opacity);
  /**
   * Specifies the logo size, values from 0...10,
   * where 1 is a nice little logo
   */
  virtual void SetZoomFactor( double factor );
  
  /**
   * Enables drawing of the logo.
   * If you want to disable it, call the Disable() function.
   */
  virtual void Enable();

  /**
   * Disables drawing of the logo.
   * If you want to enable it, call the Enable() function.
   */
  virtual void Disable();
  
  /**
   * Checks, if the logo is currently
   * enabled (visible)
   */
  virtual bool IsEnabled();
   
  /**
   * Empty implementation, since the LogoRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  /**
   * Empty implementation, since the LogoRendering doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  /**
   * Empty implementation, since the LogoRendering doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion();
  
  /**
   * Empty implementation, since the LogoRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion(itk::DataObject*);

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the logo
   */
  virtual mitk::RenderWindow* GetRenderWindow();

  /**
   * Returns the renderer responsible for
   * rendering the logo into the
   * vtkRenderWindow
   */
  virtual vtkRenderer* GetVtkRenderer();

  /**
   * Returns the actor associated with the logo
   */
  virtual vtkImageActor* GetActor();

  /**
   * Returns the mapper associated with the logo
   */
  virtual vtkImageMapper* GetMapper();
  
protected:
  void SetupCamera();
  void SetupPosition();

  /**
   * Constructor
   */
  LogoRendering();

  /**
   * Destructor
   */
  ~LogoRendering();

  mitk::RenderWindow* m_RenderWindow;
  vtkRenderer*        m_Renderer;
  vtkImageActor*      m_Actor;
  vtkImageMapper*     m_Mapper;
  vtkPNGReader*       m_PngReader;
  vtkCamera*          m_Camera;

  std::string         m_FileName;

  bool                m_IsEnabled;

  LogoPosition        m_LogoPosition;
  double              m_ZoomFactor;
  double              m_Opacity;

};

} //end of namespace mitk
#endif
