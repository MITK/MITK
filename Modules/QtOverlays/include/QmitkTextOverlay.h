/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTextOverlay_h
#define QmitkTextOverlay_h

// MITK
#include <QmitkOverlay.h>
#include <mitkCommon.h>
#include <mitkPropertyList.h>

// Qt
#include <QLabel>

#include <MitkQtOverlaysExports.h>

/**
 * \class  QmitkTextOverlay
 * \brief Overlay that displays a text string in a render window.
 *
 * A QmitkTextOverlay is a text-specific implementation of QmitkOverlay.
 * It can be used whenever a simple text is to be rendered as an overlay in
 * a QmitkRenderWindow.
 *
 * Instead of a generic QWidget (as in QmitkOverlay), a QmitkTextOverlay is
 * internally represented by a QLabel. You can access it via GetWidget().
 *
 * Calling GenerateData(mitk::PropertyList::Pointer) will set up the text overlay.
 * This includes setting the actual text (which must be stored as a StringProperty
 * whose name matches the overlay's ID).
 *
 * Example:
 * \code
 * mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("overlay.text.patientName", "Max");
 * QmitkTextOverlay* nameOverlay = new QmitkTextOverlay("overlay.text.patientName");
 * \endcode
 *
 * The following additional properties customize the appearance:
 *   - \b overlay.color: Text color (mitk::ColorProperty)
 *   - \b overlay.fontSize: Font point size (mitk::IntProperty, default: 9)
 *   - \b overlay.kerning: Whether to use kerning (mitk::BoolProperty, default: true)
 *   - \b overlay.fontFamily: Font family name (mitk::StringProperty, default: "Verdana")
 *
 * The overlay registers an observer on the text property so that the displayed
 * text updates automatically when the property value changes.
 *
 * \sa QmitkOverlay
 * \sa QmitkScalarBarOverlay
 * \sa QmitkOverlayController
 * \ingroup Qmitk
 */

class MITKQTOVERLAYS_EXPORT QmitkTextOverlay : public QmitkOverlay
{
public:
  /**
   * \brief Constructor. Creates the internal QLabel widget.
   * \param[in] id String identifier used to look up the text property by name.
   */
  QmitkTextOverlay(const char *id);

  /** \brief Destructor. Removes the property observer. */
  ~QmitkTextOverlay() override;

  /**
   * \brief Configures the text overlay from the given PropertyList.
   *
   * Reads font properties and the text string from the PropertyList.
   * Sets up an observer for automatic text updates when the property changes.
   *
   * \param[in] pl The PropertyList containing overlay configuration properties.
   * \warning No error is issued if the text property is not found; an empty string is shown.
   */
  void GenerateData(mitk::PropertyList::Pointer pl) override;

  /**
   * \brief Returns the minimum size needed to display the current text.
   * \return The QSize computed from the font metrics and text content.
   */
  QSize GetNeededSize() override;

protected:
  /**
   * \brief Applies font-related properties from the PropertyList to the QLabel.
   *
   * Reads color, font size, kerning, and font family properties and applies them.
   * Missing properties are replaced with default values.
   *
   * \param[in] pl The PropertyList to read font properties from.
   */
  void UpdateFontProperties(mitk::PropertyList::Pointer pl);

  /**
   * \brief Sets up an ITK observer on the given property to auto-update the text.
   * \param[in] prop The property to observe for modifications.
   */
  void SetupCallback(mitk::BaseProperty::Pointer prop);

  /**
   * \brief Reads the current text from the property and updates the QLabel.
   *
   * The text is read from the property identified by m_Id. If the property
   * is not found, the label remains unchanged.
   */
  void UpdateDisplayedTextFromProperties();

  /** \brief QLabel internally representing the text overlay. */
  QLabel *m_Label;

  /** \brief Cached PropertyList for property lookups. */
  mitk::PropertyList::Pointer m_PropertyList;

  /** \brief The observed property for automatic text updates. */
  mitk::BaseProperty::Pointer m_ObservedProperty;

  /** \brief ITK observer tag for the property modification callback. */
  unsigned long m_ObserverTag;
};

#endif
