# property_view.py: PropertyView implementation for MITK
# Part of WP-15: Property Interface Bindings

from collections.abc import MutableMapping, Iterator
from typing import Any


class PropertyView(MutableMapping):
    """Live, lazy view over an object's properties via IPropertyOwner.

    Not a snapshot -- every operation delegates to the owning object.
    This means the view always reflects the current state of the
    underlying C++ object.

    Thread safety: PropertyView provides no synchronization. If properties
    are modified concurrently (e.g., from a C++ background thread), keys
    obtained during iteration may become stale before they are used.
    Callers that need a consistent snapshot should copy the view into a
    dict: ``snapshot = dict(view.items())``.

    Read vs. write: Reading (``view[key]``, iteration, ``in``) works
    for all properties -- both owned and provided. Writing
    (``view[key] = val``, ``del view[key]``) only works for owned
    properties and raises ``PropertyNotOwnedError`` for read-only
    (provided but not owned) properties. Use
    ``obj.property_is_owned(key)`` to check before writing.
    """

    __slots__ = ("_owner",)

    def __init__(self, owner: Any) -> None:
        self._owner = owner

    def __getitem__(self, key: str) -> Any:
        # get_property returns a coerced Python-native value (str, bool, int, float,
        # (r,g,b) tuple for colours, or a mitk.BaseProperty for unknown types).
        prop = self._owner.get_property(key)
        if prop is None:
            raise KeyError(key)
        return prop

    def __setitem__(self, key: str, value: Any) -> None:
        # Delegates to set_property() -- raises PropertyNotOwnedError
        # if the property is provided read-only (not owned).
        self._owner.set_property(key, value)

    def __delitem__(self, key: str) -> None:
        if self._owner.get_property(key) is None:
            raise KeyError(key)
        # Delegates to remove_property() -- raises PropertyNotOwnedError
        # if the property is provided read-only (not owned).
        self._owner.remove_property(key)

    def __contains__(self, key: object) -> bool:
        if not isinstance(key, str):
            return False
        return self._owner.get_property(key) is not None

    def __iter__(self) -> Iterator[str]:
        return iter(self._owner.property_keys)

    def __len__(self) -> int:
        return len(self._owner.property_keys)

    def __repr__(self) -> str:
        keys = self._owner.property_keys
        if len(keys) <= 5:
            return f"PropertyView({{{', '.join(repr(k) for k in keys)}}})"
        return f"PropertyView({len(keys)} properties)"
