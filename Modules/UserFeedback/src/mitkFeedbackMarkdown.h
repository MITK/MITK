/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFeedbackMarkdown_h
#define mitkFeedbackMarkdown_h

#include <string>

namespace mitk
{
  /** \brief \p text with every '<' that Markdown renderers read as the start
   *  of HTML escaped by a preceding backslash.
   *
   *  A '<' starts HTML if an ASCII letter, '/', '!' or '?' follows it; a '<'
   *  followed by anything else stays. Text in fenced code blocks and in inline
   *  code spans is literal in Markdown and stays unchanged:
   *
   *  - A fence opens at a line that starts, after at most three spaces, with
   *    a run of at least three backticks or three tildes; after a run of
   *    backticks, the line holds no further backtick. It closes at a line
   *    that holds nothing but a run of the same character at least as long
   *    (again after at most three spaces), or at the end of \p text.
   *  - A run of N backticks opens a code span if a run of exactly N backticks
   *    follows on the same line. Without one, the run is literal text.
   *
   *  A backslash in front of '<', a backtick or another backslash already
   *  escapes that character; the pair is copied as it is.
   *
   *  The result is meant for text that is embedded in Markdown. Plain text
   *  such as a mail body or an issue title would show the added backslashes.
   *  Internal to this module. */
  std::string EscapeMarkdownHtml(const std::string& text);
}

#endif
