/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void QmitkVolumetryWidget::SetDataTreeNode(mitk::DataTreeNode* node)
{
  std::cout << "Signal received!!!" << std::endl;
  m_Node = node;
  std::string name;
  if (node->GetName(name))
  {
    m_TreeNodeNameLabel->setText(name);
  }
}


void QmitkVolumetryWidget::CalculateVolume()
{
  if (m_Node)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());
    if (image)
    {
      mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
      volCalc->SetImage(image);
      volCalc->SetThreshold(m_ThresholdSlider->value());
      volCalc->ComputeVolume();
      std::stringstream vs;
      vs << volCalc->GetVolume() << " ml";
      m_Result->setText(vs.str());
      std::cout << "Volume:" << vs.str() << std::endl;
    }
  }
}
