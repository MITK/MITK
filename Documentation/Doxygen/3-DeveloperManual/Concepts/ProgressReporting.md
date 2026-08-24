# Progress Reporting {#ProgressReportingPage}

[TOC]

## Overview

A long-running operation tells the user how far along it is by opening a
task. Each task is independent: it carries its own name and its own step
count, and reporting into one never affects another. In the Workbench, every
running task shows up as a card in the bottom-right corner of the window,
above the status bar.

Tasks are created through mitk::ProgressTask and end when the handle goes out
of scope:

~~~{.cpp}
#include <mitkProgressTask.h>

mitk::ProgressTask task("Loading files", 2 * filesToRead);

for (const auto& file : files)
{
  Read(file);
  task.Progress(2);
}
~~~

Name the operation as the user thinks of it, not the class doing the work:
"Loading files", not "IOUtil".

If the extent of the work is not known in advance, leave the step count out.
The task then reports indeterminate progress.

## There is no Reset()

Nothing resets progress, because there is no shared state left to reset. This
is the point of the design, and it is worth stating explicitly because the API
it replaced needed constant resetting.

The global progress bar this replaced kept one pair of step counters for the
whole process. Every caller added into the same two numbers, so two operations
running at once were summed into one bar, and whichever of them reached its
total first reset the bar for the other. An operation that returned early or
threw left its remaining steps behind for good, which skewed every later
operation in the session. Call sites compensated with a defensive reset on
entry and hand-written rewinds on error paths, and still got it wrong more
often than not.

A task ends with its handle, so early returns, exceptions and rethrows need no
cleanup at all. `Finish()` exists for the rare case of ending a task before its
scope does; calling it twice is harmless.

## Sizing a task

`AddStepsToDo()` grows the total. Doing that once progress has already been
reported makes the bar move **backwards**, because the denominator grows while
the numerator stays where it is. Saving a scene used to declare one step per
node, fill that up, and only then count the files it was about to compress:
with two nodes and seven files the bar read 50 %, then 22 %, then 44 %.

So either declare the whole budget before reporting anything, or, when part of
the work can only be measured later, give each phase a fixed share and map its
own progress into that share:

~~~{.cpp}
constexpr unsigned int SERIALIZATION_SHARE = 50;
constexpr unsigned int COMPRESSION_SHARE = 50;

mitk::ProgressTask task("Saving scene", SERIALIZATION_SHARE + COMPRESSION_SHARE);

// ... after serializing node i of n:
task.SetProgress(SERIALIZATION_SHARE * (i + 1) / n);

// ... after compressing file j of m:
task.SetProgress(SERIALIZATION_SHARE + COMPRESSION_SHARE * (j + 1) / m);
~~~

`SetProgress()` is absolute, which is what makes shares easy to express. Size
the shares by how long each phase takes rather than by how many steps it has,
since what a bar is for is telling the user how much time is left.

Reporting a lower value than before does nothing: the service never takes a
task's progress backwards, so a share that reports itself done out of turn
cannot pull the bar down through the shares before it. That is a backstop, not
a licence to report in any order. It also does not cover `AddStepsToDo()`,
which lowers the percentage by growing the denominator rather than by lowering
the numerator, so the rule above still holds.

## Reporting from a filter

A filter has no way of knowing which user-facing operation it is part of, so it
does not open a task of its own. It takes one:

~~~{.cpp}
filter->SetProgressTask(&task);
~~~

Passing nothing means reporting nothing, which is what command-line tools and
tests want.

A filter announces the steps it is about to contribute rather than leaving its
caller to guess how much work the pipeline it assembled amounts to:

~~~{.cpp}
if (nullptr != m_ProgressTask)
  m_ProgressTask->AddStepsToDo(5);
~~~

Relative step counts across the filters of a pipeline are what make a task
advance roughly with elapsed time, so size them by cost rather than by count.
Mind the rule above, though: filters that announce themselves one after another
as they run will each move the bar backwards a little. Announcing at the time
the task is set, before the pipeline starts, avoids that.

## Operations inside operations

An operation that calls another reported operation should end up as one
notification, not two. There are two ways to arrange that.

Where there is a seam to pass a task through, pass it. Filters take one through
`SetProgressTask()`, and so do file readers and writers: mitk::IFileIO carries a
task, which mitk::IOUtil sets to the share of the work that file accounts for.
A reader can then report in steps of its own, or drive further reads, and all of
it lands in the notification the user is already watching. That is how opening a
scene shows one card rather than one for the file and another for the scene
inside it.

Where there is no seam, quiet the nested operation instead. Saving a scene
writes one file per node through eight different serializers, none of which
knows anything about the scene around it:

~~~{.cpp}
mitk::ProgressTask task("Saving scene", ...);
mitk::IOUtil::QuietProgress quietProgress;
~~~

While that guard exists, `IOUtil::Load()` and `IOUtil::Save()` raise no
notifications **on that thread**, leaving the reporting to whoever set it.

## Threads

A handle belongs to one thread. It is move-only and must not be shared, but
different threads may each own one and report at the same time.

Notification runs without the service lock held, and listeners must neither
block nor report progress of their own.

An operation that runs on the GUI thread never reaches the event loop, so the
notification paints itself synchronously to be visible at all. A bar can still
advance that way, but only while something reports; an operation that blocks in
a single call, as reading an image through ITK does, has no moment at which
anything could be drawn. That is why file IO runs on a worker instead, through
`QmitkRunWithInputBlocked()`, which keeps the event loop turning and discards
user input for the duration.

