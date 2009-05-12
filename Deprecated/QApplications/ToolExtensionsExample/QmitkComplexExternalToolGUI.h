/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkComplexExternalToolGUI_h_Included
#define QmitkComplexExternalToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkComplexExternalTool.h"

/**
  \brief GUI for mitk::ComplexExternalTool.

  \sa ComplexExternalTool
  \ingroup Widgets
  \ingroup Reliver
  
  There is a separate page describing the general design of 
  QmitkInteractiveSegmentation: \li \ref QmitkInteractiveSegmentationTechnicalPage

  This is only meant as an example on how to create tools (and their GUIs) 
  as en external extension to MITK's InteractiveSegmentation
  (and similar functionalities). The full description of how this works
  can be found here: \li \ref toolextensions

  Last contributor: $Author$
*/
class QmitkComplexExternalToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    
    /**
     * Needed because we use itk::SmartPointers and like
     * to have some typedefs just as most itk::Objects
     */
    mitkClassMacro(QmitkComplexExternalToolGUI, QmitkToolGUI);

    /**
     * Created through itk::ObjectFactory, so we need a
     * ::New() method, which is created by this macro.
     * 
     * Look at QmitkToolGui.h to see how we marriaged
     * Qt widgets and itk::SmartPointers
     */
    itkNewMacro(QmitkComplexExternalToolGUI);
    
    virtual ~QmitkComplexExternalToolGUI();

  signals:

  public slots:
  
  protected slots:

  protected:

    /**
     * \brief Creates a QLabel.
     */
    QmitkComplexExternalToolGUI();
};

#endif

