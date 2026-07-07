/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScreenshotWidget_h
#define QmitkScreenshotWidget_h

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkScreenshotWidget;
}

/**
 * \brief Widget that displays a screenshot thumbnail with a remove button.
 *
 * Used within the screenshot question widget to show individual screenshots
 * that have been captured or loaded. Each widget shows a preview and allows
 * the user to remove it.
 */
class QmitkScreenshotWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct an empty screenshot widget.
   * \param parent Optional parent widget.
   */
  explicit QmitkScreenshotWidget(QWidget* parent = nullptr);

  /**
   * \brief Construct a screenshot widget with an initial screenshot.
   * \param screenshotPath File path to the screenshot image.
   * \param parent Optional parent widget.
   */
  explicit QmitkScreenshotWidget(const QString& screenshotPath, QWidget* parent = nullptr);
  ~QmitkScreenshotWidget() override;

  /**
   * \brief Set the screenshot image to display.
   * \param path File path to the screenshot image.
   */
  void SetScreenshot(const QString& path);

  /** \brief Return the file path of the currently displayed screenshot. */
  QString GetScreenshotPath() const;

signals:
  /**
   * \brief Emitted when the user clicks the remove button.
   * \param sender Pointer to the widget requesting removal.
   */
  void Remove(QmitkScreenshotWidget* sender);

private:
  void OnRemoveButtonClicked();

  std::unique_ptr<Ui::QmitkScreenshotWidget> m_Ui;
  QString m_ScreenshotPath;
};

#endif
