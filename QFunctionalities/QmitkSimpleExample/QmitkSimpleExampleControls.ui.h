/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void QmitkSimpleExampleControls::sliderXY_valueChanged( int val)
{
    xySliderChanged(val);
}

void QmitkSimpleExampleControls::sliderFP_valueChanged( int val)
{
    fpSliderChanged(val);
}

void QmitkSimpleExampleControls::sliderYZ_valueChanged( int val)
{
    yzSliderChanged(val);
}

void QmitkSimpleExampleControls::sliderXZ_valueChanged( int val)
{
    xzSliderChanged(val);
}

QSlider* QmitkSimpleExampleControls::getSliderXY()
{
    return sliderXY;
}


QSlider* QmitkSimpleExampleControls::getSliderXZ()
{
    return sliderXZ;
}


QSlider* QmitkSimpleExampleControls::getSliderYZ()
{
    return sliderYZ;

}

QSlider* QmitkSimpleExampleControls::getSliderFP()
{
    return sliderFP;
}

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
