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


#ifndef MITKOVERLAY_H_HEADER_INCLUDED_C10DC4EB
#define MITKOVERLAY_H_HEADER_INCLUDED_C10DC4EB

// MITK
#include "mitkCommon.h"
#include "mitkPropertyList.h"

// Qt
#include <QWidget>

#include <MitkQtOverlaysExports.h>


/**
 \brief Abstract base class for all overlay-objects in MITK

 This class is the basis for all classes representing objects that can be visualized as overlays in MITK.
 It encapsulates an ID, as well as a display-position and a layer.

 The ID is used to access mitkProperties in a PropertyList that holds information that is needed for the visualization,
 e.g. text for TextOverlays or scaleFactor for ScalarBarOverlays ...

 The display-position encodes where on the screen the overlay will be positioned at
 (see and USE the constants defined by DisplayPosition):
\verbatim
 0 - 1 - 2
 |   |   |
 3 -   - 4
 |   |   |
 5 - 6 - 7
\endverbatim

 The layer is needed if several overlays shall be put in the same position.
 In this case the layer defines the order in which the objects are layouted.

 \ingroup Qmitk
*/
class MitkQtOverlays_EXPORT QmitkOverlay : public QObject
{
  Q_OBJECT

public:

  /** \brief enumeration of all possible display positions  */
  enum DisplayPosition
  {
    top_Left = 0,
    top_Center = 1,
    top_Right = 2,
    middle_Left = 3,
    middle_Right = 4,
    bottom_Left = 5,
    bottom_Center = 6,
    bottom_Right = 7
  };

  /**
  * @brief Constructor with string ID
  **/
  QmitkOverlay(const char* id);

  /**
  * @brief Default Destructor
  **/
  virtual ~QmitkOverlay();

  /** \brief setter for the display-position  */
  virtual void SetPosition( DisplayPosition );

  /** \brief getter for the display-position  */
  virtual DisplayPosition GetPosition();

  /** \brief setter for the layer  */
  virtual void SetLayer( unsigned int );

  /** \brief getter for the layer  */
  virtual unsigned int GetLayer();

  /**
  * \brief abstract method to internally setup the overlay
  */
  virtual void GenerateData( mitk::PropertyList::Pointer /*pl*/ ) {};

  /**
  * \brief returns the internally handled QWidget
  */
  virtual QWidget* GetWidget();

  virtual QSize GetNeededSize() = 0;


protected:

  /**
    \brief Add drop shadow effect via QGraphicsEffect
  */
  void AddDropShadow( QWidget* widget );
  /** \brief ID of the overlay */
  const char* m_Id;

  /** \brief position of the overlay */
  DisplayPosition m_Position;

  /** \brief layer of the overlay */
  unsigned int m_Layer;

  /** \brief internal QWidget representing the overlay */
  QWidget* m_Widget;

  bool m_WidgetIsCustom;
};


#endif /* MITKOVERLAY_H_HEADER_INCLUDED_C10DC4EB */


