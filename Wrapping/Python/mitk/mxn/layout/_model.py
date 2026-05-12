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

"""Dataclass model for the `mitk.mxn.layout` DSL.

Mirrors the v2.0 schema (`Modules/QtWidgets/resource/mxn-layout-v2.schema.json`)
1:1 in attribute names. All dataclasses are frozen and hashable so callers can
use documents as dict keys, in sets, or memoise builder output.

Storage shapes:
- ``LayoutWindow._links`` is a sorted ``tuple[tuple[str, LinkValue], ...]`` so
  the dataclass remains hashable and equality is order-insensitive.
- ``MxNLayoutDocument._groups`` is a sorted ``tuple[tuple[str, Group], ...]``
  for the same reason.

The friendly ``Mapping`` views are exposed via ``links`` / ``groups``
properties.
"""

from __future__ import annotations

import json
from collections.abc import Iterable, Iterator, Mapping
from dataclasses import dataclass, replace
from itertools import count
from types import MappingProxyType
from typing import (
    Any,
    Literal,
    TYPE_CHECKING,
    Union,
)

from . import _validation as _v
from ._vocabulary import (
    DEFAULT_EDITOR_NAME,
    DEFAULT_GROUP_NAME,
    LAYOUT_VERSION,
    NAMESPACE_DELIMITER,
    LinkDimension,
    ViewDirection,
)


if TYPE_CHECKING:
    from ._selector import MxNWindowSelector


# --------------------------------------------------------------------------- #
# Group
# --------------------------------------------------------------------------- #


@dataclass(frozen=True)
class Group:
    """Synchronization group: a named entry holding shared state for cells
    that link to it. v2.0 carries one property (``select_all``); v3.0 will
    add further per-group properties additively as defaulted fields.
    """

    name: str
    select_all: bool = True


DEFAULT_GROUP: Group = Group(DEFAULT_GROUP_NAME, select_all=True)


# --------------------------------------------------------------------------- #
# Link value type -- v3-modifier-ready from day one
# --------------------------------------------------------------------------- #


@dataclass(frozen=True)
class Link:
    """Per-cell link to a synchronisation group on a single dimension.

    v2 has no modifiers; the bare-string form (``selection="main"``) is
    sufficient and is the canonical wire shape. v3 will additively
    introduce modifier fields (offset, polarity, ...) as defaulted
    optional members; callers compose ``Link(...)`` only when a modifier
    needs to be non-default.
    """

    group: str


# Stored or wire shape of a link: bare group name (v2) or full `Link` (v3+).
LinkValue = Union[str, Link]

# Accepted at API boundaries (`LayoutWindow.create`, selectors): a bare
# group name, a full `Link`, or a `Group` (whose `name` is used as the
# group target). Normalised to `LinkValue` (today: `str`) before storage.
LinkLike = Union[str, Link, Group]


def _normalise_link_value(value: LinkLike) -> str:
    """Coerce an API-boundary ``LinkLike`` to its v2 wire-shape string.

    v2 stores bare group names. ``Link`` instances are unwrapped to their
    ``.group`` member; ``Group`` instances contribute their ``.name``. The
    function validates the resulting group name against the schema pattern
    so a bad token is caught at the cell-construction boundary.

    When v3 dimensions with modifiers land, this helper becomes
    dimension-aware: dimensions that carry modifiers will keep the
    ``Link`` shape on storage; dimensions without modifiers stay as
    strings.
    """
    if isinstance(value, Link):
        group = value.group
    elif isinstance(value, Group):
        group = value.name
    elif isinstance(value, str):
        group = value
    else:
        raise ValueError(
            f"link value must be str, Link, or Group, got {type(value).__name__}"
        )
    _v.validate_group_name(group)
    return group


# --------------------------------------------------------------------------- #
# LayoutWindow
# --------------------------------------------------------------------------- #


_DEFAULT_LINKS: tuple[tuple[str, str], ...] = (
    (LinkDimension.SELECTION.value, DEFAULT_GROUP_NAME),
)


