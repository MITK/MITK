/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLogTail.h>

#include <mitkExceptionMacro.h>

#include <algorithm>
#include <fstream>

namespace
{
  /** Paths in messages as UTF-8: path::string() throws on Windows for
   *  characters outside the ANSI code page. */
  std::string ToUtf8(const std::filesystem::path& path)
  {
    const auto utf8 = path.u8string();
    return std::string(utf8.begin(), utf8.end());
  }

  constexpr std::streamoff BlockSize = 64 * 1024;

  /** Position just after the \p n-th '\n' counted from the end of \p text,
   *  or npos if \p text has fewer. */
  std::size_t AfterNthNewlineFromEnd(const std::string& text, std::size_t n)
  {
    auto position = text.size();

    for (std::size_t found = 0; found < n; ++found)
    {
      if (position == 0)
        return std::string::npos;

      position = text.rfind('\n', position - 1);

      if (position == std::string::npos)
        return std::string::npos;
    }

    return position + 1;
  }
}

mitk::LogTail mitk::ReadLogTail(const std::filesystem::path& path, std::size_t maxLines)
{
  std::ifstream file(path, std::ios::binary);

  if (!file.is_open())
    mitkThrow() << "Cannot open log file " << ToUtf8(path);

  file.seekg(0, std::ios::end);
  const std::streamoff fileSize = file.tellg();

  if (fileSize < 0)
    mitkThrow() << "Cannot read log file " << ToUtf8(path);

  LogTail tail;

  if (fileSize == 0 || maxLines == 0)
  {
    tail.truncated = fileSize != 0;
    return tail;
  }

  // Read backwards in blocks until the buffer holds more than maxLines line
  // ends (ignoring the final one) or the whole file: a log copy kept next to
  // a crash dump can be large, while only its end is wanted.
  std::string buffer;
  std::streamoff start = fileSize;
  std::size_t cut = std::string::npos;

  while (start > 0)
  {
    const auto length = std::min(BlockSize, start);
    start -= length;

    std::string block(static_cast<std::size_t>(length), '\0');
    file.seekg(start);

    if (!file.read(block.data(), length))
      mitkThrow() << "Cannot read log file " << ToUtf8(path);

    buffer.insert(0, block);

    const auto lastLineEnd = buffer.back() == '\n' ? buffer.size() - 1 : buffer.size();
    cut = AfterNthNewlineFromEnd(buffer.substr(0, lastLineEnd), maxLines);

    if (cut != std::string::npos)
      break;
  }

  if (cut != std::string::npos)
  {
    buffer.erase(0, cut);
    tail.truncated = true;
  }

  if (buffer.back() != '\n')
    buffer += '\n';

  tail.text.reserve(buffer.size());

  for (std::size_t i = 0; i < buffer.size(); ++i)
  {
    if (buffer[i] == '\r' && i + 1 < buffer.size() && buffer[i + 1] == '\n')
      continue;

    tail.text += buffer[i];
  }

  tail.linesTaken = static_cast<std::size_t>(std::count(tail.text.begin(), tail.text.end(), '\n'));

  return tail;
}
