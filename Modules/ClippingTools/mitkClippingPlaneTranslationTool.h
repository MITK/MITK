/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2009-04-18 20:20:25 +0200 (Sa, 18 Apr 2009) $
Version:   $Revision: 12952 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkClippingPlaneTranslationTool_h_Included
#define mitkClippingPlaneTranslationTool_h_Included

#include "mitkCommon.h"
#include "mitkVector.h"
#include "mitkTool.h"
#include "mitkDataNode.h"
#include "mitkAffineInteractor3D.h"
#include "ClippingToolsExports.h"



namespace mitk
{

/**
  \brief 
*/
class ClippingTools_EXPORT ClippingPlaneTranslationTool : public Tool
{
public:
  
  mitkClassMacro(ClippingPlaneTranslationTool, Tool);
  itkNewMacro(ClippingPlaneTranslationTool);

  virtual const char** GetXPM() const;
  virtual const char* GetName() const;
  virtual const char* GetGroup() const;


protected:

  ClippingPlaneTranslationTool(); // purposely hidden
  virtual ~ClippingPlaneTranslationTool();

  virtual void Activated();
  virtual void Deactivated();

  mitk::DataNode::Pointer m_ClippingPlaneNode;

  mitk::AffineInteractor3D::Pointer m_AffineInteractor;  

private:

  void ClippingPlaneChanged();
};

} //end namespace mitk
 


#endif


