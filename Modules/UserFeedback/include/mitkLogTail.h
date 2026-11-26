/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogTail_h
#define mitkLogTail_h

#include <MitkUserFeedbackExports.h>

#include <cstddef>
#include <filesystem>
#include <string>

namespace mitk
{
  /** \brief The last lines of a log file, as offered for a feedback report. */
  struct LogTail
  {
    std::string text;           /**< The lines, newline-terminated, oldest first. */
    std::size_t linesTaken = 0; /**< Number of lines in text. */
    bool truncated = false;     /**< True if any part of the file was omitted. */
  };

  /** \brief Read at most \p maxLines trailing lines of the log file at \p path.
   *
   *  At most \p maxBytes are read from the end of the file. \p maxLines and
   *  \p maxBytes both limit the result: a line the byte cap cuts through is
   *  dropped, except for a single line longer than \p maxBytes, of which the
   *  end is kept. The text can exceed \p maxBytes by one byte, the newline
   *  added to a last line that lacks one.
   *
   *  CRLF line ends are normalized to "\n". The current session's log is
   *  still open for appending by mitk::LogBackend; each message is flushed,
   *  so reading it concurrently ends on a whole line. Throws mitk::Exception
   *  if the file cannot be opened or read. */
  MITKUSERFEEDBACK_EXPORT LogTail ReadLogTail(
    const std::filesystem::path& path, std::size_t maxLines = 1000, std::size_t maxBytes = 512 * 1024);
}

#endif
