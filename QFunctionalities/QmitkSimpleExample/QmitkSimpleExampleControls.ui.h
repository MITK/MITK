/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

QWidget* QmitkSimpleExampleControls::getSliceNavigatorTransversal()
{
    return sliceNavigatorTransversal;
}


QWidget* QmitkSimpleExampleControls::getSliceNavigatorFrontal()
{
    return sliceNavigatorFrontal;
}


QWidget* QmitkSimpleExampleControls::getSliceNavigatorSagittal()
{
    return sliceNavigatorSagittal;
}


QWidget * QmitkSimpleExampleControls::getSliceNavigatorTime()
{
    return sliceNavigatorTime;
}


QWidget* QmitkSimpleExampleControls::getStereoSelect()
{
    return stereoSelect;
}


QPushButton* QmitkSimpleExampleControls::getReInitializeNavigatorsButton()
{
    return reInitializeNavigatorsButton;
}
