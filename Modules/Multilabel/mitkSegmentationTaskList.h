/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskList_h
#define mitkSegmentationTaskList_h

#include <mitkBaseData.h>
#include <mitkSegmentationTaskListMacros.h>

#include <MitkMultilabelExports.h>

#include <mitkFileSystem.h>
#include <optional>

namespace mitk
{
  /** \brief A list of segmentation tasks.
   *
   * See \ref MITKSegmentationTaskListsPage for more information.
   */
  class MITKMULTILABEL_EXPORT SegmentationTaskList : public BaseData
  {
  public:
    class MITKMULTILABEL_EXPORT Task
    {
    public:
      Task();
      ~Task();

      void SetDefaults(const Task* defaults);

      mitkSegmentationTaskValueMacro(std::string, Name)
      mitkSegmentationTaskValueMacro(std::string, Description)
      mitkSegmentationTaskValueMacro(fs::path, Image)
      mitkSegmentationTaskValueMacro(fs::path, Segmentation)
      mitkSegmentationTaskValueMacro(std::string, LabelName)
      mitkSegmentationTaskValueMacro(fs::path, LabelNameSuggestions)
      mitkSegmentationTaskValueMacro(fs::path, Preset)
      mitkSegmentationTaskValueMacro(fs::path, Result)
      mitkSegmentationTaskValueMacro(bool, Dynamic)
      mitkSegmentationTaskValueMacro(fs::path, Form)

    private:
      const Task* m_Defaults;
    };

    mitkClassMacro(SegmentationTaskList, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkSegmentationTaskListValueMacro(std::string, Name)
    mitkSegmentationTaskListValueMacro(std::string, Description)
    mitkSegmentationTaskListValueMacro(fs::path, Image)
    mitkSegmentationTaskListValueMacro(fs::path, Segmentation)
    mitkSegmentationTaskListValueMacro(std::string, LabelName)
    mitkSegmentationTaskListValueMacro(fs::path, LabelNameSuggestions)
    mitkSegmentationTaskListValueMacro(fs::path, Preset)
    mitkSegmentationTaskListValueMacro(fs::path, Result)
    mitkSegmentationTaskListValueMacro(bool, Dynamic)
    mitkSegmentationTaskListValueMacro(fs::path, Form)

    size_t GetNumberOfTasks() const;
    size_t AddTask(const Task& subtask);
    const Task* GetTask(size_t index) const;
    Task* GetTask(size_t index);

    const Task& GetDefaults() const;
    void SetDefaults(const Task& defaults);

    bool IsDone() const;
    bool IsDone(size_t index) const;

    fs::path GetInputLocation() const;
    fs::path GetBasePath() const;
    fs::path GetAbsolutePath(const fs::path& path) const;
    fs::path GetInterimPath(const fs::path& path) const;

    void SaveTask(size_t index, const BaseData* segmentation, bool saveAsInterimResult = false);

    std::vector<Task>::const_iterator begin() const;
    std::vector<Task>::const_iterator end() const;

    std::vector<Task>::iterator begin();
    std::vector<Task>::iterator end();

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject*) override;

  protected:
    mitkCloneMacro(Self)

    SegmentationTaskList();
    SegmentationTaskList(const Self& other);
    ~SegmentationTaskList() override;

  private:
    Task m_Defaults;
    std::vector<Task> m_Tasks;
  };
}

#endif
