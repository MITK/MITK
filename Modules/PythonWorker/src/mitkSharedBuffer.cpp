/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSharedBuffer.h>

#include <mitkExceptionMacro.h>
#include <mitkImageReadAccessor.h>
#include <mitkMemoryUtilities.h>

#include <itkCommand.h>
#include <itkEventObject.h>

#include <nlohmann/json.hpp>

#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/mapped_region.hpp>

#if defined(_WIN32)
#include <boost/interprocess/windows_shared_memory.hpp>
#include <process.h>
#else
#include <boost/interprocess/shared_memory_object.hpp>
#include <unistd.h>
#endif

#if defined(__linux__)
#include <cerrno>
#include <charconv>
#include <filesystem>
#include <signal.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>

namespace bip = boost::interprocess;

namespace
{
  /* Keeps the memory of a buffer mapped while an image from AsImage() uses it.
   * It lives in the observer list of the image, which itk::Object destroys
   * after the image released its data.
   */
  class MemoryKeeper final : public itk::Command
  {
  public:
    using Self = MemoryKeeper;
    using Pointer = itk::SmartPointer<Self>;
    itkNewMacro(Self);

    std::shared_ptr<const void> Memory;

    void Execute(itk::Object*, const itk::EventObject&) override {}
    void Execute(const itk::Object*, const itk::EventObject&) override {}
  };

  std::string ToNumPyType(const mitk::PixelType& pixelType)
  {
    if (pixelType.GetNumberOfComponents() != 1)
      mitkThrow() << "Shared buffers support single-component pixel types only, not " << pixelType.GetPixelTypeAsString() << '.';

    using Component = itk::IOComponentEnum;
    char kind = '\0';

    switch (pixelType.GetComponentType())
    {
      case Component::UCHAR:
      case Component::USHORT:
      case Component::UINT:
      case Component::ULONG:
      case Component::ULONGLONG:
        kind = 'u';
        break;

      case Component::CHAR:
      case Component::SHORT:
      case Component::INT:
      case Component::LONG:
      case Component::LONGLONG:
        kind = 'i';
        break;

      case Component::FLOAT:
      case Component::DOUBLE:
        kind = 'f';
        break;

      default:
        mitkThrow() << "Shared buffers do not support the component type " << pixelType.GetComponentTypeAsString() << '.';
    }

    return kind + std::to_string(pixelType.GetSize());
  }

  // One time step of an image in NumPy order.
  std::vector<std::size_t> ShapeOf(const mitk::Image* image)
  {
    const auto dimension = std::min(image->GetDimension(), 3u);
    std::vector<std::size_t> shape;

    for (auto i = dimension; i > 0; --i)
      shape.push_back(image->GetDimension(i - 1));

    return shape;
  }

  // Images drop a singleton z dimension, buffers keep it; the memory is the same.
  bool SameExtent(std::vector<std::size_t> imageShape, std::vector<std::size_t> bufferShape)
  {
    while (imageShape.size() < bufferShape.size())
      imageShape.insert(imageShape.begin(), 1);

    return imageShape == bufferShape;
  }

  int CurrentProcessId()
  {
#if defined(_WIN32)
    return _getpid();
#else
    return getpid();
#endif
  }

  // macOS limits names to 31 characters including the leading slash that
  // POSIX shared memory adds.
  std::string GenerateName()
  {
    static std::atomic<unsigned int> counter = 0;
    static std::mt19937 random(std::random_device{}());
    static std::mutex randomMutex;

    unsigned int salt = 0;

    {
      std::lock_guard lock(randomMutex);
      salt = random() & 0xFFFF;
    }

    std::ostringstream name;
    name << std::hex << "mitk" << CurrentProcessId() << '_' << counter++ << '_' << salt;

    return name.str();
  }

#if defined(__linux__)
  /* A segment is named after the process that created it. If that process is
   * gone, MITK died during a call, and the segment would keep its memory in
   * /dev/shm until the next reboot. Segments of running processes stay, and
   * those of other users cannot be removed (owner-only in a sticky
   * directory). macOS offers no way to list POSIX shared memory.
   */
  void RemoveSegmentsOfEndedProcesses()
  {
    try
    {
      for (const auto& entry : std::filesystem::directory_iterator("/dev/shm"))
      {
        const auto name = entry.path().filename().string();
        constexpr std::size_t prefixLength = 4;

        if (!name.starts_with("mitk"))
          continue;

        const auto separator = name.find('_', prefixLength);

        if (separator == std::string::npos)
          continue;

        pid_t pid = 0;
        const auto* first = name.data() + prefixLength;
        const auto* last = name.data() + separator;
        const auto [end, error] = std::from_chars(first, last, pid, 16);

        if (error != std::errc() || end != last || pid <= 0)
          continue;

        if (kill(pid, 0) == -1 && errno == ESRCH)
          bip::shared_memory_object::remove(name.c_str());
      }
    }
    catch (const std::exception&)
    {
      // Best effort: a missing or unreadable /dev/shm leaves nothing to do.
    }
  }
#endif
}

