/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAlgorithmProfileViewer_h
#define QmitkAlgorithmProfileViewer_h

#include <mapDeploymentDLLInfo.h>

#include <MitkMatchPointRegistrationUIExports.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkAlgorithmProfileViewer;
}

/**
 * \class QmitkAlgorithmProfileViewer
 * \brief Widget that displays the detailed profile and identification information of a MatchPoint registration algorithm.
 *
 * This widget renders a rich-text view showing the algorithm's identification (namespace, name,
 * version, build tag, library path), description, keywords, characteristics (data type, computation
 * style, determinism, resolution style, dimensions, modalities, transform model, metric,
 * optimization, interaction), and contact/usage information (contact, terms, citation).
 *
 * The displayed information is parsed from the algorithm's profile string stored in the
 * map::deployment::DLLInfo object. The widget does not retain a reference to the DLLInfo;
 * call updateInfo() each time the information should be refreshed.
 *
 * \sa QmitkAlgorithmListModel, QmitkAlgorithmSettingsConfig
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmProfileViewer : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the algorithm profile viewer widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkAlgorithmProfileViewer(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkAlgorithmProfileViewer() override;

  /**
   * \brief Updates the widget to display information from the given DLLInfo.
   *
   * Parses the algorithm profile string from the DLLInfo and renders identification,
   * description, keywords, characteristics, and contact information as HTML in the
   * internal text widget. If \p newInfo is \c nullptr, the widget content is cleared.
   *
   * \param[in] newInfo Pointer to the DLLInfo instance to display. The pointer is not
   *   stored internally; to refresh the display, call this method again.
   *
   * \note The DLLInfo is not stored internally or as reference.
   *   To update the widget you must call updateInfo() again.
   */
  void updateInfo(const map::deployment::DLLInfo *newInfo);

public Q_SLOTS:
  /**
   * \brief Slot that delegates to updateInfo().
   *
   * Convenience slot that can be connected to signals providing a DLLInfo pointer
   * to trigger a display update.
   *
   * \param[in] newInfo Pointer to the DLLInfo instance to display.
   */
  void OnInfoChanged(const map::deployment::DLLInfo *newInfo);

protected:
  std::unique_ptr<Ui::QmitkAlgorithmProfileViewer> m_Controls;
};

#endif
