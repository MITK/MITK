/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTEXTOVERLAY_H_HEADER_INCLUDED_C10DC4EB
#define MITKTEXTOVERLAY_H_HEADER_INCLUDED_C10DC4EB

// MITK-Stuff
#include "mitkCommon.h"
#include "QmitkOverlay.h"

#include <QLabel>

/** \class  QmitkTextOverlay
* \brief object representing a text that is drawn as an overlay
*
* A QmitkTextOverlay is a text-specific implementation of QmitkOverlay.
* It can be used whenever a simple text is to be rendered as an overlay in 
* a QmitkRenderWindow.
*
* Instead of a QWidget (as in QmitkOverlay) a QmitkTextOverlay is internally 
* represented by a QLabel. You can access it via GetWidget().
*
* Calling GenerateData( mitk::PropertyList::Pointer ) will setup the textoverlay.
* This includes setting of the actual text (that must be stored in the property 
* with the name that is given the overlay as ID).
* 
* e.g. mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( "overlay.text.patientName", "Max" );
* --
* QmitkTextOverlay* nameOverlay = new QmitkTextOverlay( "overlay.text.patientName" );
*
* In order to customize the look of the textoverlays, a number of additional properties can be set 
* (default values in square brackets):
* 
* overlay.color       : defines the text-color (mitk::ColorProperty)
* overlay.fontSize    : defines the fontSize of the text (mitk::IntProperty)
* overlay.kerning     : defines if kerning is to be used (mitk::BoolProperty)
* overlay.fontFamily  : defines the fon family that is to be used (mitk::StringProperty)
*
* \ingroup Qmitk
*/


  class QMITK_EXPORT QmitkTextOverlay : public QmitkOverlay
  {
  public:

    /**
    * @brief Default Constructor
    **/
    QmitkTextOverlay( const char* id );

    /**
    * @brief Default Destructor
    **/
    virtual ~QmitkTextOverlay();

    void GenerateData( mitk::PropertyList::Pointer );

    QLabel* GetWidget();

  protected:

    void GetTextProperties( mitk::PropertyList::Pointer );
    
    QLabel* m_Widget;    
  };


#endif /* MITKTEXTOVERLAY_H_HEADER_INCLUDED_C10DC4EB */


