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


QPushButton* QmitkSimpleExampleControls::getGenerateMovieButton()
{
    return genMovieButton;
}


QWidget* QmitkSimpleExampleControls::getMovieNavigatorTime()
{
    return movieNavigatorTime;
}
