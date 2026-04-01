/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkOverlay_h
#define QmitkOverlay_h

// MITK
#include <mitkCommon.h>
#include <mitkPropertyList.h>

// Qt
#include <QWidget>

#include <MitkQtOverlaysExports.h>

/**
 * \brief Abstract base class for all Qt-based overlay widgets in MITK.
 *
 * This class is the basis for all classes representing objects that can be visualized
 * as overlays in MITK render windows. It encapsulates an ID, a display-position, and
 * a layer.
 *
 * The ID is used to access mitkProperties in a PropertyList that holds information
 * needed for the visualization, e.g. text for TextOverlays or scaleFactor for
 * ScalarBarOverlays.
 *
 * The display-position encodes where on the screen the overlay will be positioned at
 * (see and USE the constants defined by DisplayPosition):
 * \verbatim
 * 0 - 1 - 2
 * |   |   |
 * 3 -   - 4
 * |   |   |
 * 5 - 6 - 7
 * \endverbatim
 *
 * The layer is needed if several overlays shall be put in the same position.
 * In this case the layer defines the order in which the objects are laid out.
 *
 * \sa QmitkOverlayController
 * \sa QmitkTextOverlay
 * \sa QmitkScalarBarOverlay
 * \sa QmitkCustomWidgetOverlay
 * \ingroup Qmitk
 */
class MITKQTOVERLAYS_EXPORT QmitkOverlay : public QObject
{
  Q_OBJECT

public:
  /**
   * \brief Enumeration of all possible display positions in a render window.
   *
   * Positions are arranged in a 3x3 grid (excluding the center cell):
   * top-left(0), top-center(1), top-right(2), middle-left(3), middle-right(4),
   * bottom-left(5), bottom-center(6), bottom-right(7).
   */
  enum DisplayPosition
  {
    top_Left = 0,      ///< Top-left corner position
    top_Center = 1,    ///< Top-center position
    top_Right = 2,     ///< Top-right corner position
    middle_Left = 3,   ///< Middle-left position
    middle_Right = 4,  ///< Middle-right position
    bottom_Left = 5,   ///< Bottom-left corner position
    bottom_Center = 6, ///< Bottom-center position
    bottom_Right = 7   ///< Bottom-right corner position
  };

  /**
   * \brief Constructor.
   * \param[in] id String identifier used to look up overlay properties in a PropertyList.
   */
  QmitkOverlay(const char *id);

  /** \brief Destructor. Deletes the internal widget unless it was set externally. */
  ~QmitkOverlay() override;

  /**
   * \brief Sets the display position of this overlay.
   * \param[in] pos The desired position in the render window.
   */
  virtual void SetPosition(DisplayPosition pos);

  /**
   * \brief Returns the current display position.
   * \return The DisplayPosition of this overlay.
   */
  virtual DisplayPosition GetPosition();

  /**
   * \brief Sets the stacking layer of this overlay.
   * \param[in] layer The layer index (lower values are rendered first).
   */
  virtual void SetLayer(unsigned int layer);

  /**
   * \brief Returns the current stacking layer.
   * \return The layer index.
   */
  virtual unsigned int GetLayer();

  /**
   * \brief Configures the overlay using properties from the given PropertyList.
   *
   * Subclasses override this to extract relevant properties (e.g. text, scale factor)
   * from the PropertyList and apply them to the internal widget.
   *
   * \param[in] pl The PropertyList containing overlay configuration properties.
   */
  virtual void GenerateData(mitk::PropertyList::Pointer pl);

  /**
   * \brief Returns the internal QWidget representing this overlay.
   * \return Pointer to the QWidget, or nullptr if none has been created.
   */
  virtual QWidget *GetWidget();

  /**
   * \brief Returns the minimum size needed to fully display this overlay.
   * \return The needed QSize.
   */
  virtual QSize GetNeededSize() = 0;

protected:
  /**
   * \brief Adds a drop shadow graphics effect to the given widget.
   * \param[in] widget The widget to add the shadow effect to.
   */
  void AddDropShadow(QWidget *widget);

  /** \brief ID of the overlay, used as property key for lookups. */
  const char *m_Id;

  /** \brief Current display position of the overlay. */
  DisplayPosition m_Position;

  /** \brief Stacking layer of the overlay. */
  unsigned int m_Layer;

  /** \brief Internal QWidget representing the overlay content. */
  QWidget *m_Widget;

  /** \brief Flag indicating whether the widget was set externally (custom overlay). */
  bool m_WidgetIsCustom;
};

#endif
