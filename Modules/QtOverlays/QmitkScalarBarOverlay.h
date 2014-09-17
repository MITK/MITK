/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB
#define MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB

#include <MitkQtOverlaysExports.h>

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


  class MitkQtOverlays_EXPORT QmitkScalarBarOverlay : public QmitkOverlay
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

    QSize GetNeededSize();


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

    mitk::BaseProperty::Pointer m_ObservedProperty;

    mitk::PropertyList::Pointer m_PropertyList;
    unsigned long m_ObserverTag;
  };


#endif /* MITKSCALARBAROVERLAY_H_HEADER_INCLUDED_C10DC4EB */


