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
  connect ( qmitkNodeSelector, SIGNAL(TreeNodeChanged()), this, SLOT(ImageNodeChanged()) ); 
}


void QmitkImageCropperControls::ImageNodeChanged()
{
  // called when the selection of the image selector changes
  btnCrop->setEnabled( qmitkNodeSelector->GetSelectedNode() );
  emit ImageSelectionChanged();
}


const mitk::DataTreeIteratorClone QmitkImageCropperControls::selectedImage()
{
  return *qmitkNodeSelector->GetSelectedIterator();
}


void QmitkImageCropperControls::chkInformation_toggled( bool on )
{
  if (on)
    groupInfo->show();
  else
    groupInfo->hide();
}
