/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentConversationWidget_h
#define QmitkAgentConversationWidget_h

#include <MitkAgentUIExports.h>

#include <QmitkAgentStep.h>

#include <QByteArray>
#include <QHash>
#include <QTextBrowser>
#include <QTextCursor>

#include <vector>

class QImage;
class QTextImageFormat;
class QTextList;

/**
 * \brief The conversation with the agent: the user's messages, the agent's
 *        answers, and the steps in between.
 *
 * The user's messages appear in frames that look like the input. The steps
 * of a turn form a group, which a summary line shows or hides. The images of
 * a step appear as thumbnails, which open the image when clicked. While the
 * steps are hidden, smaller thumbnails of all their images show below the
 * summary, as the steps arrive. The text is larger than the rest of the
 * application, notes and steps keep its size.
 */
class MITKAGENTUI_EXPORT QmitkAgentConversationWidget : public QTextBrowser
{
public:
  /** \brief How much larger the text is than that of the application, also for the input. */
  static constexpr qreal TextScale = 1.2;

  explicit QmitkAgentConversationWidget(QWidget* parent = nullptr);
  ~QmitkAgentConversationWidget() override;

  void AppendUserMessage(const QString& text);
  void AppendMarkdown(const QString& markdown);
  void AppendNote(const QString& text);
  void AppendError(const QString& text);

  /** \brief Starts a block for text that arrives in pieces. Any other append ends the pieces. */
  void BeginStreamedText();

  /** \brief Adds a piece to the text that BeginStreamedText() started, unless other content followed it. */
  void AppendStreamedText(const QString& text);

  /**
   * \brief Replaces the streamed text with the complete one, formatted, or appends it if none is streamed.
   *
   * The text is replaced where it is, also above content that followed it.
   */
  void EndStreamedText(const QString& markdown);

  /** \brief Adds a step to the last group of steps, or starts a new group after other content. */
  void AppendStep(const QmitkAgentStep& step);

  /** \brief Shows or hides the steps of all groups, also of those that follow. */
  void SetStepsShown(bool shown);

  /** \brief Removes the whole conversation. */
  void Clear();

protected:
  void paintEvent(QPaintEvent* event) override;

  /** \brief Shows the real target of an http or https link as the tool tip, whatever title the Markdown gave the link. */
  bool viewportEvent(QEvent* event) override;

private:
  /** \brief Consecutive steps, shown as a list or as one summary line. */
  struct StepGroup
  {
    int SummaryBlock;

    /** \brief Below the summary, the thumbnails of the images of all steps, shown while the steps are hidden. */
    int PreviewBlock;

    int LastBlock;

    /** \brief The list of the steps; null until the first step. */
    QTextList* List;

    int Steps;
    int Failures;

    /** \brief Changes of the agent's memory, which the summary shows, unlike other steps. */
    int SavedNotes;
    int DeletedNotes;

    bool Expanded;
  };

  /**
   * \brief A cursor in a new block at the end for a message.
   *
   * Ends the pieces of the streamed text and the group of steps, if there are.
   */
  QTextCursor AppendBlock();

  /** \brief Keeps where the streamed text ends, as other content follows it now, or forgets it if it is empty. */
  void InterruptStreamedText();

  /** \brief A cursor in a new list item at the end for a step, which joins the last group of steps or starts a new one. */
  QTextCursor AppendStepItem();

  /** \brief A cursor in a new block below the item of the last step, for its images. */
  QTextCursor AppendStepImageBlock();

  /** \brief Hides what the step appended if the group is collapsed, and updates its summary. */
  void FinishStep(const QmitkAgentStep& step);

  void UpdateStepSummary(std::size_t index);
  void SetStepGroupExpanded(std::size_t index, bool expanded);

  /** \brief Adds the thumbnails to the preview of the last group of steps. */
  void AppendPreviews(const QList<QTextImageFormat>& previews);

  /** \brief Shows the preview of the group if its steps are hidden and it has thumbnails, else hides it. */
  void UpdatePreview(const StepGroup& group);

  void AppendHtml(const QString& html);

  /**
   * \brief Stores a thumbnail of the image in the document and returns it, as a link.
   *
   * \param name The name of the thumbnail in the document.
   */
  QTextImageFormat CreateThumbnail(const QImage& image, const QSize& size, const QString& name, const QString& link);

  void ShowImage(int id);
  void OnLinkClicked(const QUrl& url);
  void ScrollToEnd();

  /**
   * \brief The block where the streamed text starts, or -1.
   *
   * A block number, not a position: rewriting the summary of a group of
   * steps further up moves the positions after it.
   */
  int m_StreamBlock;

  /** \brief The block where the streamed text ends once other content follows it, or -1 while it is the end of the conversation. */
  int m_StreamLastBlock;

  /** \brief Whether the conversation scrolls along, which stops while the user reads further up. */
  bool m_FollowEnd;

  /** \brief The font size of notes and steps in points, smaller than the text's; 0 if unknown. */
  qreal m_NoteFontSize;

  std::vector<StepGroup> m_StepGroups;

  /** \brief Whether the next step joins the last group, which it does until other content follows. */
  bool m_StepGroupOpen;

  bool m_StepsShown;

  /** \brief Images of the conversation, encoded, as a long conversation collects many screenshots. */
  QHash<int, QByteArray> m_Images;

  /** \brief Never reused, so that the conversation cannot show an image of an earlier one. */
  int m_NextImageId;
};

#endif
