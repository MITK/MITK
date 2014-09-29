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


#ifndef MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191
#define MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191

// MITK-Stuff
#include "mitkCommon.h"
#include "mitkPropertyList.h"
#include "QmitkOverlay.h"

#include <QObject>

#include <string>

#include <MitkQtOverlaysExports.h>


class QmitkRenderWindow;

/** \class  QmitkOverlayController
* \brief controller that manages the positioning and stacking of QmitkOverlays
*
* This controller manages all QmitkOverlays of one QmitkRenderWindow.
*
* When constructed, it creates one QWidget for each possible display-position and sets the
* appropriate attributes and layouts.
*
* It is possible to add new Overlays using AddOverlay( QmitkOverlay ).
* This overlay will be added to the correct Widget according to its destined position (stored in QmitkOverlay).
* If this widget already holds an overlay, the layer-property is taken into account. If no layer has been set,
* the overlay will be appended at the end.
*
* It is possible to set the visibility of all overlays at a time using SetOverlayVisibility(bool).
*
* RenderWindow specific properties can be set using the internal mitk::PropertyList. This propertyList and the
* 'default' propertyList of the RenderingManager will be concatenated before the overlay is set up.
* If one property exists in both propertyLists, the one in the QmitkOverlayController will be used!
*
* \sa QmitkOverlay
* \sa QmitkRenderWindow
* \ingroup Qmitk
*/

class MitkQtOverlays_EXPORT QmitkOverlayController : public QObject
{
  Q_OBJECT

public:

  /**
  * \brief constructor with mandatory QmitkRenderWindow and optional mitk::PropertyList
  */
  QmitkOverlayController( QmitkRenderWindow* rw, mitk::PropertyList* pl = NULL );
  virtual ~QmitkOverlayController();

  /**
  * \brief adds an instance of QmitkOverlay to the RenderWindow
  *
  * This method adds the given QmitkOverlay as a sub-widget to the registered RenderWindow.
  * It will be added to the correct position in the RenderWindow as it's defined by the overlays
  * position-variable. The layer-property will only be considered if necessary.
  */
  void AddOverlay( QmitkOverlay* );

  void RemoveOverlay( QmitkOverlay* );

  void RemoveAllOverlays();


  /**
  * \brief setting the visibility of all overlays
  */
  void SetOverlayVisibility( bool visible );

  /**
  * \brief getter for the RenderWindow-specific PropertyList
  */
  mitk::PropertyList* GetPropertyList();

  /**
  * \brief setter for the RenderWindow-specific PropertyList
  */
  void SetPropertyList( mitk::PropertyList* );


public slots :
    /**
    * \brief adjusts the position of all overlays to the position of the RenderWindow
    *
    * This method updates the position of all Widgets according to the position of the RenderWindow
    * and the extend of the overlays.
    */
    void AdjustAllOverlayPosition();

    void AdjustOverlayPosition( QmitkOverlay::DisplayPosition displayPosition );

    void UpdateAllOverlays();

    void UpdateOverlayData( QmitkOverlay* overlay );

protected:

  /**
  * \brief setting up the widgets that will hold all overlays
  *
  * This method sets up the 8 QWidgets that will later hold all QmitkOverlays.
  * This includes the correct setting of layouts, alignments and the widget
  * attributes necessary to achieve a translucent background and correct rendering
  * on all platforms.
  */
  void InitializeOverlayLayout();

  /**
  * \brief re-aligning the overlays - not implemented yet
  */
  virtual void AlignOverlays();

  /**
  * \brief initializes one QWidget - internally used by InitializeOverlayLayout()
  */
  void InitializeWidget( QmitkOverlay::DisplayPosition pos );

  void RestackOverlays( QmitkOverlay::DisplayPosition pos );

  QSize GetMinimumSizeForWidget( QmitkOverlay::DisplayPosition displayPosition );

  typedef std::map< QmitkOverlay::DisplayPosition, QWidget* > OverlayPositionMap;
  typedef std::vector< QmitkOverlay* > OverlayVector;

  /**
  * \brief all QmitkOverlays that are currently added
  */
  OverlayVector                 m_AllOverlays;

  /**
  * \brief all possible positions and the QWidgets representing the corresponding QmitkOverlays
  */
  OverlayPositionMap            m_PositionedOverlays;

  /**
  * \brief RenderWindow that all Overlays will be added to
  */
  QmitkRenderWindow*            m_RenderWindow;

  /**
  * \brief PropertyList for RenderWindow-specific properties
  */
  mitk::PropertyList::Pointer   m_PropertyList;

};


#endif /* MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191 */