def _coerce_view_direction(value: ViewDirection | str) -> ViewDirection:
    if isinstance(value, ViewDirection):
        return value
    if isinstance(value, str):
        return ViewDirection(value)
    raise ValueError(
        f"view_direction must be ViewDirection or str, got {type(value).__name__}"
    )


@dataclass(frozen=True)
class LayoutWindow:
    """A leaf render-window. Routing identity (``id``) is required and
    schema-pattern checked; ``name`` is an optional free-form display
    label and is omitted from JSON when ``None``.

    Direct dataclass init is the *private* form: it sorts ``_links`` to
    keep equality order-insensitive but does **not** re-run regex / length
    / link checks. Always go through :meth:`create` for input from
    user-controlled sources.
    """

    id: str
    view_direction: ViewDirection
    _links: tuple[tuple[str, LinkValue], ...] = _DEFAULT_LINKS
    name: str | None = None
    size: int = 1

    def __post_init__(self) -> None:
        # Structural normalisation only -- make storage canonical so that
        # equality and hashing are order-insensitive on the link map. No
        # validation; that lives in `create()`.
        # Use object.__setattr__ to write through the frozen dataclass.
        if not isinstance(self._links, tuple):
            object.__setattr__(self, "_links", tuple(self._links))
        if any(self._links[i] > self._links[i + 1] for i in range(len(self._links) - 1)):
            object.__setattr__(self, "_links", tuple(sorted(self._links)))
        if isinstance(self.view_direction, str) and not isinstance(
            self.view_direction, ViewDirection
        ):
            object.__setattr__(
                self, "view_direction", ViewDirection(self.view_direction)
            )

    # ---- friendly construction -------------------------------------------

    @classmethod
    def create(
        cls,
        id: str,
        view_direction: ViewDirection | str,
        *,
        name: str | None = None,
        selection: LinkLike | None = None,
        links: Mapping[LinkDimension | str, LinkLike] | None = None,
        size: int = 1,
    ) -> LayoutWindow:
        """Primary, validating constructor.

        Args:
            id: Routing identity. Must match ``WINDOW_ID_PATTERN`` (qualified form).
            view_direction: ``ViewDirection`` enum or matching string.
            name: Optional human-readable display label. Omitted from JSON
                when ``None``. Tools should pass ``None`` unless authoring
                for humans.
            selection: Common-case shorthand for the only v2 link
                dimension. Pass any ``LinkLike`` (group name string,
                ``Link``, or ``Group``).
            links: Power-user form for setting multiple link dimensions
                at once. Values share the ``LinkLike`` accept-set. Passing
                both ``selection`` and ``links`` raises ``ValueError``.
            size: Splitter weight; only ratios matter. Defaults to 1.
        """
        _v.validate_window_id(id)
        _v.validate_window_name(name)
        _v.validate_size(size)
        view_direction = _coerce_view_direction(view_direction)

        if selection is not None and links is not None:
            raise ValueError(
                "pass either 'selection' or 'links', not both: "
                "'selection' is shorthand for links={LinkDimension.SELECTION: ...}"
            )

        normalised: dict[str, LinkValue] = {}
        if links is not None:
            for raw_key, raw_value in links.items():
                key = _v.validate_dimension_key(raw_key)
                normalised[key] = _normalise_link_value(raw_value)
        else:
            sel_value = DEFAULT_GROUP if selection is None else selection
            normalised[LinkDimension.SELECTION.value] = _normalise_link_value(sel_value)

        if LinkDimension.SELECTION.value not in normalised:
            raise ValueError(
                "links must include the required 'selection' dimension "
                "(schema v2.0 mandates it on every window)"
            )

        # Sorted-tuple storage so equality and hashing are order-insensitive.
        stored = tuple(sorted(normalised.items()))
        return cls(id=id, view_direction=view_direction, _links=stored, name=name, size=size)

    # ---- introspection ---------------------------------------------------

    @property
    def links(self) -> dict[str, LinkValue]:
        """Fresh dict ``{dimension: link_value}``. Mutations to the
        returned dict do not affect the window. Under v2 every value is
        ``str``; v3 may surface ``Link`` for modifier-bearing dimensions.
        """
        return dict(self._links)

    @property
    def selection(self) -> str:
        """Group name linked on ``LinkDimension.SELECTION``.

        v2 has no modifiers on the selection dimension, so the return is
        always a bare string. Raises ``KeyError`` if a malformed window
        somehow lacks the selection link (should be impossible for objects
        produced via :meth:`create`).
        """
        for dim, value in self._links:
            if dim == LinkDimension.SELECTION.value:
                return value if isinstance(value, str) else value.group
        raise KeyError("selection link is missing on this window")

    def linked_to(
        self,
        group: str | Group,
        *,
        dim: LinkDimension | str = LinkDimension.SELECTION,
    ) -> bool:
        """Whether this window is linked to the given group on the given dimension."""
        target = group.name if isinstance(group, Group) else group
        dim_key = _v.validate_dimension_key(dim)
        for d, value in self._links:
            if d == dim_key:
                stored = value if isinstance(value, str) else value.group
                return stored == target
        return False

    # ---- transformations -------------------------------------------------

    def with_link(
        self,
        dimension: LinkDimension | str,
        link: LinkLike,
    ) -> LayoutWindow:
        """Return a copy with the given dimension linked to the given group."""
        dim_key = _v.validate_dimension_key(dimension)
        new_value = _normalise_link_value(link)
        new_links = {k: v for k, v in self._links}
        new_links[dim_key] = new_value
        return replace(self, _links=tuple(sorted(new_links.items())))

    def with_id(self, new_id: str) -> LayoutWindow:
        """Return a copy with a new id.

        WARNING: ids are routing identities; renaming an id that has
        already been served via REST or saved to a session breaks every
        cached reference (persisted session state, REST URL bookmarks,
        per-renderer DataNode property context keys). Safe on in-memory
        documents that have not yet been published.
        """
        _v.validate_window_id(new_id)
        return replace(self, id=new_id)

    def with_name(self, new_name: str | None) -> LayoutWindow:
        """Return a copy with a new display name (None to clear it).

        Free of the caveats on :meth:`with_id` -- ``name`` is pure metadata.
        """
        _v.validate_window_name(new_name)
        return replace(self, name=new_name)

    # ---- repr -----------------------------------------------------------

    def __repr__(self) -> str:
        from ._repr import window_repr

        return window_repr(self)

    def _repr_html_(self) -> str:
        from ._repr import window_html

        return window_html(self)


