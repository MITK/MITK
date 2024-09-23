# Contributing to MITK

First of all, we appreciate every piece of code or documentation that you contribute to MITK.
There are many ways to contribute:

* Use [GitHub Issues][MITK_issues_page] to submit a feature request, report a bug, or participate in discussions on existing issues. You can find more details in the section [How to File an MITK issue](#how-to-file-an-mitk-issue).

* Submit a [Pull Request][MITK_pr_page] if you want to actively contribute to the MITK code repository. Please review the guidelines on [How to Submit a Pull Request (PR)](#how-to-submit-a-pr) for more details.

# Code of Conduct

Please note that we have a [Code of Conduct][CODE_OF_CONDUCT_path] that you should follow in all your interactions with the project.

# How to File an MITK Issue?

Follow the general instructions on [How to Create a GitHub Issue][github_issues_page].

Please use the MITK public issue template. Be aware that before submitting a PR, you need to create an issue describing the bug or feature and reference the issue number in the PR description.

# Pull Requests (PR) and Related Topics


## How to Submit a PR?


Please follow the instructions on [Creating a Pull Request][github_pr_page]. This corresponds to the Fork & Pull Model described here: [Fork & Pull Model][github_fork_and_pull_page].


Make sure to open an issue along with your PR. This helps us understand which problem the PR solves or which feature it adds. MITK developers will be notified upon submission of your PR.

### How to Write Commit Messages ?

We kindly ask you to follow a few rules and guidelines:

* [Sign off your contribution][sign_off_page] (this is a Git feature).

* Follow the [seven rules of a great Git commit message][seven_commit_rules_page].

* Split your work into easy-to-digest and coherent commits.

* Minimize reformatting of existing code or do it in a separate commit. This makes it easier to review the code and accelerates the processing of the request.

* If you modify code in header files or write documentation, build the doc target to check for any warnings.

### How to Write Code in MITK?

Please follow our [Style Guide][MITK_styleguide_page] if you contribute code to MITK and follow the style of the file that is being edited for the sake of consistency.

### Continuous Integration of PRs

Our Continuous Integration system is triggered every time a PR is created or updated in the repository and will check for issues such as failing unit tests, code quality violations, or integration problems. The results are displayed on the PR page, indicating whether the code passes or fails the tests.
Additionally, nightly tests are conducted, and you will be informed if something in your contributed code is not working.

### How to Merge a PR ?

A PR can be merged once
* the automatic tests for the PR are successful.
* it has been accepted by an MITK developer.

PRs are merged by MITK developers and cannot be merged by external contributors on their own.

### Decision-Making Process

Following a PR submission, the MITK developer team will discuss the corresponding issues and the PR. A developer will be assigned to review the code and may request some changes before acceptance. Good communication is essential throughout this process.

[MITK_issues_page]: https://github.com/MITK/MITK/issues

[MITK_pr_page]: https://github.com/MITK/MITK/pulls

[CODE_OF_CONDUCT_path]: ./CODE_OF_CONDUCT.md

[github_issues_page]: https://docs.github.com/en/issues/tracking-your-work-with-issues/creating-an-issue

[github_pr_page]: https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request

[github_fork_and_pull_page]: https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/getting-started/about-collaborative-development-models#fork-and-pull-model

[sign_off_page]: https://www.mitk.org/wiki/Sign_off_contribution

[seven_commit_rules_page]: https://cbea.ms/git-commit/

[MITK_styleguide_page]: https://docs.mitk.org/nightly/StyleGuideAndNotesPage.html
