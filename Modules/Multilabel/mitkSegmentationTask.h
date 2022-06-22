/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTask_h
#define mitkSegmentationTask_h

#include <mitkBaseData.h>
#include <mitkSegmentationTaskMacros.h>

#include <MitkMultilabelExports.h>

#include <filesystem>
#include <optional>

namespace mitk
{
  class MITKMULTILABEL_EXPORT SegmentationTask : public BaseData
  {
  public:
    class MITKMULTILABEL_EXPORT Subtask
    {
    public:
      Subtask();
      ~Subtask();

      void SetDefaults(const Subtask* defaults);

      mitkSegmentationSubtaskValueMacro(std::string, Name)
      mitkSegmentationSubtaskValueMacro(std::string, Description)
      mitkSegmentationSubtaskValueMacro(std::string, Image)
      mitkSegmentationSubtaskValueMacro(std::string, Segmentation)
      mitkSegmentationSubtaskValueMacro(std::string, LabelName)
      mitkSegmentationSubtaskValueMacro(std::string, Preset)
      mitkSegmentationSubtaskValueMacro(std::string, Result)
      mitkSegmentationSubtaskValueMacro(bool, Dynamic)

    private:
      const Subtask* m_Defaults;
    };

    mitkClassMacro(SegmentationTask, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkSegmentationTaskValueMacro(std::string, Name)
    mitkSegmentationTaskValueMacro(std::string, Description)
    mitkSegmentationTaskValueMacro(std::string, Image)
    mitkSegmentationTaskValueMacro(std::string, Segmentation)
    mitkSegmentationTaskValueMacro(std::string, LabelName)
    mitkSegmentationTaskValueMacro(std::string, Preset)
    mitkSegmentationTaskValueMacro(std::string, Result)
    mitkSegmentationTaskValueMacro(bool, Dynamic)

    size_t GetNumberOfSubtasks() const;
    size_t AddSubtask(const Subtask& subtask);
    const Subtask* GetSubtask(size_t index) const;
    Subtask* GetSubtask(size_t index);

    const Subtask& GetDefaults() const;
    void SetDefaults(const Subtask& defaults);

    bool IsDone() const;
    bool IsDone(size_t index) const;

    std::filesystem::path GetInputLocation() const;
    std::filesystem::path GetBasePath() const;
    std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) const;

    void SaveSubtask(size_t index, const BaseData* segmentation);

    std::vector<Subtask>::const_iterator begin() const;
    std::vector<Subtask>::const_iterator end() const;

    std::vector<Subtask>::iterator begin();
    std::vector<Subtask>::iterator end();

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject*) override;

  protected:
    mitkCloneMacro(Self)

    SegmentationTask();
    SegmentationTask(const Self& other);
    ~SegmentationTask() override;

  private:
    Subtask m_Defaults;
    std::vector<Subtask> m_Subtasks;
  };
}

#endif