# --------------------------------------------------------------------------- #
# Split
# --------------------------------------------------------------------------- #


SplitOrientation = Literal["horizontal", "vertical"]


@dataclass(frozen=True)
class Split:
    """An interior splitter dividing its area among children. Maps to QSplitter."""

    orientation: SplitOrientation
    children: tuple[Union["Split", LayoutWindow], ...]
    size: int = 1

    def __post_init__(self) -> None:
        if self.orientation not in ("horizontal", "vertical"):
            raise ValueError(
                f"orientation must be 'horizontal' or 'vertical', got {self.orientation!r}"
            )
        if not isinstance(self.children, tuple):
            object.__setattr__(self, "children", tuple(self.children))
        if not self.children:
            raise ValueError("split.children must contain at least one node (schema minItems: 1)")
        for child in self.children:
            if not isinstance(child, (Split, LayoutWindow)):
                raise ValueError(
                    f"split children must be Split or LayoutWindow, got {type(child).__name__}"
                )
        _v.validate_size(self.size)

    # ---- vararg builders -------------------------------------------------

    @classmethod
    def horizontal(
        cls,
        *children: "Split | LayoutWindow",
        size: int = 1,
    ) -> Split:
        """Build a left-to-right split from a vararg of child nodes."""
        return cls(orientation="horizontal", children=tuple(children), size=size)

    @classmethod
    def vertical(
        cls,
        *children: "Split | LayoutWindow",
        size: int = 1,
    ) -> Split:
        """Build a top-to-bottom split from a vararg of child nodes."""
        return cls(orientation="vertical", children=tuple(children), size=size)

    # ---- tree iteration (pre-order) -------------------------------------

    def windows(self) -> Iterator[LayoutWindow]:
        """Pre-order iterator over the leaf windows."""
        for child in self.children:
            if isinstance(child, LayoutWindow):
                yield child
            else:
                yield from child.windows()

    def window_ids(self) -> list[str]:
        """Pre-order list of window ids (routing identities)."""
        return [w.id for w in self.windows()]

    def splits(self) -> Iterator[Split]:
        """Pre-order iterator over the interior splits, including ``self``."""
        yield self
        for child in self.children:
            if isinstance(child, Split):
                yield from child.splits()

    def find_window(self, id: str) -> LayoutWindow | None:
        """Return the leaf window with the given id, or ``None`` if no
        such window exists in this subtree.
        """
        for window in self.windows():
            if window.id == id:
                return window
        return None

    # ---- selector entry point -------------------------------------------

    @property
    def select_windows(self) -> "MxNWindowSelector[Split]":
        """Entry point for the fluent
        :class:`MxNWindowSelector` filter and bulk-transform API rooted at
        this subtree.
        """
        from ._selector import MxNWindowSelector

        return MxNWindowSelector(self)

    # ---- repr -----------------------------------------------------------

    def __repr__(self) -> str:
        from ._repr import split_repr

        return split_repr(self)

    def _repr_html_(self) -> str:
        from ._repr import split_html

        return split_html(self)


