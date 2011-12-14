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


#ifndef MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB
#define MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB

#include "OverlaysExports.h"

// MITK-Stuff
#include "mitkCommon.h"
#include "mitkPropertyList.h"
#include "QmitkOverlay.h"
#include <QmitkScalarBar.h>


/** \class  QmitkScalarBarOverlay
* \brief object representing a text that is drawn as an overlay
*
* \ingroup Qmitk
*/


  class Overlays_EXPORT QmitkScalarBarOverlay : public QmitkOverlay
  {
    Q_OBJECT

  public:

    /**
    * @brief Default Constructor
    **/
    QmitkScalarBarOverlay( const char* id );

    /**
    * @brief Default Destructor
    **/
    virtual ~QmitkScalarBarOverlay();

    /**
    * \brief Setup the QLabel with overlay specific information
    *
    * First, this method sets text-overlay specific properties as described in the class docu above.
    * Secondly, the actual text of the label is set.
    * 
    * \WARNING No error will be issued if the property containing the text is not found, the TextOverlay 
    * will show an empty string!
    */
    virtual void GenerateData( mitk::PropertyList::Pointer );

  protected:

    /**
    * \brief internal helper class to determine text-properties
    *
    * This method is only used internally to apply the text specific properties that can be set 
    * using a mitk::PropertyList. If a property cannot be found, a default value is used.
    *
    * The values of these properties are then attributed to the label using QFont and QPalette.
    */ 
    void GetProperties( mitk::PropertyList::Pointer );
   
    void SetupCallback( mitk::BaseProperty::Pointer prop );
   
    void SetScaleFactor();

    /** \brief QWidget internally representing the TextOverlay */
    QmitkScalarBar* m_ScalarBar;  

    mitk::PropertyList::Pointer m_PropertyList;
    unsigned long m_ObserverTag;
  };


#endif /* MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB */


