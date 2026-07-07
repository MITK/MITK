# ============================================================================
#
# The Medical Imaging Interaction Toolkit (MITK)
#
# Copyright (c) German Cancer Research Center (DKFZ)
# All rights reserved.
#
# Use of this source code is governed by a 3-clause BSD license that can be
# found in the LICENSE file.
#
# ============================================================================

"""Tests for ``__repr__`` and ``_repr_html_``.

Includes the HTML trust-boundary matrix: every user-controlled string
field passes through `_html_escape`, exercised across all five HTML
metacharacters (`<`, `>`, `&`, `"`, `'`).
"""

from __future__ import annotations

import pytest

from mitk.mxn.layout import (
    Group,
    LayoutWindow,
    MxNLayoutDocument,
    Split,
    ViewDirection,
)


_HTML_METACHARS = ["<", ">", "&", '"', "'"]


def _expected_escapes(text: str) -> list[str]:
    """Each metacharacter present in `text` must appear in escaped form
    in the rendered HTML. The list represents the set of substrings the
    output MUST contain.
    """
    expected = []
    for ch in _HTML_METACHARS:
        if ch in text:
            mapping = {
                "<": "&lt;",
                ">": "&gt;",
                "&": "&amp;",
                '"': "&quot;",
                "'": ("&#x27;", "&#39;"),  # Python `html.escape` uses &#x27;
            }
            entry = mapping[ch]
            expected.append(entry if isinstance(entry, str) else entry[0])
    return expected


class TestPlainTextRepr:
    def test_window_repr_includes_id(self):
        w = LayoutWindow.create("mxn__alpha", "axial")
        assert "mxn__alpha" in repr(w)
        assert "AXIAL" in repr(w)
        assert "name" not in repr(w)

    def test_window_repr_includes_name_when_set(self):
        w = LayoutWindow.create("mxn__alpha", "axial", name="Hello")
        assert "Hello" in repr(w)

    def test_document_repr(self, fresh_doc):
        text = repr(fresh_doc)
        assert "MxNLayoutDocument" in text
        assert "Mixed Fixture" in text
        assert "windows=4" in text


class TestHtmlEscapeMatrix:
    """One test per (field, metachar) cell across every user-controlled
    string field. Spotting a regression that lets an unescaped string slip
    into the HTML output is then immediate.
    """

    @pytest.mark.parametrize("ch", _HTML_METACHARS)
    def test_window_id_escaped(self, ch):
        # Build a window via direct init (skips id pattern check) so we
        # can stuff metacharacters into the id even though the regex
        # would normally reject them.
        w = LayoutWindow(
            id=f"mxn__a{ch}b",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", "main"),),
        )
        html = w._repr_html_()
        for esc in _expected_escapes(f"mxn__a{ch}b"):
            assert esc in html

    @pytest.mark.parametrize("ch", _HTML_METACHARS)
    def test_window_name_escaped(self, ch):
        w = LayoutWindow.create("mxn__a", "axial", name=f"label {ch} text")
        html = w._repr_html_()
        for esc in _expected_escapes(f"label {ch} text"):
            assert esc in html

    @pytest.mark.parametrize("ch", _HTML_METACHARS)
    def test_document_name_escaped(self, ch):
        a = LayoutWindow.create("mxn__a", "axial")
        doc = MxNLayoutDocument.create(root=Split.horizontal(a), name=f"doc {ch} name")
        html = doc._repr_html_()
        for esc in _expected_escapes(f"doc {ch} name"):
            assert esc in html

    @pytest.mark.parametrize("ch", _HTML_METACHARS)
    def test_link_dimension_key_escaped(self, ch):
        # Force-store a free-form dimension key with metacharacters via
        # direct init to exercise the v3 forward-compat code path in the
        # HTML renderer.
        w = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", "main"), (f"dim{ch}", "g")),
        )
        html = w._repr_html_()
        for esc in _expected_escapes(f"dim{ch}"):
            assert esc in html

    @pytest.mark.parametrize("ch", _HTML_METACHARS)
    def test_group_name_escaped_in_doc_summary(self, ch):
        # Direct init for the document so we can store a malformed group
        # name without going through validation.
        a = LayoutWindow(
            id="mxn__a",
            view_direction=ViewDirection.AXIAL,
            _links=(("selection", f"main{ch}"),),
        )
        bad_name = f"main{ch}"
        doc = MxNLayoutDocument(
            root=Split.horizontal(a),
            _groups=((bad_name, Group(name=bad_name, select_all=True)),),
        )
        html = doc._repr_html_()
        for esc in _expected_escapes(bad_name):
            assert esc in html

    def test_html_no_javascript_or_external_refs(self, fresh_doc):
        html = fresh_doc._repr_html_()
        assert "<script" not in html.lower()
        assert "http://" not in html
        assert "https://" not in html
        assert "javascript:" not in html.lower()


class TestHtmlStructure:
    def test_split_html_table_for_horizontal(self):
        from mitk.mxn.layout import grid
        s = grid(1, 3)
        html = s._repr_html_()
        assert "<table" in html
        assert "<tr>" in html
        assert html.count("<td") == 3  # one cell per child

    def test_split_html_table_for_vertical(self):
        from mitk.mxn.layout import grid
        s = grid(3, 1)
        html = s._repr_html_()
        assert "<table" in html
        # Three rows, one cell each.
        assert html.count("<tr>") == 3