# --------------------------------------------------------------------------- #
# MxNLayoutDocument
# --------------------------------------------------------------------------- #


def _collect_referenced_group_names(root: Split) -> list[str]:
    """Pre-order list of group names referenced by any window's links.

    Order is significant for seeding rules (matches the schema's
    document-order seeding convention) but the function only deduplicates
    based on first encounter.
    """
    seen: dict[str, None] = {}
    for window in root.windows():
        for _, value in window._links:
            name = value if isinstance(value, str) else value.group
            seen.setdefault(name, None)
    return list(seen)


def _materialise_groups(
    root: Split,
    raw: Mapping[str, Group] | Iterable[Group] | None,
) -> tuple[tuple[str, Group], ...]:
    """Resolve ``MxNLayoutDocument.create``'s ``groups`` argument to the
    sorted-tuple storage shape, validating consistency.

    ``None`` -> auto-materialise an entry with property defaults for every
    label referenced from any window's links.
    """
    if raw is None:
        materialised: dict[str, Group] = {}
        for name in _collect_referenced_group_names(root):
            _v.validate_group_name(name)
            materialised[name] = Group(name)
        return tuple(sorted(materialised.items()))

    # `str` / `bytes` are technically `Iterable`, but iterating them
    # yields characters and produces a confusing downstream error.
    # Reject early.
    if isinstance(raw, (str, bytes)):
        raise ValueError(
            f"groups must be Mapping[str, Group], Iterable[Group], or None; "
            f"got {type(raw).__name__}"
        )

    if isinstance(raw, Mapping):
        materialised = {}
        for key, group in raw.items():
            if not isinstance(group, Group):
                raise ValueError(
                    f"groups[{key!r}] must be a Group, got {type(group).__name__}"
                )
            if key != group.name:
                raise ValueError(
                    f"groups key {key!r} does not match Group.name {group.name!r}"
                )
            _v.validate_group_name(key)
            materialised[key] = group
        return tuple(sorted(materialised.items()))

    if isinstance(raw, Iterable):
        materialised = {}
        for group in raw:
            if not isinstance(group, Group):
                raise ValueError(
                    f"groups iterable must yield Group instances, got {type(group).__name__}"
                )
            _v.validate_group_name(group.name)
            if group.name in materialised:
                raise ValueError(f"duplicate group name {group.name!r} in iterable")
            materialised[group.name] = group
        return tuple(sorted(materialised.items()))

    raise ValueError(
        f"groups must be Mapping[str, Group], Iterable[Group], or None; "
        f"got {type(raw).__name__}"
    )


