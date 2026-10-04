/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInfoCard_h
#define QmitkInfoCard_h

#include <MitkQtWidgetsExports.h>

#include <QFrame>

class QLabel;

/**
 * \ingroup QmitkModule
 * \brief A card that shows a message next to an icon.
 *
 * An info message has the colors of the progress notification cards, an error
 * message an error icon and error colors. The message is rich text and wraps.
 * The card follows switches between the light and the dark theme.
 */
class MITKQTWIDGETS_EXPORT QmitkInfoCard : public QFrame
{
  Q_OBJECT

public:
  enum class Severity
  {
    Info,
    Error
  };

  explicit QmitkInfoCard(QWidget* parent = nullptr);
  ~QmitkInfoCard() override;

  /** \brief Shows a message, which is rich text: escape plain text that may contain markup. */
  void SetMessage(const QString& message, Severity severity = Severity::Info);

  QString GetMessage() const;
  Severity GetSeverity() const;

private:
  void UpdateStyle();

  QLabel* m_IconLabel;
  QLabel* m_MessageLabel;
  Severity m_Severity = Severity::Info;
};

#endif
