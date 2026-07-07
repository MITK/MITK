/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowWidget_h
#define QmitkLevelWindowWidget_h

#include <MitkQtWidgetsExports.h>
#include <mitkLevelWindowManager.h>

#include <QWidget>
#include <memory>

// Forward declarations
namespace Ui
{
  class QmitkLevelWindow;
}

/** \brief Composite widget for level/window adjustment.
 *
 * Combines a slider widget and a line-edit widget for interactive
 * level/window manipulation. Manages a LevelWindowManager internally.
 *
 * \ingroup QmitkModule
 * \sa QmitkSliderLevelWindowWidget, QmitkLineEditLevelWindowWidget, mitk::LevelWindowManager
 */
class MITKQTWIDGETS_EXPORT QmitkLevelWindowWidget : public QWidget
{
  Q_OBJECT
public:
  /** \brief Construct the level window widget.
   * \param[in] parent Parent widget.
   * \param[in] f Window flags.
   */
  QmitkLevelWindowWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkLevelWindowWidget() override;

  /** \brief Get the internal LevelWindowManager.
   * \return Pointer to the LevelWindowManager.
   */
  mitk::LevelWindowManager *GetManager();

public slots:
  /** \brief Set the data storage for the level window manager.
   * \param[in] ds The data storage to observe.
   */
  void SetDataStorage(mitk::DataStorage *ds);

protected:
  // unsigned long m_ObserverTag;
  mitk::LevelWindowManager::Pointer m_Manager;

private:
  // GUI controls of this plugin
  std::unique_ptr<Ui::QmitkLevelWindow> ui;

};
#endif