@dataclass(frozen=True)
class MxNLayoutDocument:
    """A complete MxN layout document. In-memory documents are always
    strict: ``groups`` covers every label any window references.
    """

    root: Split
    name: str | None = None
    _groups: tuple[tuple[str, Group], ...] = ()
    version: str = LAYOUT_VERSION

    def __post_init__(self) -> None:
        if not isinstance(self.root, Split):
            raise ValueError(
                f"root must be a Split, got {type(self.root).__name__}"
            )
        if not isinstance(self._groups, tuple):
            object.__setattr__(self, "_groups", tuple(self._groups))

    # ---- friendly construction -------------------------------------------

    @classmethod
    def create(
        cls,
        root: Split,
        *,
        name: str | None = None,
        groups: Mapping[str, Group] | Iterable[Group] | None = None,
        version: str = LAYOUT_VERSION,
    ) -> MxNLayoutDocument:
        """Primary constructor. Always materialises a strict-mode document.

        Args:
            root: Tree root. Always a Split (matches schema).
            name: Optional preset display label.
            groups: Group registry. ``Mapping[str, Group]`` (key must equal
                ``Group.name``), ``Iterable[Group]`` (uniqueness enforced),
                or ``None`` to auto-materialise: every label referenced
                in any window's ``links`` becomes a ``Group`` with
                property defaults.
        """
        if name is not None and (not isinstance(name, str) or len(name) < 1):
            raise ValueError("document name must be a non-empty string when set; pass None to omit")
        _v.validate_layout_version(version)
        groups_storage = _materialise_groups(root, groups)
        doc = cls(root=root, name=name, _groups=groups_storage, version=version)
        doc.validate()
        return doc

    # ---- introspection ---------------------------------------------------

    @property
    def groups(self) -> Mapping[str, Group]:
        """Read-only ``Mapping`` view; mutations to the returned object do
        not affect the document.
        """
        return MappingProxyType(dict(self._groups))

    def windows(self) -> Iterator[LayoutWindow]:
        """Pre-order iterator over every leaf window in the document."""
        return self.root.windows()

    def window_ids(self) -> list[str]:
        """Pre-order list of window ids (the routing identities)."""
        return self.root.window_ids()

    def find_window(self, id: str) -> LayoutWindow | None:
        """Return the leaf window with the given id, or ``None`` if the
        document does not contain one.
        """
        return self.root.find_window(id)

    @property
    def select_windows(self) -> "MxNWindowSelector[MxNLayoutDocument]":
        """Entry point for the fluent
        :class:`MxNWindowSelector` filter and bulk-transform API. Selector
        terminals return a fresh :class:`MxNLayoutDocument` (the original
        is left untouched).
        """
        from ._selector import MxNWindowSelector

        return MxNWindowSelector(self)

    # ---- transformations -------------------------------------------------

    def with_default_ids(
        self,
        *,
        editor_name: str = DEFAULT_EDITOR_NAME,
        bare_id_stem: str = "widget",
        start_index: int = 0,
    ) -> MxNLayoutDocument:
        """Re-number all leaves in pre-order to qualified form.

        Produces ``f"{editor_name}__{bare_id_stem}{i}"`` for ``i`` starting
        at ``start_index``. Display names (``.name``) are preserved; only
        ids change.

        WARNING: same caveat as :meth:`LayoutWindow.with_id` -- re-numbering
        an already-published document invalidates persisted references.
        Intended for use on freshly-built or freshly-loaded documents
        prior to PUT.
        """
        if isinstance(start_index, bool) or not isinstance(start_index, int) or start_index < 0:
            raise ValueError(f"start_index must be a non-negative int, got {start_index!r}")

        counter = count(start_index)

        def renumber(node: Split | LayoutWindow) -> Split | LayoutWindow:
            if isinstance(node, LayoutWindow):
                new_id = f"{editor_name}{NAMESPACE_DELIMITER}{bare_id_stem}{next(counter)}"
                _v.validate_window_id(new_id)
                return replace(node, id=new_id)
            new_children = tuple(renumber(c) for c in node.children)
            return replace(node, children=new_children)

        new_root = renumber(self.root)
        assert isinstance(new_root, Split)
        return replace(self, root=new_root)

    # ---- JSON I/O -------------------------------------------------------

    @classmethod
    def from_json(cls, doc: Mapping[str, Any] | str) -> MxNLayoutDocument:
        """Parse a v2.0 JSON document.

        Accepts either a parsed mapping or a JSON-string. Up-front version
        check raises ``ValueError`` *before* any structural parsing when
        the document's ``version`` is not :data:`LAYOUT_VERSION`. Lazy-mode
        input (no top-level ``groups``) is materialised to strict eagerly
        during parse -- the in-memory document never represents lazy state.
        """
        if isinstance(doc, str):
            try:
                doc = json.loads(doc)
            except json.JSONDecodeError as exc:
                raise ValueError(f"document is not valid JSON: {exc}") from exc
        if not isinstance(doc, Mapping):
            raise ValueError(
                f"document must be a Mapping or JSON string, got {type(doc).__name__}"
            )

        version = doc.get("version")
        if not isinstance(version, str):
            raise ValueError("document is missing a 'version' string field")
        _v.validate_layout_version(version)

        if "root" not in doc:
            raise ValueError("document is missing the required 'root' field")
        root = _split_from_json(doc["root"], is_root=True)

        groups: tuple[tuple[str, Group], ...]
        if "groups" in doc:
            raw_groups = doc["groups"]
            if not isinstance(raw_groups, Mapping):
                raise ValueError("'groups' must be an object/Mapping")
            parsed: dict[str, Group] = {}
            for key, raw_entry in raw_groups.items():
                if not isinstance(raw_entry, Mapping):
                    raise ValueError(
                        f"groups[{key!r}] must be an object, got {type(raw_entry).__name__}"
                    )
                _v.validate_group_name(key)
                select_all = raw_entry.get("select_all", True)
                if not isinstance(select_all, bool):
                    raise ValueError(
                        f"groups[{key!r}].select_all must be a boolean, "
                        f"got {type(select_all).__name__}"
                    )
                parsed[key] = Group(name=key, select_all=select_all)
            groups = tuple(sorted(parsed.items()))
        else:
            # Lazy mode: materialise referenced labels with property defaults.
            groups = _materialise_groups(root, None)

        name = doc.get("name")
        if name is not None and (not isinstance(name, str) or len(name) < 1):
            raise ValueError("'name' must be a non-empty string when present")

        instance = cls(root=root, name=name, _groups=groups, version=version)
        instance.validate()
        return instance

    def to_json(self) -> dict[str, Any]:
        """Emit a strict-mode v2.0 JSON document with stable key ordering.

        Key order follows the schema's example ordering:
        - top-level: ``version``, ``name?``, ``groups``, ``root``
        - per window: ``type``, ``id``, ``name?``, ``view_direction``, ``links``, ``size``
        - per split: ``type``, ``orientation``, ``children``, ``size?``
        ``size`` on the document root is always omitted (no parent splitter).
        """
        out: dict[str, Any] = {"version": self.version}
        if self.name is not None:
            out["name"] = self.name
        out["groups"] = {name: _group_to_json(g) for name, g in self._groups}
        out["root"] = _split_to_json(self.root, is_root=True)
        return out

    # ---- validation -----------------------------------------------------

    def validate(self) -> None:
        """Run cross-document checks on ``self``.

        Raises ``ValueError`` with a message that names the offending
        element. Per-window structural checks are not re-run here --
        those are enforced once in :meth:`LayoutWindow.create`.
        """
        _v.validate_layout_version(self.version)

        seen_ids: set[str] = set()
        for window in self.root.windows():
            if window.id in seen_ids:
                raise ValueError(f"duplicate window id {window.id!r} in document")
            seen_ids.add(window.id)

        group_names = {name for name, _ in self._groups}
        for name in group_names:
            _v.validate_group_name(name)

        for window in self.root.windows():
            for dim, value in window._links:
                target = value if isinstance(value, str) else value.group
                if target not in group_names:
                    raise ValueError(
                        f"window {window.id!r} links {dim!r} to unknown group {target!r}; "
                        f"declare it in groups or rely on auto-materialisation"
                    )

    # ---- repr -----------------------------------------------------------

    def __repr__(self) -> str:
        from ._repr import document_repr

        return document_repr(self)

    def _repr_html_(self) -> str:
        from ._repr import document_html

        return document_html(self)


