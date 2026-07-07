/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef qclickablelabel_h
#define qclickablelabel_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>

#include <map>
#include <vector>

#include <mitkCommon.h>

/**
 * \brief A QLabel with named rectangular hotspots that emit signals when clicked.
 *
 * Useful for overlaying interactive regions on a pixmap. Each hotspot is defined
 * by a name and a QRect. When the user clicks within a hotspot, signals are emitted
 * with both the hotspot name and index.
 *
 * \note Stretched images should be avoided because hotspot positions are not adjusted.
 *
 * \sa QmitkStandardViews
 */
class MITKQTWIDGETSEXT_EXPORT QClickableLabel : public QLabel
{
  Q_OBJECT

public:
  /**
   * \brief Construct a clickable label.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QClickableLabel(QWidget *parent, Qt::WindowFlags f = {});

  /**
   * \brief Construct a clickable label with initial text.
   * \param[in] text The label text.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QClickableLabel(const QString &text, QWidget *parent, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QClickableLabel() override;

  /**
   * \brief Add a named hotspot at the given rectangle position.
   * \param[in] name The hotspot name (used in signals).
   * \param[in] position The rectangular region of the hotspot.
   */
  void AddHotspot(const QString &name, const QRect position);

  /**
   * \brief Remove a hotspot by name.
   * \param[in] name The name of the hotspot to remove.
   */
  void RemoveHotspot(const QString &name);

  /**
   * \brief Remove a hotspot by index.
   * \param[in] hotspotIndex The index of the hotspot to remove.
   */
  void RemoveHotspot(unsigned int hotspotIndex);

  /** \brief Remove all hotspots. */
  void RemoveAllHotspots();

signals:
  /**
   * \brief Emitted on mouse press within a hotspot (by name).
   * \param[in] hotspotName The name of the pressed hotspot.
   */
  void mousePressed(const QString &hotspotName);

  /**
   * \brief Emitted on mouse press within a hotspot (by index).
   * \param[in] hotspotIndex The index of the pressed hotspot.
   */
  void mousePressed(unsigned int hotspotIndex);

  /**
   * \brief Emitted on mouse release within a hotspot (by name).
   * \param[in] hotspotName The name of the released hotspot.
   */
  void mouseReleased(const QString &hotspotName);

  /**
   * \brief Emitted on mouse release within a hotspot (by index).
   * \param[in] hotspotIndex The index of the released hotspot.
   */
  void mouseReleased(unsigned int hotspotIndex);

protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  /// returns index == m_Hotspots.size() if nothing is hit
  unsigned int matchingRect(const QPoint &p);

  typedef std::vector<QRect> RectVectorType;
  RectVectorType m_Hotspots;

  typedef std::map<QString, unsigned int> NameToIndexMapType;
  typedef std::map<unsigned int, QString> IndexToNameMapType;
  NameToIndexMapType m_HotspotIndexForName;
  IndexToNameMapType m_HotspotNameForIndex;
};

#endif
