/*=========================================================================
 Program:   Medical Imaging & Interaction Toolkit
 Module:    $RCSfile$
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 13820 $
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/ for details.
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 =========================================================================*/

#include <mitkBaseApplication.h>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVariant>

int main(int argc, char** argv)
{
    
    mitk::BaseApplication myApp(argc, argv);
    myApp.setApplicationName("MITK Qmlbench");
    myApp.setOrganizationName("DKFZ");
    
    setenv("QML_BAD_GUI_RENDER_LOOP", "1", 1);

    myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "org.mitk.gui.qml.extapplication");
    return myApp.run();
}