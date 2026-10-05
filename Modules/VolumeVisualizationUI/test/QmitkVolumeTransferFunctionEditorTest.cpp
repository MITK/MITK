/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkVolumeTransferFunctionEditor.h>

#include "QmitkTestQApplication.h"

#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <ctkDoubleSlider.h>

#include <QCoreApplication>
#include <QElapsedTimer>

#include <functional>

class QmitkVolumeTransferFunctionEditorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkVolumeTransferFunctionEditorTestSuite);
  MITK_TEST(EnableRendering_AdjustingWaitsForAnalysis);
  MITK_TEST(SelectionChange_AnalysisOfLastImageWins);
  MITK_TEST(DisabledEditor_DoesNotAnalyze);
  MITK_TEST(PipelineOutput_AnalyzedOnTheGuiThread);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    EnsureQApplication();
  }

  /** A volume-rendered node whose voxels ramp from 0 to the given maximum. */
  static mitk::DataNode::Pointer CreateNode(short maximum)
  {
    constexpr unsigned int size = 64;

    auto image = mitk::Image::New();
    unsigned int dimensions[3] = {size, size, size};
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);

    {
      mitk::ImageWriteAccessor accessor(image);
      auto *pixels = static_cast<short *>(accessor.GetData());

      for (unsigned int i = 0; i < size * size * size; ++i)
        pixels[i] = static_cast<short>(static_cast<long long>(maximum) * i / (size * size * size - 1));
    }

    auto node = mitk::DataNode::New();
    node->SetData(image);
    node->SetBoolProperty("volumerendering", true);

    return node;
  }

  /** Processes events until the condition holds, for ten seconds at most. */
  static bool ProcessEventsUntil(const std::function<bool()> &done)
  {
    QElapsedTimer timer;
    timer.start();

    while (!done() && timer.elapsed() < 10000)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

    return done();
  }

  static void ProcessEventsFor(int milliseconds)
  {
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < milliseconds)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
  }

  void EnableRendering_AdjustingWaitsForAnalysis()
  {
    QmitkVolumeTransferFunctionEditor editor;
    auto node = CreateNode(1000);

    editor.SetDataNode(node);
    editor.EnsureTransferFunction();

    auto *adjustPanel = editor.findChild<QWidget *>("adjustPresetPanel");
    CPPUNIT_ASSERT(adjustPanel != nullptr);

    CPPUNIT_ASSERT_MESSAGE("A curve can be adjusted before the image is analyzed", !adjustPanel->isEnabled());
    CPPUNIT_ASSERT_MESSAGE("The analysis never made the curve adjustable",
      ProcessEventsUntil([adjustPanel]() { return adjustPanel->isEnabled(); }));

    bool volumeRendering = false;
    node->GetBoolProperty("volumerendering", volumeRendering);
    CPPUNIT_ASSERT_MESSAGE("The analysis switched volume rendering off", volumeRendering);
  }

  void SelectionChange_AnalysisOfLastImageWins()
  {
    QmitkVolumeTransferFunctionEditor editor;

    // Ranges far apart, so that the sliders say whose band they were measured
    // against: the shift slider reaches half the band either side.
    auto narrowNode = CreateNode(100);
    auto wideNode = CreateNode(30000);

    editor.SetDataNode(narrowNode);
    editor.EnsureTransferFunction();

    // One turn of the event loop starts the first analysis, so that switching
    // abandons it rather than preventing it from starting.
    QCoreApplication::processEvents();

    editor.SetDataNode(wideNode);
    editor.EnsureTransferFunction();

    auto *adjustPanel = editor.findChild<QWidget *>("adjustPresetPanel");

    CPPUNIT_ASSERT_MESSAGE("The analysis never made the curve adjustable",
      ProcessEventsUntil([adjustPanel]() { return adjustPanel->isEnabled(); }));

    // Long enough for anything still queued from the first image to arrive.
    ProcessEventsFor(300);

    const auto *shiftSlider = editor.findChild<ctkDoubleSlider *>("opacityShiftSlider");
    CPPUNIT_ASSERT_MESSAGE("The sliders are not measured against the image bound last", shiftSlider->maximum() > 5000.0);
  }

  void DisabledEditor_DoesNotAnalyze()
  {
    QmitkVolumeTransferFunctionEditor editor;
    editor.setEnabled(false);

    auto node = CreateNode(1000);

    editor.SetDataNode(node);
    editor.EnsureTransferFunction();

    auto *adjustPanel = editor.findChild<QWidget *>("adjustPresetPanel");

    ProcessEventsFor(300);

    CPPUNIT_ASSERT_MESSAGE("A disabled editor analyzed its image", !adjustPanel->isEnabledTo(&editor));

    editor.setEnabled(true);

    CPPUNIT_ASSERT_MESSAGE("Enabling the editor did not analyze its image",
      ProcessEventsUntil([adjustPanel]() { return adjustPanel->isEnabled(); }));
  }

  void PipelineOutput_AnalyzedOnTheGuiThread()
  {
    QmitkVolumeTransferFunctionEditor editor;
    auto node = CreateNode(1000);

    auto selector = mitk::ImageTimeSelector::New();
    selector->SetInput(node->GetDataAs<mitk::Image>());
    selector->SetTimeNr(0);
    selector->Update();

    node->SetData(selector->GetOutput());

    editor.SetDataNode(node);
    editor.EnsureTransferFunction();

    // The turn of the event loop that starts the analysis also finishes it,
    // with no worker result to wait for.
    QCoreApplication::processEvents();

    auto *adjustPanel = editor.findChild<QWidget *>("adjustPresetPanel");
    CPPUNIT_ASSERT_MESSAGE("A pipeline output was left to a worker", adjustPanel->isEnabled());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkVolumeTransferFunctionEditor)
