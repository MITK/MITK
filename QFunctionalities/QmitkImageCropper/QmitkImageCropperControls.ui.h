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

void QmitkImageCropperControls::btnCrop_clicked()
{
  emit cropImage();
}


void QmitkImageCropperControls::init()
{
  groupInfo->hide();
  connect ( cmbImage, SIGNAL(activated(const mitk::DataTreeFilter::Item*)), this, SLOT(ImageNodeChanged(const mitk::DataTreeFilter::Item*)) ); 
  m_SurroundingSlider->hide();
  m_SurroundingSpin->hide();
  m_TLGrayvalue->hide();
}


void QmitkImageCropperControls::ImageNodeChanged(const mitk::DataTreeFilter::Item* item)
{
  // called when the selection of the image selector changes
  btnCrop->setEnabled( item );
  emit ImageSelectionChanged();
}


const mitk::DataTreeIteratorClone QmitkImageCropperControls::selectedImage()
{
  return cmbImage->GetFilter()->GetIteratorToSelectedItem();
}


void QmitkImageCropperControls::chkInformation_toggled( bool on )
{
  if (on)
    groupInfo->show();
  else
    groupInfo->hide();
}


void QmitkImageCropperControls::m_EnableSurroundingCheckBox_toggled( bool value)
{
    if(value)
    {
	m_SurroundingSlider->show();
	m_SurroundingSpin->show();
	m_TLGrayvalue->show();
    }
    else
    {
	m_SurroundingSlider->hide();
	m_SurroundingSpin->hide();
	m_TLGrayvalue->hide();
    }
    
}