Running work off the GUI thread is only safe because
mitk::StandaloneDataStorage hands `Add()` and `Remove()` to the thread that owns
the storage; adding a node notifies observers synchronously, and those observers
are rendering and user interface code. Handing over blocks until the task has
run, so a caller waiting on a worker **must keep its event loop turning**.
Waiting the bare way, as `QFuture::waitForFinished()` does, deadlocks;
`QmitkProcessEventsUntil()` is the wait that does not. Queries such as
`GetAll()` are not handed over, only mutations, and an exception thrown by one
that was handed over is carried back to the thread that asked for it rather
than left to unwind the event loop it ran on.

What is not handed over is everything else the worker touches. A writer that
asks a mitk::Image or mitk::Surface for its VTK representation builds that
representation on first access, on whatever thread asks, while the mappers on
the owning thread read the same object. So the data a worker reads is declared
where the worker is started, and `QmitkRunWithInputBlocked()` builds it first,
on the thread that owns it:

~~~{.cpp}
QmitkRunWithInputBlocked([&]() { Save(saveInfos); }, written);
~~~

`QmitkPrebuildVtkRepresentation()` does the same for work that does not run
through that helper.

## What a worker may touch

Declaring the data is one half of a contract that has no other enforcement, so
it is worth stating in full. On a worker, this is allowed:

- Creating new mitk::BaseData. Nothing can be reading what does not exist yet,
  which is why a reader building a node needs no preparation.
- Reading data that was declared to `QmitkRunWithInputBlocked()`.
- `DataStorage::Add()` and `Remove()`, which are handed over.
- Reading data that is not on display at all.

This is not:

- `DataNode::SetData()` on a node that is already in the storage. It is not
  handed over, and it publishes straight into what the mappers render. Compute
  the new data on the worker and set it once the call has returned, which is
  what cropping an image does.
- Writing properties of a node that is on display. `RunWhereTheDataLives()`
  puts such a write where the reads are.
- Mutating mitk::BaseData that is on display, declared or not. Declaring it
  covers a worker *reading* it, not writing to it.

Getting the first of those wrong used to fail intermittently and silently.
Building a VTK representation or replacing a node's data off the owning thread
now says so in the log, which is how a call site that forgot is found. The log
is where it stops, though: nothing refuses the operation.

## Cancellation

A task that intends to honour a cancel request says so when it starts, and
polls:

~~~{.cpp}
mitk::ProgressTask task("Segmenting", 100, true);

while (...)
{
  if (task.IsCancelRequested())
    break;

  task.Progress();
}
~~~

Polling is a lock-free read, so it costs nothing in a tight loop.

Claiming to be cancelable without polling offers the user a cancel that never
happens. The reverse is just as misleading: a cancelable operation that blocks
the GUI thread without ever letting it breathe leaves the user with a cancel
button they cannot press.

An operation that is not a loop of its own but a call into a filter cannot poll
at all, and has to be stopped through whatever the filter offers.
mitk::ToolCommand turns a cancel request into
`itk::ProcessObject::SetAbortGenerateData()` on the filter it observes, which
only a filter that reads that flag can act on. ITK reads it in
`itk::ProgressReporter`, so a filter reporting progress the ITK way aborts by
throwing `itk::ProcessAborted`; one that computes in a single uninterrupted
call, as several of the segmentation filters do, never looks at it and runs to
completion regardless.

So the capability belongs to the filter, and the tool in front of it has to
know whether it has it. mitk::SegWithPreviewTool asks `IsCancelable()`, which
is false unless a tool overrides it, and offers a cancel only if it says yes.
Nothing overrides it at the time of writing, which is why a segmentation
preview shows no cancel button.

Where an abort does arrive, it arrives as an exception, which the caller has to
tell apart from a genuine failure, or the user gets an error message for their
own decision. Whatever the operation wrote before it stopped is not a result
either: leave it on display and the user is looking at half an answer.

## Showing tasks

QmitkProgressNotificationOverlay renders the running tasks. The workbench
window advisors create one per window; a plugin does not need to do anything
to have its tasks shown.

A task is only shown once it has run for about a second, so the many operations
that finish immediately never raise a notification at all. A task that is shown
but has yet to report its first step spins, rather than sitting at zero as if it
were stuck: by the time a card appears the operation has already run for a
second, which is long enough to conclude that no step count is coming.

That second counts from when the task started, not from when the overlay heard
about it. A snapshot reaches the GUI thread only when that thread returns to its
event loop, so whatever keeps it from painting keeps it from the delivery as
well, and a task can already be seconds old by the time its first snapshot is
applied. Measured from the start, such a task is shown the moment it is seen
rather than a second later still.

There is a limit to that, and it is worth knowing. The delay is a timer on the
GUI thread, and the age is checked when a snapshot arrives, so a task that
starts, reports nothing, and holds the GUI thread until it finishes is never
shown at all: the timer cannot fire and no second snapshot ever comes to be
checked. An operation that blocks the GUI thread therefore has to report as it
goes, even coarsely, or run on a worker instead. One that reports nothing and
blocks is exactly the case this whole mechanism cannot help with.

For the GUI-independent side, mitk::IProgressService is a CppMicroServices
service reached through `mitk::CoreServices::GetProgressService()`. Anything
that wants to observe tasks implements mitk::IProgressListener and registers
there. Listeners receive a complete mitk::ProgressTaskInfo snapshot rather than
increments, numbered so that they can discard snapshots a delivery path
reordered, and stamped with the time the task started so that a listener can tell
how long it has been running however late the snapshot arrives.
