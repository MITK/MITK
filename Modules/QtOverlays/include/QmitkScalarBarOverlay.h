/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScalarBarOverlay_h
#define QmitkScalarBarOverlay_h

#include <MitkQtOverlaysExports.h>

// MITK-Stuff
#include <QmitkOverlay.h>
#include <mitkCommon.h>
#include <mitkPropertyList.h>
#include <QmitkScalarBar.h>

/**
 * \class  QmitkScalarBarOverlay
 * \brief Overlay displaying a scale bar in a render window.
 *
 * This overlay wraps a QmitkScalarBar widget and configures it based on
 * properties from a mitk::PropertyList. The scale factor is read from the
 * property identified by the overlay's ID (as a float property), and the
 * color is read from the "overlay.color" property.
 *
 * The overlay registers an observer on the scale property so that the bar
 * updates automatically when the property value changes.
 *
 * \sa QmitkScalarBar
 * \sa QmitkOverlay
 * \sa QmitkTextOverlay
 * \ingroup Qmitk
 */

class MITKQTOVERLAYS_EXPORT QmitkScalarBarOverlay : public QmitkOverlay
{
  Q_OBJECT

public:
  /**
   * \brief Constructor. Creates the internal QmitkScalarBar widget.
   * \param[in] id String identifier used to look up the scale factor property.
   */
  QmitkScalarBarOverlay(const char *id);

  /** \brief Destructor. Removes the property observer. */
  ~QmitkScalarBarOverlay() override;

  /**
   * \brief Configures the scalar bar overlay from the given PropertyList.
   *
   * Reads overlay-specific properties (color, scale factor) and sets up
   * an observer for automatic updates when the scale property changes.
   *
   * \param[in] pl The PropertyList containing the configuration properties.
   */
  void GenerateData(mitk::PropertyList::Pointer pl) override;

  /**
   * \brief Returns the current size of the scalar bar widget.
   * \return The QSize of the internal widget.
   */
  QSize GetNeededSize() override;

protected:
  /**
   * \brief Reads visual properties (color) from the PropertyList and applies them.
   *
   * Reads the "overlay.color" property and configures the pen of the internal
   * QmitkScalarBar accordingly. Falls back to a default color if the property is missing.
   *
   * \param[in] pl The PropertyList to read properties from.
   */
  void GetProperties(mitk::PropertyList::Pointer pl);

  /**
   * \brief Sets up an ITK observer on the given property to auto-update the scale factor.
   * \param[in] prop The property to observe for modifications.
   */
  void SetupCallback(mitk::BaseProperty::Pointer prop);

  /**
   * \brief Reads the scale factor property and applies it to the scalar bar.
   *
   * The scale factor is read from the property identified by m_Id. Falls back
   * to a default value of 2 if the property is not found.
   */
  void SetScaleFactor();

  /** \brief The internal QmitkScalarBar widget. */
  QmitkScalarBar *m_ScalarBar;

  /** \brief The observed property for automatic updates. */
  mitk::BaseProperty::Pointer m_ObservedProperty;

  /** \brief Cached PropertyList for property lookups. */
  mitk::PropertyList::Pointer m_PropertyList;

  /** \brief ITK observer tag for the property modification callback. */
  unsigned long m_ObserverTag;
};

#endif
