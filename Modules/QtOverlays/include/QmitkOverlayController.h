/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOverlayController_h
#define QmitkOverlayController_h

// MITK-Stuff
#include <QmitkOverlay.h>
#include <mitkCommon.h>
#include <mitkPropertyList.h>

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

class MITKQTOVERLAYS_EXPORT QmitkOverlayController : public QObject
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   *
   * Initializes overlay container widgets for all eight display positions and
   * connects to the render window's moved() signal for automatic repositioning.
   *
   * \param[in] rw The QmitkRenderWindow that overlays will be displayed on. Must not be nullptr.
   * \param[in] pl Optional PropertyList with render-window-specific properties.
   */
  QmitkOverlayController(QmitkRenderWindow *rw, mitk::PropertyList *pl = nullptr);

  /** \brief Destructor. */
  ~QmitkOverlayController() override;

  /**
   * \brief Adds an overlay to the managed render window.
   *
   * The overlay is placed in the container widget corresponding to its
   * display position. Its GenerateData() method is called with the
   * concatenated property list, and the layout is restacked and repositioned.
   *
   * \param[in] overlay The overlay to add. Ignored if nullptr.
   */
  void AddOverlay(QmitkOverlay* overlay);

  /**
   * \brief Removes a specific overlay from the render window.
   *
   * The overlay's widget is unparented and hidden, and the overlay is
   * scheduled for deletion.
   *
   * \param[in] overlay The overlay to remove. Ignored if nullptr.
   */
  void RemoveOverlay(QmitkOverlay* overlay);

  /**
   * \brief Removes all overlays from the render window.
   *
   * All overlay widgets are unparented, hidden, and scheduled for deletion.
   */
  void RemoveAllOverlays();

  /**
   * \brief Sets the visibility of all overlay container widgets and their contents.
   * \param[in] visible If true, all overlays are shown; if false, all are hidden.
   */
  void SetOverlayVisibility(bool visible);

  /**
   * \brief Sets the render-window-specific PropertyList.
   * \param[in] pl The new PropertyList.
   */
  void SetPropertyList(mitk::PropertyList* pl);

public slots:
  /**
   * \brief Repositions all overlay container widgets to match the current render window geometry.
   *
   * Called automatically when the render window is moved.
   */
  void AdjustAllOverlayPosition();

  /**
   * \brief Repositions the overlay container widget at the specified display position.
   * \param[in] displayPosition The display position whose container widget should be repositioned.
   */
  void AdjustOverlayPosition(QmitkOverlay::DisplayPosition displayPosition);

  /**
   * \brief Updates the data of all managed overlays by calling their GenerateData() methods.
   */
  void UpdateAllOverlays();

  /**
   * \brief Updates the data of a single overlay and readjusts its position.
   * \param[in] overlay The overlay to update. Ignored if nullptr.
   */
  void UpdateOverlayData(QmitkOverlay *overlay);

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
  void InitializeWidget(QmitkOverlay::DisplayPosition pos);

  void RestackOverlays(QmitkOverlay::DisplayPosition pos);

  QSize GetMinimumSizeForWidget(QmitkOverlay::DisplayPosition displayPosition);

  typedef std::map<QmitkOverlay::DisplayPosition, QWidget *> OverlayPositionMap;
  typedef std::vector<QmitkOverlay *> OverlayVector;

  /**
  * \brief all QmitkOverlays that are currently added
  */
  OverlayVector m_AllOverlays;

  /**
  * \brief all possible positions and the QWidgets representing the corresponding QmitkOverlays
  */
  OverlayPositionMap m_PositionedOverlays;

  /**
  * \brief RenderWindow that all Overlays will be added to
  */
  QmitkRenderWindow *m_RenderWindow;

  /**
  * \brief PropertyList for RenderWindow-specific properties
  */
  mitk::PropertyList::Pointer m_PropertyList;
};

#endif
