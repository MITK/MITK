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
class QToolButton;

/**
 * \ingroup QmitkModule
 * \brief A card that shows a message next to an icon.
 *
 * An info message has the colors of the progress notification cards. A warning
 * and an error message have the error icon, in amber and in red colors. The
 * message is rich text and wraps. The card follows switches between the light
 * and the dark theme. A closable card has a close button, which hides it.
 */
class MITKQTWIDGETS_EXPORT QmitkInfoCard : public QFrame
{
  Q_OBJECT

public:
  enum class Severity
  {
    Info,
    Warning,
    Error
  };

  explicit QmitkInfoCard(QWidget* parent = nullptr);
  ~QmitkInfoCard() override;

  /**
   * \brief Shows a message, which is rich text: escape plain text that may contain markup.
   *
   * A click on a link in the message emits LinkActivated().
   */
  void SetMessage(const QString& message, Severity severity = Severity::Info);

  QString GetMessage() const;
  Severity GetSeverity() const;

  /** \brief Shows or hides the close button. A card is not closable by default. */
  void SetClosable(bool closable);
  bool IsClosable() const;

signals:
  /** \brief The user closed the card with its close button, which hid it. */
  void Closed();

  /** \brief The user clicked a link in the message. */
  void LinkActivated(const QString& link);

private:
  void UpdateStyle();

  QLabel* m_IconLabel;
  QLabel* m_MessageLabel;
  QToolButton* m_CloseButton;
  Severity m_Severity = Severity::Info;
};

#endif