# --------------------------------------------------------------------------- #
# JSON helpers (free functions to keep parse/emit local to the model module)
# --------------------------------------------------------------------------- #


def _node_from_json(node: Any) -> Split | LayoutWindow:
    if not isinstance(node, Mapping):
        raise ValueError(f"node must be an object, got {type(node).__name__}")
    node_type = node.get("type")
    if node_type == "split":
        return _split_from_json(node, is_root=False)
    if node_type == "window":
        return _window_from_json(node)
    raise ValueError(f"unknown node type {node_type!r}; expected 'split' or 'window'")


def _split_from_json(node: Any, *, is_root: bool) -> Split:
    if not isinstance(node, Mapping):
        raise ValueError(f"split must be an object, got {type(node).__name__}")
    if node.get("type") != "split":
        raise ValueError(
            f"split node must have type='split', got type={node.get('type')!r}"
        )
    orientation = node.get("orientation")
    if orientation not in ("horizontal", "vertical"):
        raise ValueError(
            f"split orientation must be 'horizontal' or 'vertical', got {orientation!r}"
        )
    raw_children = node.get("children")
    if not isinstance(raw_children, list) or not raw_children:
        raise ValueError("split must declare a non-empty 'children' array")
    children = tuple(_node_from_json(c) for c in raw_children)
    size = node.get("size", 1)
    return Split(orientation=orientation, children=children, size=size)


