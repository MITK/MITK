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

      mitkSubtaskValueMacro(Name)
      mitkSubtaskValueMacro(Description)
      mitkSubtaskValueMacro(Image)
      mitkSubtaskValueMacro(Segmentation)
      mitkSubtaskValueMacro(LabelName)
      mitkSubtaskValueMacro(Preset)
      mitkSubtaskValueMacro(Result)

    private:
      const Subtask* m_Defaults;
    };

    mitkClassMacro(SegmentationTask, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkTaskValueMacro(Name)
    mitkTaskValueMacro(Description)
    mitkTaskValueMacro(Image)
    mitkTaskValueMacro(Segmentation)
    mitkTaskValueMacro(LabelName)
    mitkTaskValueMacro(Preset)
    mitkTaskValueMacro(Result)

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
