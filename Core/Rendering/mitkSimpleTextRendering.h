/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-13 17:30:13 +0200 (Fr, 13 Jul 2007) $
Version:   $Revision: 11283 $

Copyright (c) German Cancer Research Center, Division of Medickal and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _vtk_Simple_Text_Rendering_h_
#define _vtk_Simple_Text_Rendering_h_


#include <vtkSystemIncludes.h>
#include <mitkBaseData.h>

#include <vector>
#include <map>

class vtkRenderer;
class vtkCamera;
class vtkTextProperty;
class vtkTextActor;


namespace mitk {

class RenderWindow;  
/**
 * Renders a company text in the foreground
 * of a vtkRenderWindow.
 
 */
class SimpleTextRendering : public BaseData
{
public:

  mitkClassMacro( SimpleTextRendering, BaseData );
  
  itkNewMacro( Self );

  int  AddTextLabel(int posX, int posY, std::string text);
  void RemoveTextLabel(int text_id);
  vtkTextProperty* GetVtkTextProperty(int text_id);

  void ClearTextLabelCollection();

  int GetNumberOfTextLabels();
  
  /**
   * Sets the renderwindow, in which the text
   * will be shown. Make sure, you have called this function
   * before calling Enable()
   */
  virtual void SetRenderWindow( mitk::RenderWindow* renderWindow );

  /**
   * Enables drawing of the text.
   * If you want to disable it, call the Disable() function.
   */
  virtual void Enable();

  /**
   * Disables drawing of the text.
   * If you want to enable it, call the Enable() function.
   */
  virtual void Disable();
  
  /**
   * Checks, if the text is currently
   * enabled (visible)
   */
  virtual bool IsEnabled();
   
  /**
   * Empty implementation, since the textRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  /**
   * Empty implementation, since the textRendering doesn't
   * support the requested region concept
   */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  /**
   * Empty implementation, since the textRendering doesn't
   * support the requested region concept
   */
  virtual bool VerifyRequestedRegion();
  
  /**
   * Empty implementation, since the textRendering doesn't
   * support the requested region concept
   */
  virtual void SetRequestedRegion(itk::DataObject*);

  /**
   * Returns the vtkRenderWindow, which is used
   * for displaying the text
   */
  virtual mitk::RenderWindow* GetRenderWindow();

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
  SimpleTextRendering();

  /**
   * Destructor
   */
  ~SimpleTextRendering();

  typedef std::map<unsigned int,vtkTextActor*> TextMapType;
  TextMapType m_TextCollection;
  
  mitk::RenderWindow* m_RenderWindow;
  vtkRenderer*        m_TextRenderer;
  bool                m_IsEnabled;
};

} //end of namespace mitk
#endif
