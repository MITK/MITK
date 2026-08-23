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
The task then reports indeterminate progress, and can still become determinate
later by calling `AddStepsToDo()` once the extent is known.

## There is no Reset()

Nothing resets progress, because there is no shared state left to reset. This
is the point of the design, and it is worth stating explicitly because the API
it replaced needed constant resetting.

The old mitk::ProgressBar kept one pair of step counters for the whole
process. Every caller added into the same two numbers, so two operations
running at once were summed into one bar, and whichever of them reached its
total first reset the bar for the other. An operation that returned early or
threw left its remaining steps behind for good, which skewed every later
operation in the session. Call sites compensated with a defensive `Reset()` on
entry and hand-written rewinds on error paths, and still got it wrong more
often than not.

A task ends with its handle, so early returns, exceptions and rethrows need no
cleanup at all. `Finish()` exists for the rare case of ending a task before its
scope does; calling it twice is harmless.

## Reporting from a filter

A filter has no way of knowing which user-facing operation it is part of, so it
does not open a task of its own. It takes one:

~~~{.cpp}
filter->SetProgressTask(&task);
~~~

Passing nothing means reporting nothing, which is what command-line tools and
tests want.

A filter announces the steps it is about to contribute when it starts, rather
than leaving its caller to guess:

~~~{.cpp}
if (nullptr != m_ProgressTask)
  m_ProgressTask->AddStepsToDo(5);
~~~

Relative step counts across the filters of a pipeline are what make a task
advance roughly with elapsed time, so size them by cost rather than by count.

## Threads

A handle belongs to one thread. It is move-only and must not be shared, but
different threads may each own one and report at the same time.

Operations that run on the GUI thread never reach the event loop, so the
notification paints itself synchronously to be visible at all. Nothing else is
done from a reporting call: notification runs without the service lock held,
and listeners must neither block nor report progress of their own.

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

mitk::ToolCommand turns a cancel request into
`itk::ProcessObject::SetAbortGenerateData()` on the filter it observes. ITK
reports that abort as an exception, which the caller has to tell apart from a
genuine failure, or the user gets an error message for their own decision.

## Showing tasks

QmitkProgressNotificationOverlay renders the running tasks. The workbench
window advisors create one per window; a plugin does not need to do anything
to have its tasks shown.

A task is only shown once it has run for about a second, so the many
operations that finish immediately never raise a notification at all. A task
that is shown but has yet to report its first step switches to a spinning bar
after another second, rather than sitting at zero as if it were stuck.

For the GUI-independent side, mitk::IProgressService is a CppMicroServices
service reached through `mitk::CoreServices::GetProgressService()`. Anything
that wants to observe tasks implements mitk::IProgressListener and registers
there. Listeners receive a complete mitk::ProgressTaskInfo snapshot rather than
increments, numbered so that they can discard snapshots a delivery path
reordered.