def _window_from_json(node: Mapping[str, Any]) -> LayoutWindow:
    raw_id = node.get("id")
    if not isinstance(raw_id, str):
        raise ValueError("window must declare a string 'id'")
    raw_view = node.get("view_direction")
    if not isinstance(raw_view, str):
        raise ValueError(f"window {raw_id!r} must declare a string 'view_direction'")
    raw_links = node.get("links")
    if not isinstance(raw_links, Mapping):
        raise ValueError(f"window {raw_id!r} must declare a 'links' object")
    raw_name = node.get("name")
    raw_size = node.get("size", 1)
    return LayoutWindow.create(
        id=raw_id,
        view_direction=raw_view,
        name=raw_name,
        links=raw_links,
        size=raw_size,
    )


def _group_to_json(group: Group) -> dict[str, Any]:
    return {"select_all": group.select_all}


def _split_to_json(split: Split, *, is_root: bool) -> dict[str, Any]:
    out: dict[str, Any] = {
        "type": "split",
        "orientation": split.orientation,
        "children": [_node_to_json(c) for c in split.children],
    }
    if not is_root:
        out["size"] = split.size
    return out


def _node_to_json(node: Split | LayoutWindow) -> dict[str, Any]:
    if isinstance(node, Split):
        return _split_to_json(node, is_root=False)
    return _window_to_json(node)


def _window_to_json(window: LayoutWindow) -> dict[str, Any]:
    out: dict[str, Any] = {"type": "window", "id": window.id}
    if window.name is not None:
        out["name"] = window.name
    out["view_direction"] = window.view_direction.value
    out["links"] = {
        dim: (value if isinstance(value, str) else value.group)
        for dim, value in window._links
    }
    out["size"] = window.size
    return out
