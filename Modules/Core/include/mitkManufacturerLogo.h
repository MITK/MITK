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

#ifndef _vtk_Logo_Rendering_h_
#define _vtk_Logo_Rendering_h_


#include <mitkBaseData.h>

class vtkRenderer;
class vtkRenderWindow;
class vtkMapper;
class vtkCamera;
class vtkImageActor;
class vtkImageMapper;
class vtkLookupTable;
class vtkPolyData;
class vtkPNGReader;
class vtkImageImport;

namespace mitk {

class RenderWindow;
/**
 * Renders a company logo in the foreground
 * of a vtkRenderWindow.
 * @deprecatedSince{next_release} Use mitk::LogoOverlay instead
 */
class MITKCORE_EXPORT ManufacturerLogo : public BaseData
{
public:

  mitkClassMacro( ManufacturerLogo, BaseData );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  enum LogoPosition{ UpperLeft, UpperRight, LowerLeft, LowerRight, Middle };



  /**
   * Sets the renderwindow, in which the logo
   * will be shown. Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( vtkRenderWindow* renderWindow );

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
   * Empty implementation, since the ManufacturerLogo doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion() override;

  /**
   * Empty implementation, since the ManufacturerLogo doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

  /**
   * Empty implementation, since the ManufacturerLogo doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion() override;

  /**
   * Empty implementation, since the ManufacturerLogo doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion( const itk::DataObject*) override;

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the logo
   */
  virtual vtkRenderWindow* GetRenderWindow();

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

  /**
   * If set true, this method forces the logo rendering mechanism that it always
   * renders the MBI department logo, independent from mainapp option settings.
   */
  virtual void ForceMBILogoVisible(bool visible);

protected:
  void SetupCamera();
  void SetupPosition();

  /**
   * Constructor
   */
  ManufacturerLogo();

  /**
   * Destructor
   */
  ~ManufacturerLogo();

  vtkRenderWindow*    m_RenderWindow;
  vtkRenderer*        m_Renderer;
  vtkImageActor*      m_Actor;
  vtkImageMapper*     m_Mapper;
  vtkPNGReader*       m_PngReader;
  vtkCamera*          m_Camera;
  vtkImageImport*     m_VtkImageImport;

  std::string         m_FileName;

  bool                m_IsEnabled;
  bool                m_ForceShowMBIDepartmentLogo;

  LogoPosition        m_LogoPosition;
  double              m_ZoomFactor;
  double              m_Opacity;

  char *              m_ImageData;

};

} //end of namespace mitk
#endif