struct mitk::SharedBuffer::Impl
{
  Impl(const PixelType& pixelType, const std::vector<std::size_t>& shape)
    : Type(pixelType),
      Shape(shape),
      Size(std::accumulate(shape.begin(), shape.end(), pixelType.GetSize(), std::multiplies<>()))
  {
    this->DType = ToNumPyType(pixelType);

    if (shape.empty() || this->Size == 0)
      mitkThrow() << "Shared buffers must not be empty.";

#if defined(__linux__)
    static std::once_flag leftoversRemoved;
    std::call_once(leftoversRemoved, RemoveSegmentsOfEndedProcesses);
#endif

    // A stale segment of a crashed process may still hold a name, so try a few.
    for (int attempt = 0; attempt < 8; ++attempt)
    {
      this->Name = GenerateName();

      try
      {
#if defined(_WIN32)
        this->Memory = bip::windows_shared_memory(bip::create_only, this->Name.c_str(), bip::read_write, this->Size);
        this->Published = true;
        this->Region = bip::mapped_region(this->Memory, bip::read_write);
#else
        // Owner-only: Boost's default 0644 would let other local users map
        // the image data while the name is published.
        bip::shared_memory_object memory(bip::create_only, this->Name.c_str(), bip::read_write, bip::permissions(0600));
        this->Published = true;
        memory.truncate(static_cast<bip::offset_t>(this->Size));
        this->Region = bip::mapped_region(memory, bip::read_write);
#endif
        return;
      }
      catch (const bip::interprocess_exception& e)
      {
        if (e.get_error_code() == bip::already_exists_error)
          continue;

        this->Unpublish();
        mitkThrow() << "Could not allocate " << MemoryUtilities::FormatSize(this->Size) << " of shared memory"
#if defined(__linux__)
                    << " in /dev/shm"
#endif
                    << ": " << e.what();
      }
    }

    mitkThrow() << "Could not find an unused name for shared memory.";
  }

  ~Impl()
  {
    this->Unpublish();
  }

  void Unpublish()
  {
#if !defined(_WIN32)
    // On Windows, the kernel removes the memory with its last mapping; a name
    // cannot outlive the processes that use it.
    if (this->Published)
      bip::shared_memory_object::remove(this->Name.c_str());
#endif

    this->Published = false;
  }

  PixelType Type;
  std::vector<std::size_t> Shape;
  std::size_t Size;
  std::string DType;
  std::string Name;
  bool Published = false;
#if defined(_WIN32)
  bip::windows_shared_memory Memory;
#endif
  bip::mapped_region Region;
};

mitk::SharedBuffer::SharedBuffer(const PixelType& pixelType, const std::vector<std::size_t>& shape)
  : m_Impl(std::make_shared<Impl>(pixelType, shape))
{
}

mitk::SharedBuffer::~SharedBuffer()
{
  // An image from AsImage() may keep the memory, but not the name.
  m_Impl->Unpublish();
}

std::unique_ptr<mitk::SharedBuffer> mitk::SharedBuffer::CopyOf(const Image* image, TimeStepType timeStep)
{
  if (image == nullptr)
    mitkThrow() << "Cannot copy a null image into a shared buffer.";

  if (!image->IsVolumeSet(static_cast<int>(timeStep)))
    mitkThrow() << "The image has no data at time step " << timeStep << '.';

  auto buffer = std::make_unique<SharedBuffer>(image->GetPixelType(), ShapeOf(image));

  ImageReadAccessor accessor(image, image->GetVolumeData(static_cast<int>(timeStep)));
  std::memcpy(buffer->GetData(), accessor.GetData(), buffer->GetSize());

  return buffer;
}

mitk::Image::Pointer mitk::SharedBuffer::AsImage(const TimeGeometry& geometry) const
{
  if (geometry.CountTimeSteps() != 1)
    mitkThrow() << "A shared buffer holds one time step, but the geometry has " << geometry.CountTimeSteps() << '.';

  auto image = Image::New();
  image->Initialize(m_Impl->Type, geometry);

  if (!SameExtent(ShapeOf(image), m_Impl->Shape))
    mitkThrow() << "The extent of the geometry does not match the shape of the shared buffer.";

  if (!image->SetImportVolume(this->GetData(), 0, 0, Image::ReferenceMemory))
    mitkThrow() << "Could not use the shared buffer as image volume.";

  auto keeper = MemoryKeeper::New();
  keeper->Memory = m_Impl;
  image->AddObserver(itk::DeleteEvent(), keeper);

  return image;
}

void* mitk::SharedBuffer::GetData() const
{
  return m_Impl->Region.get_address();
}

std::size_t mitk::SharedBuffer::GetSize() const
{
  return m_Impl->Size;
}

const mitk::PixelType& mitk::SharedBuffer::GetPixelType() const
{
  return m_Impl->Type;
}

const std::vector<std::size_t>& mitk::SharedBuffer::GetShape() const
{
  return m_Impl->Shape;
}

nlohmann::json mitk::SharedBuffer::GetDescriptor() const
{
  return {
    { "name", m_Impl->Name },
    { "dtype", m_Impl->DType },
    { "shape", m_Impl->Shape }
  };
}

void mitk::SharedBuffer::Unpublish()
{
  m_Impl->Unpublish();
}

bool mitk::SharedBuffer::IsPublished() const
{
  return m_Impl->Published;
}
