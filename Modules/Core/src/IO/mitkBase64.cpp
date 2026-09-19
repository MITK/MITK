/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBase64.h>

namespace
{
  constexpr char base64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

std::string mitk::Base64::Encode(std::span<const std::byte> data)
{
  const std::size_t size = data.size();

  std::string result;
  result.reserve((size + 2) / 3 * 4);

  std::size_t i = 0;
  for (; i + 3 <= size; i += 3)
  {
    const auto b0 = std::to_integer<unsigned>(data[i]);
    const auto b1 = std::to_integer<unsigned>(data[i + 1]);
    const auto b2 = std::to_integer<unsigned>(data[i + 2]);

    result += base64Alphabet[b0 >> 2];
    result += base64Alphabet[((b0 & 0x03) << 4) | (b1 >> 4)];
    result += base64Alphabet[((b1 & 0x0f) << 2) | (b2 >> 6)];
    result += base64Alphabet[b2 & 0x3f];
  }

  if (const auto remaining = size - i; remaining != 0)
  {
    const auto b0 = std::to_integer<unsigned>(data[i]);
    result += base64Alphabet[b0 >> 2];

    if (remaining == 1)
    {
      result += base64Alphabet[(b0 & 0x03) << 4];
    }
    else
    {
      const auto b1 = std::to_integer<unsigned>(data[i + 1]);
      result += base64Alphabet[((b0 & 0x03) << 4) | (b1 >> 4)];
      result += base64Alphabet[(b1 & 0x0f) << 2];
    }

    result.append(remaining == 1 ? 2 : 1, '=');
  }

  return result;
}

std::vector<std::byte> mitk::Base64::Decode(const std::string& encoded)
{
  const auto value = [](char c) -> int
  {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
  };

  std::vector<std::byte> result;
  result.reserve(encoded.size() / 4 * 3);

  int buffer = 0;
  int bits = 0;

  for (const char c : encoded)
  {
    const int sextet = value(c);

    if (sextet < 0)
      break; // '=' padding or any non-alphabet character ends the input

    buffer = (buffer << 6) | sextet;
    bits += 6;

    if (bits >= 8)
    {
      bits -= 8;
      result.push_back(static_cast<std::byte>((buffer >> bits) & 0xff));
    }
  }

  return result;
}
