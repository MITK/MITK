# MITK Workbench REST API Specification {#MITKRESTAPISpec}

**Version:** 1.2.0
**Date:** May 2, 2026

---

## Table of Contents

1. [Overview](#sec-1-overview)
2. [Design Principles](#sec-2-design-principles)
3. [Base URL and Versioning](#sec-3-base-url-and-versioning)
4. [Authentication](#sec-4-authentication)
5. [Common Patterns](#sec-5-common-patterns)
6. [Node Identification Strategy](#sec-6-node-identification-strategy)
7. [Data Transfer Modes](#sec-7-data-transfer-modes)
8. [API Endpoints](#sec-8-api-endpoints)
   - [Discovery and Health](#sec-81-discovery-and-health)
   - [Data Storage](#sec-82-data-storage---nodes)
     - [Nodes](#sec-821-data-storage---nodes)
     - [Node Data Payload](#sec-822-node-data-payload)
     - [Node Children](#sec-823-node-children)
     - [Node Properties](#sec-824-node-properties)
   - [Rendering](#sec-83-rendering)
9. [Error Handling](#sec-9-error-handling)
10. [Examples](#sec-10-examples)
11. [Future Extensions](#sec-11-future-extensions)

---

## 1. Overview {#sec-1-overview}

This document specifies the REST API for MITK Workbench external process integration. The API enables:

- **Python scripts** to control MITK Workbench programmatically
- **Web applications** (e.g., Kaapana) to integrate with MITK Workbench
- **External tools** to interact via standard HTTP

The REST API serves as the **primary control interface** (control plane) as defined in the architecture document. It handles all synchronous CRUD operations on data storage, view configuration, and project management.

### Scope

This specification covers the **Data Storage API** (nodes, data, properties) and the **Rendering API** (render window update and reinit). Future specifications will cover:

- Project Management API
- Task/Async Operations API

### Relationship to Other Communication Channels

| Channel | Purpose | Use Case |
|---------|---------|----------|
| **REST API** | Control plane, CRUD operations | All synchronous operations |
| **ZeroMQ** | Event notifications | State change events (node added, property changed) |
| **Shared Memory** | High-performance data transfer | Large image data between co-located processes |

---

## 2. Design Principles {#sec-2-design-principles}

### 2.1 RESTful Conventions

Based on Architecture Document Appendix B:

1. **Resource-Oriented:** URLs represent resources (nodes, properties)
2. **HTTP Methods:** Standard verbs with precise semantics
3. **Stateless:** Each request is self-contained
4. **Consistent:** Uniform patterns across all endpoints

### 2.2 Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| URL paths | lowercase, hyphens | `/datastorage/nodes` |
| Collections | plural nouns | `/nodes` not `/node` |
| Query parameters | lowercase, dots for namespacing | `filter.name`, `context` |
| JSON fields | snake_case | `node_uid`, `data_type` |

### 2.3 HTTP Methods Semantics

| Method | Semantics | Idempotent |
|--------|-----------|------------|
| `GET` | Retrieve resource(s), no side effects | Yes |
| `POST` | Create new resource | No |
| `PUT` | Replace resource entirely | Yes |
| `PATCH` | Partial update (merge semantics) | Yes |
| `DELETE` | Remove resource | Yes |

---

## 3. Base URL and Versioning {#sec-3-base-url-and-versioning}

### Base URL Structure

```
http(s)://{host}:{port}/api/v{major}
```

**Default:** `http://localhost:8080/api/v1`

### Versioning Strategy

- Major version in URL path: `/api/v1/`, `/api/v2/`
- Minor/patch versions are backwards-compatible (no URL change)
- Breaking changes require new major version
- Deprecation period: minimum 6 months

### Version Discovery

```http
GET /api/v1/info
```

Returns API metadata including supported versions and deprecation notices.

---

## 4. Authentication {#sec-4-authentication}

### Phase 1: API Token Authentication

The server expects an [RFC 6750](https://datatracker.ietf.org/doc/html/rfc6750) Bearer token in the `Authorization` header:

```http
GET /api/v1/datastorage/nodes
Authorization: Bearer {token}
```

When authentication fails, the server responds with `401 Unauthorized` and a `WWW-Authenticate: Bearer` header. When `requireAuth` is disabled (development mode), the header may be omitted.

### Future: OAuth2/OIDC

For Kaapana integration and enterprise deployments.

### Development Mode

Authentication can be disabled for local development:

```yaml
security:
  enabled: false
```

---

## 5. Common Patterns {#sec-5-common-patterns}

### 5.1 Request Headers

| Header | Required | Description |
|--------|----------|-------------|
| `Content-Type` | For POST/PUT/PATCH | `application/json` for metadata |
| `Accept` | Optional | Desired response format |
| `Authorization` | When auth enabled | `Bearer {token}` — see §4 |
| `X-MITK-Transfer-Mode` | `GET /data` responses | `direct`, `file-reference`. For uploads (POST/PUT/PATCH), the transfer mode is inferred from `Content-Type` (`application/json` = file-reference; `application/octet-stream` = direct). |

### 5.2 Standard Response Envelope

All successful responses use a consistent structure:

```json
{
  "data": { ... },
  "meta": {  }
}
```

Collection responses include pagination:

```json
{
  "data": [ ... ],
  "meta": {
    "total_count": 150,
    "returned_count": 50,
    "limit": 50,
    "offset": 0,
    "links": {
      "next": "/api/v1/datastorage/nodes?limit=50&offset=50"
    },
  }
}
```

Pagination links (`links.prev`, `links.next`) are only included when applicable. Links preserve all query parameters from the original request.

### 5.3 Query Parameters

#### Pagination

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `limit` | integer | 50 | Maximum items to return (max: 1000) |
| `offset` | integer | 0 | Number of items to skip |

#### Filtering

**System field filters** use direct parameters:

```
?data_type=Image&parent_uid=node-001
```

**Property filters** use the `filter.` prefix:

```
?filter.visible=true&filter.name=CT*
```

**Combined example:**
```http
GET /api/v1/datastorage/nodes?data_type=Image&filter.visible=true
```

Supported filter operators:
- Exact match: `filter.name=CT_Scan`
- Not equal: `filter.visible!=true`
- Wildcard `*` (zero or more characters): `filter.name=CT*`, `filter.name=*Scan`, `filter.name=CT*2024*Scan`
- Wildcard `?` (exactly one character): `filter.name=Node_0?`, `filter.name=????`

**Supported system field filters:**
- `data_type` — Unqualified MITK class name as returned by `BaseData::GetNameOfClass()` (e.g., `Image`, `LabelSetImage`, `Surface` — note: no `mitk::` prefix)
- `parent_uid` — Filter by parent (use `null` for top-level nodes)
- `path` — Path-based lookup

#### Property Filter Context

When filtering by properties, you can specify the context and scope for property evaluation:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context for property lookup |
| `property_scope` | string | `all` | `all`, `node`, `data` — which property level to check |

```http
GET /api/v1/datastorage/nodes?filter.visible=true&context=stdmulti.widget0&property_scope=node
```

#### Sorting

```
?sort=name          # Ascending by name
?sort=-timestamp    # Descending by timestamp (minus prefix)
```

#### Field Selection

```
?fields=uid,name,data_type    # Return only specified fields
```

### 5.4 REST Conventions and Deviations

The API follows standard REST conventions with the following deliberate deviations:

**404 for "operation not applicable" on a valid resource.** When a sub-resource path is well-formed and the addressed resource exists, but the requested operation does not apply to that particular resource variant, the API returns **404 `UNSUPPORTED_OPERATION`** rather than 405 / 409 / 422. Example: `GET /rendering/editors/stdmulti/windows/3d/selected-slice` returns 404 `UNSUPPORTED_OPERATION` because slice navigation is meaningless on the 3D window. Rationale: the conceptual sub-resource (`selected-slice` of the `3d` window) does not exist for that variant, so 404 communicates absence consistently with `RENDER_WINDOW_NOT_FOUND`. Clients should treat any 404 from a per-window sub-resource as "this variant does not expose this operation" and inspect `error.code` to disambiguate from genuine "window not found".

**DELETE returns 200 with body, not 204 No Content.** `DELETE /datastorage/nodes/{uid}` and `DELETE /datastorage/nodes/{uid}/properties/{name}` return **200** with a JSON body rather than the conventional 204. Rationale: the response body carries client-useful information (node deletes return `deleted_uid` and `deleted_children`; property deletes echo the resolved `meta.property_scope`) that lets the client confirm what was actually removed without an extra round-trip. Clients should not assume any DELETE in this API returns 204.

---

## 6. Node Identification Strategy {#sec-6-node-identification-strategy}

### 6.1 Dual Identification Approach

MITK nodes can be identified by:

1. **UID (Canonical):** Unique identifier assigned by MITK, used in all URL paths
2. **Path (Query):** Human-readable tree path, used as a filter/lookup mechanism

### 6.2 UID as Canonical Identifier

All resource URLs use UID:

```
/api/v1/datastorage/nodes/{uid}
```

**UID Characteristics:**
- Assigned by MITK Workbench when node is created
- Immutable for the lifetime of the node
- Globally unique within the data storage
- Format: Implementation-defined (e.g., UUID, internal ID)

### 6.3 Path as Query Mechanism

Path-based lookup via query parameter:

```http
GET /api/v1/datastorage/nodes?path=/CT_Scan/Segmentation
```

**Path Characteristics:**
- Built from node names in the hierarchy
- Human-readable, matches Data Manager display
- May resolve to multiple nodes (names are not unique)
- Format: `/ParentName/ChildName/...`

### 6.4 Path Query Response

Path queries **always return an array**, even for single matches. Use `total_count` to determine the number of results.

```http
GET /api/v1/datastorage/nodes?path=/CT_Scan/Segmentation
```

**Response (single match):**

```json
{
  "data": [
    {
      "uid": "abc-123",
      "name": "Segmentation",
      "path": "/CT_Scan/Segmentation",
      "parent_uid": "node-001",
      "data_type": "LabelSetImage",
      "timestamp": 1705661400
    }
  ],
  "meta": {
    "total_count": 1,
    "path_query": "/CT_Scan/Segmentation"
  }
}
```

**Response (multiple matches):**

```json
{
  "data": [
    {
      "uid": "abc-123",
      "name": "Segmentation",
      "path": "/CT_Scan/Segmentation",
      "parent_uid": "node-001",
      "data_type": "LabelSetImage",
      "timestamp": 1705661400
    },
    {
      "uid": "def-456",
      "name": "Segmentation",
      "path": "/CT_Scan/Segmentation",
      "parent_uid": "node-001",
      "data_type": "LabelSetImage",
      "timestamp": 1705661700
    }
  ],
  "meta": {
    "total_count": 2,
    "path_query": "/CT_Scan/Segmentation"
  }
}
```

### 6.5 Node Response Structure

Node responses contain **system information only**. The `name` property is included as a convenience shortcut due to its fundamental importance. All other properties must be retrieved via the `/properties` endpoint.

```json
{
  "uid": "abc-123-def-456",
  "name": "CT_Scan",
  "path": "/CT_Scan",
  "parent_uid": null,
  "data_type": "Image",
  "children_count": 2,
  "timestamp": 1705661400
}
```

**System fields:**

| Field | Type | Description |
|-------|------|-------------|
| `uid` | string | Unique identifier (canonical) |
| `name` | string | Node name (shortcut to name property) |
| `path` | string | Full path in hierarchy |
| `parent_uid` | string | Parent node UID (null for top-level) |
| `data_type` | string | Unqualified MITK data class name (e.g., `Image`, `LabelSetImage`, `Surface`) |
| `children_count` | integer | Number of child nodes |
| `timestamp` | integer | ITK modified time (ever-increasing integer within session; higher values are newer) |

> **Note:** The `timestamp` is an ITK modification time — an ever-increasing integer valid only within the current session. Higher values indicate more recent modifications. It is not persisted across sessions and cannot be compared across different sessions.

---

## 7. Data Transfer Modes {#sec-7-data-transfer-modes}

Large data (images, meshes, segmentations) can be transferred via three modes, supporting different performance requirements.

### 7.1 Mode Selection

Clients indicate preferred mode via header:

```http
X-MITK-Transfer-Mode: direct | file-reference
```

If omitted, server chooses based on data size and client location.

### 7.2 Mode: Direct Payload

Data transferred directly in HTTP body.

**Request:**
```http
GET /api/v1/datastorage/nodes/{uid}/data
Accept: application/octet-stream
X-MITK-Transfer-Mode: direct
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="CT_Scan.nrrd"
Content-Length: 134217728
```

Response body contains the raw binary data.

**Format detection:**
- The `Content-Disposition` header provides the filename with extension, which is sufficient for MITK I/O format detection
- The actual format depends on the serializer used by MITK and may vary based on data type and configuration

**Default formats (typical):**
- Images: `nrrd`
- Segmentations: `nrrd` (MITK multi-label format)
- Surfaces: `stl` or `vtp`

> **Note:** The exact file format cannot be guaranteed by the server as it depends on the MITK I/O serialization pipeline. Clients should rely on the file extension for format detection.

**Use case:** Small to medium data, remote clients, simplicity.

### 7.3 Mode: File Reference {#sec-73-mode-file-reference}

Server writes data to file, returns path and metadata.

**Request:**
```http
GET /api/v1/datastorage/nodes/{uid}/data
Accept: application/json
X-MITK-Transfer-Mode: file-reference
```

**Response:**
```json
{
  "transfer": {
    "mode": "file-reference",
    "size_bytes": 125829120,
    "file_path": "/tmp/mitk-data/export-abc123.nrrd",
    "directory_path": "/tmp/mitk-data"
  },
  "data_metadata": {
    "dimensions": [512, 512, 256],
    "spacing": [0.5, 0.5, 1.0],
    "origin": [0.0, 0.0, 0.0],
    "pixel_type": "int16"
  }
}
```

> **Note:** The file format can be determined from the file extension in `file_path`. The `directory_path` field provides the parent directory of the exported file.

**Use case:** Large data, co-located processes, existing file I/O workflows.

**File lifetime and per-client quota:** Exported files are stored in a server-managed temporary directory. Each client (identified by its IP address) may hold at most **N active file-reference results simultaneously** (default: 5; configurable by the server operator). When a client exceeds this limit, the server automatically deletes its *oldest* active temp directory to make room for the new one. Files are additionally cleaned up when the server stops.

> **Implication for clients:** Fetch or copy the file referenced by `transfer.file_path` before issuing more than N subsequent file-reference requests from the same IP. Clients from different IP addresses do not affect each other's quotas.

> **Note:** The `data_metadata` section provides informative metadata about the data content. The exact fields depend on the data type (image, surface, point set, etc.). The file content is the authoritative source.

> **Note:** Checksum support (`checksum` field in `transfer` section) is planned for a future version.

### 7.4 Response Structure

All JSON data transfer responses follow the same structure with two clearly separated sections:

| Section | Purpose | Content |
|---------|---------|---------|
| `transfer` | Transfer-specific information | Mode, format, location, size |
| `data_metadata` | Data-specific information | Type-dependent fields (e.g., dimensions, spacing for images) |

The `transfer` section varies by mode. The `data_metadata` section is consistent across all modes but its contents depend on the data type:

- **Images**: `dimensions`, `spacing`, `origin`, `pixel_type`
- **Surfaces**: `points_count`, `cells_count`, `bounds`
- **Point Sets**: `points_count`, `bounds`

### 7.5 Mode Negotiation

If server cannot fulfill requested mode:

```http
HTTP/1.1 406 Not Acceptable

{
  "error": {
    "code": "TRANSFER_MODE_NOT_AVAILABLE",
    "message": "Shared memory not available for remote clients",
    "available_modes": ["direct", "file-reference"]
  }
}
```

### 7.6 Upload Data Transfer

For `POST` and `PUT` operations, same modes apply:

**Direct upload:**
```http
POST /api/v1/datastorage/nodes
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="my_image.nrrd"
X-MITK-Data-Format: nrrd

<binary data>
```

**File reference upload:**
```http
POST /api/v1/datastorage/nodes
Content-Type: application/json

{
  "name": "New_Image",
  "transfer": {
	"mode": "file-reference"
    "file_path": "/data/images/ct_scan.nrrd"
  }
}
```

**File ownership:** When uploading via file-reference mode, the sender (client) always retains ownership of the source file. The server reads the file but does not delete it. The sender can assume the file is no longer needed by the server after receiving the response, and is responsible for cleanup if desired.

The server accepts any file format supported by MITK I/O. Format detection is based on file extension.

---

## 8. API Endpoints {#sec-8-api-endpoints}

### 8.1 Discovery and Health {#sec-81-discovery-and-health}

#### GET /api/v1/info

API information and capabilities.

**Response:**
```json
{
  "data": {
    "name": "MITK Workbench REST API",
    "api_version": "v1",
    "mitk_version": "2025.12.99",
    "capabilities": {
      "transfer_modes": ["direct", "file-reference"],
      "authentication": ["api-token"]
    },
    "documentation_url": "https://docs.mitk.org/nightly/MITKRESTAPISpec.html"
  }
}
```

The `documentation_url` is constructed from the running MITK version: released builds use `https://docs.mitk.org/<MAJOR>.<MINOR>/MITKRESTAPISpec.html`, while development builds (patch level 99) use `https://docs.mitk.org/nightly/MITKRESTAPISpec.html`.

> **Note:** The `capabilities.events` section with `zeromq_endpoint` is planned for a future version when ZeroMQ event notification support is implemented. Currently, this field is not included in the response.

#### Interactive Documentation

The server also exposes the OpenAPI document and a Swagger UI for live exploration. These resources are not part of the versioned contract — they are discovery/tooling routes that the conformance test deliberately skips when cross-checking spec coverage:

- `GET /api/v1/openapi.json` — raw OpenAPI 3 document
- `GET /api/v1/docs/` — interactive Swagger UI (with associated CSS/JS assets under `/api/v1/docs/`)

#### GET /api/v1/health

Health check endpoint.

**Response:**
```json
{
  "data": {
    "status": "healthy",
    "checks": {
      "datastorage": "ok"
    },
    "uptime_seconds": 3600
  }
}
```

---

#### GET /api/v1/config/file-access

Returns the current file access configuration. Clients can use this to discover which file paths are permitted for file-reference transfers.

**Response (unrestricted):**
```json
{
  "data": {
    "mode": "unrestricted",
    "restrictions_active": false,
    "max_active_temp_dirs_per_ip": 5
  }
}
```

**Response (restricted):**
```json
{
  "data": {
    "mode": "allowed-directories",
    "restrictions_active": true,
    "allowed_paths": ["/data/images", "/data/exports"],
    "max_active_temp_dirs_per_ip": 5
  }
}
```

**Response fields:**

| Field | Type | Description |
|-------|------|-------------|
| `mode` | string | `unrestricted` or `allowed-directories` |
| `restrictions_active` | boolean | Whether file path restrictions are currently enforced |
| `allowed_paths` | array | Allowed directory paths (only present when restrictions are active) |
| `max_active_temp_dirs_per_ip` | integer (always present) | Maximum number of concurrently active file-reference temp directories the server will keep per client IP before evicting the oldest. Clients should use this value to avoid unexpected eviction of open file references. Default: 5. |

---

### 8.2 Data Storage {#sec-82-data-storage---nodes}
#### 8.2.1 Nodes {#sec-821-data-storage---nodes}

##### GET /api/v1/datastorage/nodes

List all nodes in the data storage. Always returns an array.

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `hierarchy` | string | `all` | `all` or `toplevel` (root nodes only) |
| `path` | string | — | Path-based lookup |
| `data_type` | string | — | Filter by MITK data type |
| `parent_uid` | string | — | Filter by parent (use `null` for root) |
| `filter.{property}` | string | — | Filter by property value (supports `*` and `?` wildcards) |
| `context` | string | — | Renderer context for property filters |
| `property_scope` | string | `all` | Property scope for filters: `all`, `node`, `data` |
| `fields` | string | — | Comma-separated fields to include |
| `sort` | string | — | Sort field (prefix `-` for descending) |
| `limit` | integer | 50 | Max results (max: 1000) |
| `offset` | integer | 0 | Pagination offset |

**Example Request:**
```http
GET /api/v1/datastorage/nodes?hierarchy=toplevel&data_type=Image&filter.visible=true
```

**Response:**
```json
{
  "data": [
    {
      "uid": "node-001",
      "name": "CT_Scan",
      "path": "/CT_Scan",
      "parent_uid": null,
      "data_type": "Image",
      "children_count": 2,
      "timestamp": 1705659600
    },
    {
      "uid": "node-002",
      "name": "MRI_T1",
      "path": "/MRI_T1",
      "parent_uid": null,
      "data_type": "Image",
      "children_count": 0,
      "timestamp": 1705659900
    }
  ],
  "meta": {
    "total_count": 2,
    "returned_count": 2,
    "limit": 50,
    "offset": 0
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `total_count` | integer | Total number of nodes matching the filters (before pagination) |
| `returned_count` | integer | Actual number of nodes returned in this response |
| `limit` | integer | The limit used for pagination |
| `offset` | integer | The offset used for pagination |
| `links` | object | Pagination links (only present when applicable) |
| `links.prev` | string | URL for previous page (only present when offset > 0) |
| `links.next` | string | URL for next page (only present when more items exist) |
| `context` | string/null | The context used for property filters (null for default) |
| `property_scope` | string | The property scope used for filters (`all`, `node`, `data`) |
| `filters` | object | Applied filters (only present when filters were used) |
| `sort` | object | Applied sorting (only present when sorting was requested) |
| `fields` | array | Field selection (only present when fields were limited) |

**Example with pagination links:**
```json
{
  "data": [...],
  "meta": {
    "total_count": 150,
    "returned_count": 50,
    "limit": 50,
    "offset": 50,
    "links": {
      "prev": "/api/v1/datastorage/nodes?data_type=Image&limit=50&offset=0",
      "next": "/api/v1/datastorage/nodes?data_type=Image&limit=50&offset=100"
    }
  }
}
```

---

##### POST /api/v1/datastorage/nodes

Create a new top-level node.

**Request (file reference):**
```http
POST /api/v1/datastorage/nodes
Content-Type: application/json

{
  "name": "CT_Scan",
  "transfer": {
    "file_path": "/data/patient001/ct.nrrd"
  }
}
```

**Request (direct upload):**
```http
POST /api/v1/datastorage/nodes?name=CT_Scan
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="ct.nrrd"
X-MITK-Data-Format: nrrd

<binary data: raw bytes of the NRRD file>
```

> **Note:** The request body contains the raw binary content of the file. The file format is determined from the filename in the `Content-Disposition` header or the `X-MITK-Data-Format` header.

**Response (201 Created):**
```json
{
  "data": {
    "uid": "node-003",
    "name": "CT_Scan",
    "path": "/CT_Scan",
    "parent_uid": null,
    "data_type": "Image",
    "children_count": 0,
    "timestamp": 1705661400
  },
  "meta": {
    "location": "/api/v1/datastorage/nodes/node-003"
  }
}
```

**Response with failed properties:**

If some properties in the request could not be deserialized, the node is still created but the response includes a `failed_properties` field listing the property keys that failed:

```json
{
  "data": {
    "uid": "node-003",
    "name": "CT_Scan",
    "path": "/CT_Scan",
    "parent_uid": null,
    "data_type": null,
    "children_count": 0,
    "timestamp": 1705661400
  },
  "meta": {
    "location": "/api/v1/datastorage/nodes/node-003",
    "failed_properties": ["invalid_prop1", "invalid_prop2"]
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `location` | string | URI of the created node |
| `failed_properties` | array | (Optional) Property keys that failed to deserialize. Logged as warnings server-side. |
| `warning` | string | (Optional) Warning message if the loaded file contained multiple data objects. Only the first data object is assigned to the node; additional objects are discarded. |

> **Content-Type:** Use `application/json` for file-reference mode (JSON body with optional `transfer.file_path`) or `application/octet-stream` for direct transfer mode (binary body). Omitting Content-Type entirely creates a node with default values only (no data, auto-generated name).

**Status codes:**

| Status | Code | Description |
|--------|------|-------------|
| 201 | — | Node created |
| 400 | `INVALID_REQUEST` | Malformed JSON body or invalid parameters |
| 406 | `TRANSFER_MODE_NOT_AVAILABLE` | Requested transfer mode not supported |
| 415 | `UNSUPPORTED_FORMAT` | Content-Type or data format not supported |
| 422 | `FILE_NOT_FOUND` / `FILE_READ_ERROR` | Referenced file path could not be loaded |
| 500 | `SERIALIZATION_ERROR` / `INTERNAL_ERROR` | Server error while creating the node |
| 503 | `DATASTORAGE_NOT_AVAILABLE` | DataStorage not connected |

---

##### GET /api/v1/datastorage/nodes/{uid}

Get node system information.

**Response:**
```json
{
  "data": {
    "uid": "node-001",
    "name": "CT_Scan",
    "path": "/CT_Scan",
    "parent_uid": null,
    "data_type": "Image",
    "children_count": 2,
    "timestamp": 1705661400
  },
  "meta": {
    "available_contexts": [null, "stdmulti.widget0", "stdmulti.widget1"]
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `available_contexts` | array | List of property contexts available for this node. Contains `null` for the default context and strings for named renderer contexts. |

> **Note:** To retrieve node properties (except `name`), use `GET /api/v1/datastorage/nodes/{uid}/properties`.

---

##### PATCH /api/v1/datastorage/nodes/{uid}

Update node structure (reparenting only).

**Request:**
```http
PATCH /api/v1/datastorage/nodes/{uid}
Content-Type: application/json

{
  "parent_uid": "node-010"
}
```

**Response (200 OK):**
```json
{
  "data": {
    "uid": "node-001",
    "name": "CT_Scan",
    "path": "/Parent_Node/CT_Scan",
    "parent_uid": "node-010",
    "data_type": "Image",
    "children_count": 2,
    "timestamp": 1705663500
  }
}
```

> **Note:** This endpoint does not return a `meta` object. The response data contains the updated node state.

> **Note:** To rename a node, use `PUT /api/v1/datastorage/nodes/{uid}/properties/name`.

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Body is not valid JSON, or `parent_uid` has an invalid format |
| 404 | `NODE_NOT_FOUND` | No node exists with the given UID or parent UID |
| 409 | `CIRCULAR_HIERARCHY_REFERENCE` | The target parent is a descendant of the node being reparented |
| 500 | `INTERNAL_ERROR` | Unexpected server error |
| 503 | `DATASTORAGE_NOT_AVAILABLE` | No DataStorage is currently connected |

---

##### DELETE /api/v1/datastorage/nodes/{uid}

Remove node from data storage.

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `recursive` | boolean | `false` | Also delete children |

**Request:**
```http
DELETE /api/v1/datastorage/nodes/node-001?recursive=true
```

**Response (200 OK):**
```json
{
  "data": {
    "deleted_uid": "node-001",
    "deleted_children": ["node-001-child-1", "node-001-child-2"]
  }
}
```

> **Note:** This endpoint does not return a `meta` object. The `deleted_children` field is only present when `recursive=true` and children were deleted.

**Error (409 Conflict) - Has children but recursive=false:**
```json
{
  "error": {
    "code": "NODE_HAS_CHILDREN",
    "message": "Node has 2 children. Use ?recursive=true to delete.",
    "children_count": 2
  }
}
```

---

#### 8.2.2 Node Data Payload {#sec-822-node-data-payload}

##### GET /api/v1/datastorage/nodes/{uid}/data

Retrieve the actual data (image, mesh, etc.) of a node.

**Headers:**

| Header | Values | Description |
|--------|--------|-------------|
| `Accept` | `application/octet-stream`, `application/json` | Response format |
| `X-MITK-Transfer-Mode` | `direct`, `file-reference` | Transfer mode |

**Example (direct download):**
```http
GET /api/v1/datastorage/nodes/node-001/data
Accept: application/octet-stream
X-MITK-Transfer-Mode: direct
```

**Response:**
```http
HTTP/1.1 200 OK
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="CT_Scan.nrrd"
Content-Length: 134217728
```

Response body contains raw binary data.

**Example (file reference):**
```http
GET /api/v1/datastorage/nodes/node-001/data
Accept: application/json
X-MITK-Transfer-Mode: file-reference
```

**Response:** See [Section 7.3](#sec-73-mode-file-reference)

---

##### PUT /api/v1/datastorage/nodes/{uid}/data

Replace or set node data. This endpoint can be used on nodes that currently have no data (`data_type` is `null`), allowing you to create an empty node first and add data later.

**Request Headers:**

| Header | Required | Description |
|--------|----------|-------------|
| `Content-Type` | Yes | `application/json` for file-reference, `application/octet-stream` for direct |
| `Content-Disposition` | For direct | Filename with extension for format detection |
| `X-MITK-Data-Format` | No | Explicit format identifier (optional) |

**Request (file reference):**
```http
PUT /api/v1/datastorage/nodes/node-001/data
Content-Type: application/json

{
  "transfer": {
    "file_path": "/data/patient001/ct_updated.nrrd"
  }
}
```

**File ownership:** When uploading via file-reference mode, the sender retains ownership of the source file. The server reads the file but does not delete it. The sender can assume the file is no longer needed by the server after receiving the response.

**Request (direct upload):**
```http
PUT /api/v1/datastorage/nodes/node-001/data
Content-Type: application/octet-stream
Content-Disposition: attachment; filename="ct_updated.nrrd"

<binary data: raw bytes of the NRRD file>
```

> **Note:** The request body contains the raw binary content of the file. The file format is determined from the filename in the `Content-Disposition` header.

**Response (200 OK):**
```json
{
  "data": {
    "uid": "node-001",
    "name": "CT_Scan",
    "path": "/CT_Scan",
    "parent_uid": null,
    "data_type": "Image",
    "children_count": 0,
    "timestamp": 1705663200
  }
}
```

**Response with warning (when file contains multiple data objects):**
```json
{
  "data": {
    "uid": "node-001",
    "name": "CT_Scan",
    "path": "/CT_Scan",
    "parent_uid": null,
    "data_type": "Image",
    "children_count": 0,
    "timestamp": 1705663200
  },
  "meta": {
    "warning": "File contained 3 data objects. Only the first one was assigned to the node. 2 data object(s) were discarded."
  }
}
```

> **Note:** The server accepts any file format supported by MITK I/O. Format detection is based on file extension (from `Content-Disposition` header or `file_path`).

> **Error 415:** Requests with missing or unsupported `Content-Type` header will receive a 415 Unsupported Media Type response. Use `application/json` for file-reference mode or `application/octet-stream` for direct transfer mode.

---

#### 8.2.3 Node Children {#sec-823-node-children}

##### GET /api/v1/datastorage/nodes/{uid}/children

List child nodes of a given node. Always returns an array.

**Query Parameters:**

Same filtering/pagination as `GET /datastorage/nodes`.

**Response:**
```json
{
  "data": [
    {
      "uid": "node-001-seg",
      "name": "Segmentation",
      "path": "/CT_Scan/Segmentation",
      "parent_uid": "node-001",
      "data_type": "LabelSetImage",
      "children_count": 0,
      "timestamp": 1705661400
    },
    {
      "uid": "node-001-mask",
      "name": "Mask",
      "path": "/CT_Scan/Mask",
      "parent_uid": "node-001",
      "data_type": "Image",
      "children_count": 0,
      "timestamp": 1705661700
    }
  ],
  "meta": {
    "parent_uid": "node-001",
    "total_count": 2,
    "returned_count": 2,
    "limit": 50,
    "offset": 0
  }
}
```

**Response meta fields:**

Same pagination fields as `GET /datastorage/nodes` (including `links` with `prev`/`next` when applicable), plus:

| Field | Type | Description |
|-------|------|-------------|
| `parent_uid` | string | UID of the parent node |

---

##### POST /api/v1/datastorage/nodes/{uid}/children

Add a new child node to the specified parent.

**Request:**
```http
POST /api/v1/datastorage/nodes/node-001/children
Content-Type: application/json

{
  "name": "Segmentation",
  "transfer": {
    "file_path": "/data/patient001/segmentation.nrrd"
  }
}
```

**Response (201 Created):**
```json
{
  "data": {
    "uid": "node-001-seg",
    "name": "Segmentation",
    "path": "/CT_Scan/Segmentation",
    "parent_uid": "node-001",
    "data_type": "LabelSetImage",
    "children_count": 0,
    "timestamp": 1705663200
  },
  "meta": {
    "location": "/api/v1/datastorage/nodes/node-001-seg"
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `location` | string | URI of the created child node |
| `failed_properties` | array | (Optional) Property keys that failed to deserialize. Logged as warnings server-side. |
| `warning` | string | (Optional) Warning message if the loaded file contained multiple data objects. Only the first data object is assigned to the node; additional objects are discarded. |

> **Content-Type:** Use `application/json` for file-reference mode (JSON body with optional `transfer.file_path`) or `application/octet-stream` for direct transfer mode (binary body). Requests with unsupported Content-Type values will receive a 415 Unsupported Media Type response. Omitting Content-Type entirely creates a child node with default values only (no data, auto-generated name).

---

#### 8.2.4 Node Properties {#sec-824-node-properties}

##### GET /api/v1/datastorage/nodes/{uid}/properties

Get all properties of a node.

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context (e.g., `stdmulti.widget0`) |
| `property_scope` | string | `all` | `all`, `node`, `data` |
| `content` | boolean | `true` | Include property values |
| `names` | string | (all) | Comma-separated property names to return |

**Example (full content):**
```http
GET /api/v1/datastorage/nodes/node-001/properties?property_scope=all
```

**Response:**
```json
{
  "data": {
    "properties": {
      "name": "CT_Scan",
      "visible": true,
      "layer": 0,
      "opacity": 1.0,
      "color": {
        "type": "ColorProperty",
        "value": [1.0, 1.0, 1.0]
      },
      "levelwindow": {
        "type": "LevelWindowProperty",
        "value": {
          "level": 40.0,
          "window": 400.0
        }
      },
      "DICOM.0010.0010": {
        "type": "TemporoSpatialStringProperty",
        "value": {
          "values": [
            {"t": 0, "z": 0, "value": "Anonymous"}
          ]
        }
      }
    }
  },
  "meta": {
    "count": 7,
    "property_scope": "all",
    "context": null,
    "available_contexts": [null, "stdmulti.widget0", "stdmulti.widget1"]
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `count` | integer | Number of properties returned |
| `property_scope` | string | The scope used for property lookup (`all`, `node`, `data`) |
| `context` | string/null | The context used for property lookup (null for default). The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `available_contexts` | array | List of property contexts available for this node |

**Example (names only):**
```http
GET /api/v1/datastorage/nodes/node-001/properties?content=false
```

**Response:**
```json
{
  "data": {
    "properties": [
      "name",
      "visible",
      "layer",
      "opacity",
      "color",
      "levelwindow",
      "DICOM.0010.0010"
    ]
  },
  "meta": {
    "count": 7,
    "property_scope": "all",
    "context": null,
    "available_contexts": [null, "stdmulti.widget0", "stdmulti.widget1"]
  }
}
```

**Example (specific properties):**
```http
GET /api/v1/datastorage/nodes/node-001/properties?names=visible,opacity,color
```

**Response:**
```json
{
  "data": {
    "properties": {
      "visible": true,
      "opacity": 1.0,
      "color": {
        "type": "ColorProperty",
        "value": [1.0, 1.0, 1.0]
      }
    }
  },
  "meta": {
    "count": 3,
    "property_scope": "all",
    "context": null
  }
}
```

**Example (renderer-specific):**
```http
GET /api/v1/datastorage/nodes/node-001/properties?context=stdmulti.widget0&property_scope=node
```

---

##### PUT /api/v1/datastorage/nodes/{uid}/properties

Replace all properties of a node (full replacement).

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context. The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `property_scope` | string | `node` | `node`, `data` |

**Request:**
```http
PUT /api/v1/datastorage/nodes/node-001/properties
Content-Type: application/json

{
  "visible": true,
  "opacity": 0.5,
  "layer": 1,
  "color": {
    "type": "ColorProperty",
    "value": [1.0, 0.0, 0.0]
  }
}
```

**Response (200 OK):**
```json
{
  "data": {
    "replaced": ["visible", "opacity", "layer", "color"],
    "removed": ["levelwindow", "custom_property"]
  },
  "meta": {
    "node_uid": "node-001",
    "property_scope": "node",
    "context": null
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `node_uid` | string | UID of the node |
| `property_scope` | string | The scope used for property replacement (`node`, `data`) |
| `context` | string/null | The context used for property replacement (null for default) |

> **Warning:** `PUT` replaces all properties in the specified scope. Properties not included in the request will be removed (except protected system properties). Use `PATCH` for partial updates.

---

##### PATCH /api/v1/datastorage/nodes/{uid}/properties

Update multiple properties at once (merge semantics).

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `property_scope` | string | `node` | `node`, `data` |

**Request:**
```http
PATCH /api/v1/datastorage/nodes/node-001/properties
Content-Type: application/json

{
  "visible": true,
  "opacity": 0.5,
  "color": {
    "type": "ColorProperty",
    "value": [1.0, 0.0, 0.0]
  }
}
```

**Response (200 OK):**
```json
{
  "data": {
    "updated": ["visible", "opacity", "color"],
    "failed": []
  },
  "meta": {
    "node_uid": "node-001",
    "property_scope": "node",
    "context": null
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `node_uid` | string | UID of the node |
| `property_scope` | string | The scope used for property update (`node`, `data`) |
| `context` | string/null | The context used for property update (null for default) |

**Response with partial failure:**
```json
{
  "data": {
    "updated": ["visible", "opacity"],
    "failed": [
      {
        "property": "color",
        "error": "Invalid color value: expected array of 3 floats"
      }
    ]
  },
  "meta": {
    "node_uid": "node-001"
  }
}
```

---

##### GET /api/v1/datastorage/nodes/{uid}/properties/{property_key}

Get a single property value.

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `property_scope` | string | `all` | `all`, `node`, `data` |

> **Note:** When `property_scope=all` (default), lookup order is `node` → `data` (node takes precedence, matching MITK behavior).

**Response (simple property):**
```json
{
  "data": {
    "opacity": 1.0
  },
  "meta": {
    "node_uid": "node-001",
    "property_key": "opacity",
    "property_scope": "all",
    "context": null
  }
}
```

**Response (complex property):**
```json
{
  "data": {
    "DICOM.0010.0010": {
      "value": [
        {"t": 0, "z": 0, "value": "Anonymous"}
      ],
      "type": "TemporoSpatialStringProperty"
    }
  },
  "meta": {
    "node_uid": "node-001",
    "property_key": "DICOM.0010.0010",
    "property_scope": "all",
    "context": null
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `node_uid` | string | UID of the node |
| `property_key` | string | Name of the property |
| `property_scope` | string | The scope used for property lookup (`all`, `node`, `data`) |
| `context` | string/null | The context used for property lookup (null for default) |

---

##### PUT /api/v1/datastorage/nodes/{uid}/properties/{property_key}

Set a single property value (create or replace).

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `property_scope` | string | `node` | `node`, `data` |

**Request (simple property):**
```http
PUT /api/v1/datastorage/nodes/node-001/properties/opacity
Content-Type: application/json

{
  "value": 0.75
}
```

**Request (complex property):**
```http
PUT /api/v1/datastorage/nodes/node-001/properties/color
Content-Type: application/json

{
  "type": "ColorProperty",
  "value": [1.0, 0.5, 0.0]
}
```

**Response (200 OK):**
```json
{
  "data": {
    "opacity": 0.75
  },
  "meta": {
    "node_uid": "node-001",
    "property_key": "opacity",
    "replaced": true
  }
}
```

**Response meta fields:**

| Field | Type | Description |
|-------|------|-------------|
| `node_uid` | string | UID of the node |
| `property_key` | string | Name of the property that was set |
| `replaced` | boolean | `true` if an existing property was replaced, `false` if a new property was created |

---

##### DELETE /api/v1/datastorage/nodes/{uid}/properties/{property_key}

Remove a property from a node.

**Query Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `context` | string | (none) | Renderer context. The context is only relevant/valid at property_scope `node` and will be ignored at data scope (always default). |
| `property_scope` | string | `node` | `node`, `data` |

**Response (200 OK):**
```json
{
  "data": {
    "deleted": "custom_property"
  }
}
```

> **Note:** This endpoint does not return a `meta` object.

**Error (400 Bad Request) - Protected property:**
```json
{
  "error": {
    "code": "PROPERTY_PROTECTED",
    "message": "Property 'name' cannot be deleted"
  }
}
```

---

### 8.3 Rendering {#sec-83-rendering}

Rendering endpoints control how MITK Workbench render windows refresh and orient themselves. They are deliberately separate from data and property endpoints: callers can batch multiple mutations (upload data, set properties) and then trigger a single render update, avoiding per-change flicker.

All rendering calls are dispatched to the main/UI thread by the server — callers do not need to account for threading.

The `/rendering/editors/stdmulti/...` hierarchy addresses the StdMultiWidget editor and its render windows (axial / sagittal / coronal / 3d). The `mxn` editor alias is also reported by `GET /rendering/editors` — its `active` flag reflects whether an MxN multi-widget editor instance is currently open in the workbench. The symmetric `/rendering/editors/mxn/...` hierarchy adds layout management, per-cell camera, per-cell selected slice, per-cell selected position, and editor- / per-window screenshots. See §8.3.1 below for an orientation on how the MxN REST surface is structured.

#### POST /api/v1/rendering/update

Request a redraw of all registered render windows.

**Request body (optional, `application/json`):**

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `type` | string | `"all"` | Which windows to update: `"all"`, `"2d"` (2D windows only), `"3d"` (3D windows only) |

**Example (update all windows):**
```http
POST /api/v1/rendering/update
```

**Example (update only 2D windows):**
```http
POST /api/v1/rendering/update
Content-Type: application/json

{"type": "2d"}
```

**Response: 204 No Content**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Body is present but not valid JSON, or `type` is not `"all"`, `"2d"`, or `"3d"` |
| 422 | `RENDERING_ERROR` | An unexpected error occurred in the rendering framework |

---

#### POST /api/v1/rendering/reinit

Fit all render windows to the bounding box of all currently visible data (global reinit), or to the bounding geometry of one or more specific nodes when UIDs are supplied. Equivalent to clicking the global reinit button in the Workbench toolbar.

Three operating modes:
- **No body**: global reinit — fits all render windows to the bounding box of all visible data.
- **`{"uids": ["node-001"]}`**: single-node reinit — fits render windows to that node's geometry.
- **`{"uids": ["node-001", "node-002"]}`**: multi-node reinit — fits render windows to the combined bounding geometry of all listed nodes.

Every UID in the `uids` array must identify an existing node with data and a valid time geometry; the first failure returns an error.

**Request body (optional, `application/json`):**

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `uids` | array of string (minItems: 1) | (none) | When provided, fit views to the bounding geometry of the specified nodes |

**Example (global reinit — fit all views to all visible data):**
```http
POST /api/v1/rendering/reinit
```

**Example (single-node reinit — fit views to a specific node):**
```http
POST /api/v1/rendering/reinit
Content-Type: application/json

{"uids": ["node-001"]}
```

**Example (multi-node reinit — fit views to the combined bounding box of several nodes):**
```http
POST /api/v1/rendering/reinit
Content-Type: application/json

{"uids": ["node-001", "node-002"]}
```

**Response: 204 No Content**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Body is not valid JSON, or `uids` is present but not a non-empty string array |
| 404 | `NODE_NOT_FOUND` | No node exists with one of the given UIDs |
| 422 | `NO_DATA` | A listed node exists but has no data object attached |
| 422 | `NO_GEOMETRY` | A listed node has data but the data has no usable time geometry |
| 422 | `RENDERING_ERROR` | An unexpected error occurred in the rendering framework |
| 503 | `DATASTORAGE_NOT_AVAILABLE` | No DataStorage is currently connected |

#### GET /api/v1/rendering/selected-position

Returns the current crosshair position via `IRenderWindowPart::GetSelectedPosition()` on the StdMultiWidgetEditor, and the world-space axis-aligned bounding box (AABB) from the reinit geometry (TimeNavigationController input world time geometry).

Requires the Qt workbench plugin to be running (503 `RENDER_WINDOW_NOT_AVAILABLE` otherwise) and the StdMultiWidgetEditor to be open (503 `EDITOR_NOT_ACTIVE` otherwise). If no input geometry is available, `bounds.min_position` and `bounds.max_position` are `null`.

**Response 200 (`application/json`):**

```json
{
  "position": [10.0, 20.0, 30.0],
  "bounds": {
    "min_position": [-50.0, -50.0, -50.0],
    "max_position": [50.0, 50.0, 50.0]
  }
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered (headless mode or Qt workbench plugin not loaded) |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open in the workbench |
| 500 | `INTERNAL_ERROR` | Unexpected error reading crosshair or geometry state |

---

#### PUT /api/v1/rendering/selected-position

Moves the crosshair to the given 3D world position via `IRenderWindowPart::SetSelectedPosition()` on the StdMultiWidgetEditor, which updates all synchronized views.

Requires the Qt workbench plugin to be running (503 `RENDER_WINDOW_NOT_AVAILABLE` otherwise) and the StdMultiWidgetEditor to be open (503 `EDITOR_NOT_ACTIVE` otherwise).

**Request body (required, `application/json`):**

| Field | Type | Description |
|-------|------|-------------|
| `position` | array of number (length 3) | New crosshair position in world coordinates |

**Example:**
```http
PUT /api/v1/rendering/selected-position
Content-Type: application/json

{"position": [10.0, 20.0, 30.0]}
```

**Response: 204 No Content**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Body missing, not valid JSON, `position` field absent, or not an array of exactly 3 numbers |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered (headless mode or Qt workbench plugin not loaded) |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open in the workbench |
| 500 | `INTERNAL_ERROR` | Unexpected error setting crosshair position |

---

#### GET /api/v1/rendering/selected-time

Returns the current time step and time point from the global `TimeNavigationController`, together with the time bounds.

**Response 200 (`application/json`):**

```json
{
  "timepoint_ms": 1500.0,
  "timestep": 3,
  "bounds": {
    "min_timepoint_ms": 0.0,
    "max_timepoint_ms": 4500.0,
    "steps": 10
  }
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 500 | `INTERNAL_ERROR` | Unexpected error reading time navigation state |
| 503 | `TIME_NAVIGATION_NOT_AVAILABLE` | TimeNavigationController is not available |

---

#### PUT /api/v1/rendering/selected-time

Sets the active time step or time point in the global `TimeNavigationController`. Exactly one of `timepoint_ms` or `timestep` must be present.

**Request body (required, `application/json`):**

| Field | Type | Description |
|-------|------|-------------|
| `timepoint_ms` | number | Target time point in milliseconds |
| `timestep` | integer (>= 0) | Target time step index (zero-based) |

Provide exactly one of the two fields — providing both or neither is an error.

**Example (by time point):**
```http
PUT /api/v1/rendering/selected-time
Content-Type: application/json

{"timepoint_ms": 1500.0}
```

**Example (by time step):**
```http
PUT /api/v1/rendering/selected-time
Content-Type: application/json

{"timestep": 3}
```

**Response: 204 No Content**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Body missing, not valid JSON, both fields present, neither field present, wrong type, or negative timestep |
| 422 | `RENDERING_ERROR` | Resolved time step is outside the available range `[0, steps)`, or unexpected rendering framework error |
| 500 | `TIME_STEPPER_NOT_AVAILABLE` | Time stepper is not available |
| 503 | `TIME_NAVIGATION_NOT_AVAILABLE` | TimeNavigationController is not available |

Out-of-range inputs (a `timestep` greater than or equal to `steps`, or a
`timepoint_ms` outside the available time bounds resolving to such a step)
are rejected with **422** rather than silently clamped, so clients can
distinguish a user/programming error from a successful set.

---

#### GET /api/v1/rendering/screenshot

Captures a screenshot of the active application window. Requires the Qt workbench plugin to be running and the screenshot provider to be connected.

**Query parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `format` | string | `"png"` | Image encoding: `"png"` or `"jpeg"` |
| `width` | integer | (native) | Output width in pixels; must be between 1 and 8192 |
| `height` | integer | (native) | Output height in pixels; must be between 1 and 8192 |

`width` and `height` must always be provided together.

**Example (native resolution PNG):**
```http
GET /api/v1/rendering/screenshot
```

**Example (scaled JPEG):**
```http
GET /api/v1/rendering/screenshot?format=jpeg&width=1280&height=720
```

**Response: 200** with binary body (`Content-Type: image/png` or `image/jpeg`)

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Invalid `format` value, non-positive or non-integer `width`/`height`, dimensions exceeding 8192, or only one of `width`/`height` provided |
| 500 | `INTERNAL_ERROR` | Screenshot capture failed |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No screenshot provider connected (headless mode or Qt plugin not loaded) |

---

#### GET /api/v1/rendering/editors

Lists all known editor aliases with their current activity state. The alias list is always complete even if no editor is open, so clients can discover capabilities without polling sub-resources.

**Response 200 (`application/json`):**

```json
[
  { "alias": "stdmulti", "plugin_id": "org.mitk.editors.stdmultiwidget", "active": true  },
  { "alias": "mxn",      "plugin_id": "org.mitk.editors.mxnmultiwidget", "active": false }
]
```

| Field | Type | Description |
|-------|------|-------------|
| `alias` | string | Stable short alias used in URL paths (`stdmulti`, `mxn`) |
| `plugin_id` | string | Berry editor plugin id |
| `active` | boolean | `true` if an editor instance is currently open |

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No editor list provider registered (headless / plugin not loaded) |
| 500 | `INTERNAL_ERROR` | Unexpected error walking the workbench |

---

#### GET /api/v1/rendering/editors/stdmulti

Returns metadata about the StdMultiWidget editor, including its current window ids. `alias` is the sole editor discriminator — no separate `type` field is reported.

**Response 200 (`application/json`):**

```json
{
  "alias": "stdmulti",
  "plugin_id": "org.mitk.editors.stdmultiwidget",
  "active": true,
  "windows": ["axial", "sagittal", "coronal", "3d"]
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered (headless / plugin not loaded) |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/stdmulti/screenshot

Captures the StdMultiWidget editor canvas (all four render windows together, no side panels).

Query parameters, request body, response content-types and shared error shapes are **identical** to `GET /api/v1/rendering/screenshot`. Only the capture surface differs. Any future change to the global screenshot contract must be applied to this endpoint in the same commit.

**Editor-specific error (in addition to the inherited set):**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |

---

#### GET /api/v1/rendering/editors/stdmulti/windows

Lists the StdMultiWidget render windows. `view_direction` is the persisted slot mapping (axial / sagittal / coronal); it is omitted for the 3D window and is *not* a live-orientation read. Under swivel mode or node-initialised geometry the live plane is not guaranteed to match `view_direction` — live orientation, when needed, is derivable from the window's `/camera`.

**Response 200 (`application/json`):**

```json
[
  { "id": "axial",    "kind": "2d", "view_direction": "axial" },
  { "id": "sagittal", "kind": "2d", "view_direction": "sagittal" },
  { "id": "coronal",  "kind": "2d", "view_direction": "coronal" },
  { "id": "3d",       "kind": "3d" }
]
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Render window id (engine-fixed slot token; URL segment for sub-resources) |
| `kind` | string | `"2d"` or `"3d"` — drives which sub-resources apply |
| `view_direction` | string \| absent | Anatomical plane the slot renders. Present for 2D windows; omitted for the 3D window. For StdMulti the value matches `id` by construction; the field is provided so generic clients can read `view_direction` uniformly across editor types. |

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No window list provider |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/stdmulti/windows/{id}

Per-window summary.

**Path parameter:** `name` ∈ {`axial`, `sagittal`, `coronal`, `3d`}. Any other value returns 404 `RENDER_WINDOW_NOT_FOUND` at the controller layer before any bridge dispatch.

**Response 200 (`application/json`) — 2D window:**

```json
{ "id": "axial", "kind": "2d", "view_direction": "axial", "has_camera": true, "has_selected_slice": true }
```

**Response 200 — 3D window:**

```json
{ "id": "3d", "kind": "3d", "has_camera": true, "has_selected_slice": false }
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Echo of the path parameter |
| `kind` | string | `"2d"` or `"3d"` |
| `view_direction` | string \| absent | Anatomical plane the slot renders. Present for 2D windows; omitted for the 3D window. |
| `has_camera` | boolean | Always `true` in StdMulti |
| `has_selected_slice` | boolean | `true` for 2D, `false` for `3d` |

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | `{id}` is not a known StdMulti window |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered (headless / plugin not loaded) |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/stdmulti/windows/{id}/camera

Returns the camera state of the addressed render window. 2D windows (axial/sagittal/coronal) include `parallel_scale` and omit `perspective_angle`; the 3D window does the inverse.

**Path parameter:** `name` ∈ {`axial`, `sagittal`, `coronal`, `3d`}.

**Response 200 — 2D window:**

```json
{
  "position":       [127.5,  83.2, 200.0],
  "focal_point":    [127.5,  83.2,  45.0],
  "view_up":        [0.0,    1.0,   0.0],
  "parallel_scale": 120.0
}
```

**Response 200 — 3D window:**

```json
{
  "position":          [200.0, 200.0, 200.0],
  "focal_point":       [127.5,  83.2,  45.0],
  "view_up":           [0.0,    0.0,   1.0],
  "perspective_angle": 30.0
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered (headless / plugin not loaded) |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/stdmulti/windows/{id}/camera

Partial update. Any subset of the applicable fields may be sent; unspecified fields are left unchanged. `standard_view` programs the underlying `mitk::CameraController`, while explicit pose fields (`position`, `focal_point`, `view_up`) bypass it and write the raw `vtkCamera`. Combining the two would leave the controller's internal "standard view" memo inconsistent with the actual pose, so the combination is rejected with 400 `INVALID_REQUEST`. `standard_view` may still be combined with `parallel_scale` or `perspective_angle`. Only the addressed window is refreshed; coupled crosshair/slice updates on the sibling 2D windows happen through their own UI events. World coordinates are not range-checked.

**Request body (`application/json`, at least one field required):**

| Field | Type | Applies to | Notes |
|-------|------|------------|-------|
| `position` | number[3] | 2D, 3D | |
| `focal_point` | number[3] | 2D, 3D | |
| `view_up` | number[3] | 2D, 3D | |
| `parallel_scale` | number > 0 | 2D | Orthographic zoom |
| `perspective_angle` | number in (0, 180) | 3D | Vertical FOV in degrees |
| `standard_view` | string | 2D, 3D | One of `anterior`, `posterior`, `left`, `right`, `cranial`, `caudal` |

**Examples:**

```http
PUT /api/v1/rendering/editors/stdmulti/windows/axial/camera
Content-Type: application/json

{ "standard_view": "anterior", "parallel_scale": 120.0 }
```

```http
PUT /api/v1/rendering/editors/stdmulti/windows/3d/camera
Content-Type: application/json

{ "perspective_angle": 45.0 }
```

**Response: 204 No Content.**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Invalid JSON; empty body; unknown field; wrong type or length; 2D-only field on 3D (or vice versa); unknown `standard_view`; `standard_view` combined with `position`/`focal_point`/`view_up`; non-positive `parallel_scale`; `perspective_angle` out of range |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` |
| 422 | `RENDERING_ERROR` | MITK rendering framework raised an error while applying the patch |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/stdmulti/windows/{id}/selected-slice

Returns the currently selected step, the world position of the slice center, and the scene navigation bounds. Not applicable to the 3D window.

**Path parameter:** `name` ∈ {`axial`, `sagittal`, `coronal`} (`3d` returns 404 `UNSUPPORTED_OPERATION`).

**Response 200 (`application/json`):**

```json
{
  "step": 45,
  "position": [127.5, 83.2, 45.0],
  "bounds": {
    "steps": 90,
    "min_position": [0.0, 0.0, 0.0],
    "max_position": [255.0, 255.0, 90.0]
  }
}
```

When no geometry is loaded, `bounds.min_position` and `bounds.max_position` serialize as `null` (mirroring the `selected-position` convention). No `plane` field is reported — the window id identifies the navigator and the live orientation is not guaranteed to match an anatomical plane under swivel mode; read orientation from the window's `/camera` if needed.

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` |
| 404 | `UNSUPPORTED_OPERATION` | `{id}` is `3d` |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/stdmulti/windows/{id}/selected-slice

For StdMulti only `{"step": N}` is accepted — the three 2D slices are coupled, so moving by world coordinate is done via `PUT /rendering/selected-position`. Sending a `position` field returns 400 with a hint. `step` is not range-checked; out-of-range values are clamped/snapped by MITK.

**Path parameter:** `name` ∈ {`axial`, `sagittal`, `coronal`} (`3d` returns 404 `UNSUPPORTED_OPERATION`).

**Request body (required, `application/json`):**

| Field | Type | Description |
|-------|------|-------------|
| `step` | integer ≥ 0 | Target step index |

**Example:**

```http
PUT /api/v1/rendering/editors/stdmulti/windows/axial/selected-slice
Content-Type: application/json

{ "step": 42 }
```

**Response: 204 No Content.**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Invalid JSON; missing `step`; non-integer or negative `step`; `position` field present (with hint to use `/rendering/selected-position`); unknown field |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` |
| 404 | `UNSUPPORTED_OPERATION` | `{id}` is `3d` |
| 422 | `RENDERING_ERROR` | MITK raised an error while applying the step |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No callback registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/stdmulti/windows/{id}/screenshot

Captures a single StdMultiWidget render window. The live render surface is **not** resized; if a different `width`/`height` is requested, the captured image is scaled after the fact, ignoring the source aspect ratio (i.e. stretched to fit the requested dimensions). Pass dimensions matching the source ratio if a faithful aspect is needed.

**Path parameter:** `name` ∈ {`axial`, `sagittal`, `coronal`, `3d`}.

Query parameters, request body, response content-types and shared error shapes are **identical** to `GET /api/v1/rendering/screenshot`.

**Window-specific errors (in addition to the inherited set):**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` |
| 503 | `EDITOR_NOT_ACTIVE` | StdMultiWidgetEditor is not currently open |

---

#### GET /api/v1/rendering/editors/mxn

Returns metadata about the MxN multi-widget editor, including the current cell ids when the editor is open. Cell ids are in the canonical fully-qualified form `<editor_name>__<bare>` (e.g. `mxn__widget0`).

**Response 200 (`application/json`):**

```json
{
  "alias": "mxn",
  "plugin_id": "org.mitk.editors.mxnmultiwidget",
  "active": true,
  "windows": ["mxn__widget0", "mxn__widget1", "mxn__widget2"]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `alias` | string | Always `"mxn"` |
| `plugin_id` | string | Always `"org.mitk.editors.mxnmultiwidget"` |
| `active` | boolean | `true` if an MxN editor instance is currently open |
| `windows` | string[] | Cell ids from the current layout. Canonical fully-qualified form (`<editor_name>__<bare>`); the same string as the `id` field of each `window` leaf in the v2 layout document, used verbatim as `{id}` in sub-resource URLs and for the `context` query parameter on the node-properties API. |

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | MxN multi-widget editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No editor list provider registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/mxn/windows

Returns the MxN editor's cells in pre-order traversal of the current layout. Each entry carries the cell `id` in canonical fully-qualified form (the URL segment for sub-resources, identical to the layout document's `id` field), an optional human-readable `name` (display label, omitted when the cell has no display name set), the cell `kind`, the persisted `view_direction` from the layout document, and the cell's `links` object.

**Response 200 (`application/json`):**

```json
[
  { "id": "mxn__widget0", "name": "Tumor axial", "kind": "2d", "view_direction": "axial",    "links": { "selection": "main" } },
  { "id": "mxn__widget1", "kind": "2d", "view_direction": "sagittal", "links": { "selection": "main" } },
  { "id": "mxn__widget2", "kind": "2d", "view_direction": "coronal",  "links": { "selection": "row2" } }
]
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Cell id (identity), in the canonical fully-qualified form `<editor_name>__<bare>`. Matches the `id` field of the corresponding `window` leaf in the layout document verbatim, used as-is for the URL path segment for sub-resources; no prefix translation. |
| `name` | string | *Optional.* Human-readable display label. Mirrors the optional `name` field of the corresponding `window` leaf. Free-form, not unique. Omitted when the cell has no display name set. |
| `kind` | string | `"2d"` (only value under v2; `"3d"` reserved for forward-compat) |
| `view_direction` | string | One of `"axial"`, `"sagittal"`, `"coronal"`, `"original"`. Persisted state from the layout document — *authoring intent*, not live orientation. Read live orientation from `/camera` if needed. |
| `links` | object | Per-cell synchronisation links from the layout document. v2 has only the `selection` dimension; v3 will add more dimension keys here additively without breaking v2 clients. |

Distinct from the StdMulti window list: MxN cells carry the persisted `view_direction` and `links` because they are part of the on-disk layout document; StdMulti has fixed window ids whose live anatomical mapping is dynamic (under swivel mode) and intentionally not asserted by `view_direction`.

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | MxN multi-widget editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No window list provider registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

### 8.3.1 MxN Editor REST Surface — Orientation

The MxN multi-widget editor exposes a REST surface that mirrors the StdMulti editor for camera and slice navigation, plus three MxN-specific resources: layout, per-cell selected position, and per-cell screenshots. This section captures the structural decisions a client author or maintainer needs in one place; per-endpoint detail follows in §8.3.2 onwards.

**One canonical cell id everywhere.** MxN cells carry an `id` in the fully-qualified form `<editor_name>__<bare>` (e.g. `mxn__widget0`, `mxn__axView`) inside the layout document. The same string is what every surface accepts, emits, and stores: URL path segments (`/rendering/editors/mxn/windows/{id}/...`), the `context` query parameter on the node-properties API, the cell list emitted by the windows endpoint, the engine-side render-window registration in the rendering manager, the per-renderer DataNode property context keys, and persisted-session state. No prefix translation happens at any boundary; what you see in the layout JSON is what you put in the URL.

Ids must match `^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$` and be unique within a layout document. The `<editor_name>` segment is the loading editor's `multiWidgetName` (default `mxn`); the C++ loader rejects documents whose ids do not start with the loading editor's `<editor_name>__` prefix. The recommended default form for tool-generated layouts is `mxn__widget<i>` (pre-order traversal index of the leaf, 0-based, no gaps); custom bare-id segments are accepted for hand-authored presets and clients that prefer semantic identifiers.

**Display label (`name`) is separate from identity.** Each window leaf may also carry an optional `name` field — a free-form human-readable display label (no pattern constraint, not required to be unique). The display label is pure metadata: REST URLs, the `context` query parameter, the per-renderer DataNode property keys, and persisted-session references all use `id`, never `name`. Renaming the display label therefore never invalidates a cached client reference. Cells with no display label simply omit the field; the windows endpoint and the layout document both omit `name` for those cells rather than emitting an empty string.

**Layout document is the single source of truth for MxN structure and selection sync.** `GET /api/v1/rendering/editors/mxn/layout` returns a v2.0 document that is byte-equivalent (modulo whitespace) to an in-tree `mxnLayout_*.json` preset file: same schema (`mxn-layout-v2.schema.json`), same shape. The same document is what `PUT .../layout` accepts. Consequences:

- A user can dump the current layout, save the response to disk, and drop it into the preset directory unchanged.
- Hand-authored presets are first-class REST citizens: PUT a preset directly, no translation needed.
- Per-cell `view_direction` and `links.selection` are part of the layout document — they are *persisted authoring intent*, not live state. The MxN windows list (`GET .../windows`) reports them so REST clients don't need to fetch the full layout for a quick overview.

**No `/sync` endpoint.** The MxN editor's "Synchronize" toolbar bool (a workbench UX setting that controls how interactive mouse/keyboard input on one cell propagates to others) is intentionally **not** exposed via REST. REST clients always operate on per-cell primitives. The Python `mitk-workbench-remote` client adds ergonomic helpers (e.g. iterate cells to apply a change to all) on top of these primitives. v3 of the layout schema will add per-cell synchronisation links for further dimensions; the layout document remains the only REST surface for sync state across v2 and v3.

**Per-cell selected position vs. global selected position.** Two distinct resources, two distinct concepts:

| Resource | Scope | Engine path |
|---|---|---|
| `GET/PUT /api/v1/rendering/selected-position` | Global anchor; drives the StdMulti editor's coupled views | `IRenderWindowPart::Get/SetSelectedPosition()` on the StdMulti editor part |
| `GET/PUT /api/v1/rendering/editors/mxn/windows/{id}/selected-position` | Single MxN cell's anchor | `QmitkAbstractMultiWidget::Get/SetSelectedPosition(point, widgetName)` on the MxN multi-widget |

Their values may legitimately diverge — an unsynced MxN cell can have a different anchor than the global one. Whether changes to one resource visibly affect the other depends on the workbench coupling state described above; clients that need a deterministic per-cell observation read each cell's `selected-position` after their PUT.

**Selected slice on MxN cells is step-only.** `PUT .../selected-slice` accepts only `{"step": N}`. World-anchor moves on a single cell live at the per-cell `selected-position` resource; global anchor moves at `/rendering/selected-position`. Sending `position` to slice returns 400 with a hint pointing to both primitives.

**Layout PUT tears down all cells.** Applying a layout via PUT destroys the existing cell tree and rebuilds from the document. Any cached cell `id` a client held before the PUT is invalid afterwards. The PUT response body is the freshly serialized layout (same shape as GET), so clients can refresh their cell list from the response without an additional GET round-trip.

**Camera under v2 is always 2D for MxN cells.** The v2 layout schema's `view_direction` enum has no `3d` value. The camera GET response carries `parallel_scale`; PUT rejects `perspective_angle`. A v3 cell type for 3D rendering may arrive later — at that point the per-window summary's `kind` flips to `"3d"` for those cells and the camera shape switches accordingly. The per-window summary already reports `kind` so clients can be forwards-compatible today.

**Concept-level errors that can surface on every MxN endpoint:**

| Status | Code | When |
|--------|------|------|
| 503 | `EDITOR_NOT_ACTIVE` | The MxN editor is not currently open in the workbench |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | The bridge callback is not registered (headless / Qt plugin not loaded), or the registered editor list does not expose the `mxn` alias |
| 400 | `INVALID_REQUEST` | The cell `{id}` is malformed — does not match the canonical fully-qualified form `<prefix>__<bare>` with URL-segment-safe characters. Rejected controller-side before any bridge dispatch |
| 404 | `RENDER_WINDOW_NOT_FOUND` | The cell `id` is well-formed but unknown to the editor |
| 404 | `UNSUPPORTED_OPERATION` | The sub-resource does not apply to this cell (reserved for future 3D MxN cells on `/selected-slice`) |

Per-endpoint error tables list any additional codes (400 for bad bodies, 422 for engine-side rendering failures, etc.).

---

#### GET /api/v1/rendering/editors/mxn/screenshot

Captures a screenshot of the MxN multi-widget canvas (the cell tree only — no toolbars, no side panels).

Query parameters, request body, response content-types and shared error shapes are **identical** to `GET /api/v1/rendering/screenshot`.

**Editor-specific errors (in addition to the inherited set):**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |

---

#### GET /api/v1/rendering/editors/mxn/windows/{id}/screenshot

Captures a single MxN cell render window. The live render surface is **not** resized; if a different `width`/`height` is requested, the captured image is scaled after the fact, ignoring the source aspect ratio (i.e. stretched to fit). Pass dimensions matching the source ratio if a faithful aspect is needed.

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

Query parameters, request body, response content-types and shared error shapes are **identical** to `GET /api/v1/rendering/screenshot`.

**Window-specific errors (in addition to the inherited set):**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |

---

#### GET /api/v1/rendering/editors/mxn/layout

Returns the current MxN layout as a v2.0 document. Strict mode: every group referenced by a cell appears in the top-level `groups` dict. The body is byte-equivalent (modulo whitespace) to an in-tree `mxnLayout_*.json` preset file; the same schema (`mxn-layout-v2.schema.json`) validates both.

**Response 200 (`application/json`):** v2.0 layout document. See `mxn-layout-v2.schema.json` for the full field-level spec.

```json
{
  "version": "2.0",
  "name": "Three Views",
  "groups": { "main": { "select_all": true } },
  "root": {
    "type": "split",
    "orientation": "horizontal",
    "children": [
      { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 100 },
      { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 100 },
      { "type": "window", "id": "mxn__widget2", "view_direction": "coronal",  "links": { "selection": "main" }, "size": 100 }
    ]
  }
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No layout getter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/mxn/layout

Applies a v2.0 layout document. **All existing cells are torn down and rebuilt from the document** (no positional reuse) — any cell `id` a client cached prior to the PUT is invalid afterwards. The 200 response body is the freshly serialized layout, so callers can refresh their cell list from the response without an extra GET.

**Request body (required, `application/json`):** v2.0 layout document, validated against `mxn-layout-v2.schema.json`.

```json
{
  "version": "2.0",
  "groups": { "main": { "select_all": true } },
  "root": {
    "type": "split", "orientation": "horizontal",
    "children": [
      { "type": "window", "id": "mxn__widget0", "view_direction": "axial",    "links": { "selection": "main" }, "size": 1 },
      { "type": "window", "id": "mxn__widget1", "view_direction": "sagittal", "links": { "selection": "main" }, "size": 1 }
    ]
  }
}
```

**Response 200 (`application/json`):** the freshly serialized layout (same shape as GET).

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Empty body; invalid JSON; schema / structural failure (version != 2.0, duplicate window ids, unknown view direction, missing group reference in strict mode, type errors). The `detail` field carries the engine's diagnostic message. |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No layout setter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

> **Note (mitk::Exception → 400 mapping is local to this endpoint).** Other rendering endpoints map `mitk::Exception` to 422 `RENDERING_ERROR`. Layout PUT is the one site that maps it to 400 `INVALID_REQUEST` because every `mitk::Exception` thrown out of `QmitkMxNMultiWidget::ApplyLayout` is a document-shape failure. If the engine ever broadens that contract to runtime issues, the catch must be narrowed.

---

#### GET /api/v1/rendering/editors/mxn/windows/{id}/camera

Returns the camera state of the addressed MxN cell. Under v2 every MxN cell is 2D, so the response carries `parallel_scale` and omits `perspective_angle` (mirrors the StdMulti 2D-window shape).

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Response 200 (`application/json`):**

```json
{
  "position":       [127.5,  83.2, 200.0],
  "focal_point":    [127.5,  83.2,  45.0],
  "view_up":        [0.0,    1.0,   0.0],
  "parallel_scale": 120.0
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No camera getter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/mxn/windows/{id}/camera

Partial update. At least one camera field must be present. `standard_view` is applied first and cannot be combined with explicit pose fields. Under v2 every MxN cell is 2D, so `perspective_angle` is rejected with 400 (matches the StdMulti rule for non-3D windows).

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Request body (`application/json`):**

```json
{ "parallel_scale": 120.0 }
```

**Response 204 No Content.**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Empty body; invalid JSON; unknown field; wrong type / shape; non-positive `parallel_scale`; `perspective_angle` (3D-only under v2); unknown `standard_view`; combination of `standard_view` with explicit pose fields |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 422 | `RENDERING_ERROR` | MITK rendering framework raised `mitk::Exception` while applying the patch |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No camera setter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/mxn/windows/{id}/selected-slice

Returns the cell's selected-slice state: integer step, the live world position on the slice plane, and navigator bounds (`steps`, `min_position`, `max_position`). `bounds.min_position` / `bounds.max_position` are `null` when no geometry is loaded.

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Response 200 (`application/json`):**

```json
{
  "step": 42,
  "position": [127.5, 83.2, 45.0],
  "bounds": {
    "steps": 90,
    "min_position": [0.0, 0.0, 0.0],
    "max_position": [255.0, 255.0, 90.0]
  }
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No selected-slice getter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/mxn/windows/{id}/selected-slice

**Body shape: `{"step": N}` only.** World-anchor moves on a single MxN cell live at the per-cell `selected-position` resource (below); global anchor moves at `PUT /rendering/selected-position`. Sending `position` here returns 400 with a hint pointing at both. No range checking — out-of-range step values are clamped/snapped by the navigator.

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Request body (`application/json`):**

```json
{ "step": 42 }
```

**Response 204 No Content.**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Empty body; invalid JSON; missing `step`; non-integer or negative `step`; `position` field present (with hint to per-cell selected-position and global selected-position); unknown field |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 422 | `RENDERING_ERROR` | MITK navigator raised `mitk::Exception` |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No selected-slice step setter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/mxn/windows/{id}/selected-position

Returns the cell's selected position (3D world anchor) plus scene bounds.

This is **distinct** from the global `/rendering/selected-position` resource: that one targets the StdMulti anchor; this one targets a single MxN cell's anchor via `QmitkAbstractMultiWidget::Get/SetSelectedPosition(point, widgetName)`. An unsynced MxN cell may legitimately have a different anchor than the global one — that's meaningful state, exposed accordingly.

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Response 200 (`application/json`):**

```json
{
  "position": [127.5, 83.2, 45.0],
  "bounds": { "min_position": [0.0, 0.0, 0.0], "max_position": [255.0, 255.0, 90.0] }
}
```

`bounds.min_position` / `bounds.max_position` are `null` when no geometry is loaded (mirrors the global selected-position bounds convention).

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No selected-position getter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### PUT /api/v1/rendering/editors/mxn/windows/{id}/selected-position

Sets the cell's per-cell 3D anchor.

**Propagation note (observed but not contracted).** Whether the change visibly propagates to other cells (or to the global `/rendering/selected-position` anchor) depends on the workbench's interactive coupling toolbar state — that state is intentionally not exposed via REST. Clients that need a deterministic per-cell observation read each cell's `selected-position` after their PUT.

No range checking — out-of-range values are clamped/snapped by MITK.

**Path parameter:** canonical fully-qualified MxN cell `id` from the layout document.

**Request body (`application/json`):**

```json
{ "position": [127.5, 83.2, 45.0] }
```

**Response 204 No Content.**

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | Empty body; invalid JSON; missing `position`; wrong shape (not an array of 3 numbers) |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Unknown `{id}` (malformed `{id}` returns 400 `INVALID_REQUEST`; see §8.3.1) |
| 422 | `RENDERING_ERROR` | MITK engine raised `mitk::Exception` (e.g., geometry validation failure) |
| 503 | `EDITOR_NOT_ACTIVE` | MxN editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No selected-position setter registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

#### GET /api/v1/rendering/editors/mxn/windows/{id}

Per-cell summary plus capability flags.

**Path parameter:**

| Parameter | Description |
|-----------|-------------|
| `id` | Cell `id` from the current layout in canonical fully-qualified form (`<editor_name>__<bare>`, matching `^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$`, unique within the layout). Matches the `id` field of the corresponding `window` leaf verbatim, used as-is with no prefix translation. |

**Response 200 (`application/json`):**

```json
{
  "id": "mxn__widget0",
  "name": "Tumor axial",
  "kind": "2d",
  "view_direction": "axial",
  "links": { "selection": "main" },
  "has_camera": true,
  "has_selected_slice": true,
  "has_selected_position": true
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Echo of the path parameter |
| `name` | string | *Optional.* Display label of the cell, mirroring the optional `name` field of the corresponding `window` leaf. Omitted when the cell has no display name set. |
| `kind` | string | `"2d"` under v2 |
| `view_direction` | string | Persisted view direction (see windows list note above) |
| `links` | object | Per-cell synchronisation links from the layout document |
| `has_camera` | boolean | Always `true` in v2 |
| `has_selected_slice` | boolean | `true` for 2D cells; reserved `false` for v3 3D cells |
| `has_selected_position` | boolean | Always `true` — per-cell selected position is a v2 capability, distinct from the global `/rendering/selected-position` resource |

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 400 | `INVALID_REQUEST` | `{id}` is malformed (rejected controller-side before bridge dispatch) |
| 404 | `RENDER_WINDOW_NOT_FOUND` | `{id}` is well-formed but not a known MxN cell |
| 503 | `EDITOR_NOT_ACTIVE` | MxN multi-widget editor is not currently open |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No window list provider registered |
| 500 | `INTERNAL_ERROR` | Unexpected error |

---

## 9. Error Handling {#sec-9-error-handling}

### 9.1 Error Response Format

Following RFC 7807 (Problem Details for HTTP APIs):

```json
{
  "error": {
    "type": "https://docs.mitk.org/api/errors/NODE_NOT_FOUND",
    "code": "NODE_NOT_FOUND",
    "title": "Node Not Found",
    "message": "No node found with UID 'invalid-uid'",
    "status": 404,
    "instance": "/api/v1/datastorage/nodes/invalid-uid",
    "details": {
      "requested_uid": "invalid-uid"
    }
  }
}
```

### 9.2 Standard Error Codes

| HTTP Status | Code | Description |
|-------------|------|-------------|
| 400 | `INVALID_REQUEST` | Malformed request body or parameters |
| 400 | `PROPERTY_PROTECTED` | Cannot modify/delete protected property |
| 401 | `UNAUTHORIZED` | Missing or invalid authentication |
| 403 | `ACCESS_DENIED` | Client IP not permitted to access the server |
| 403 | `FILE_ACCESS_DENIED` | Requested file path is outside the allowed directories |
| 404 | `NODE_NOT_FOUND` | Node with given UID does not exist |
| 404 | `PROPERTY_NOT_FOUND` | Property does not exist on node |
| 404 | `NO_DATA` | Node exists but has no data attached (422 in `/rendering` endpoints) |
| 404 | `RENDER_WINDOW_NOT_FOUND` | Addressed render window id is not known to the editor |
| 404 | `UNSUPPORTED_OPERATION` | Sub-resource does not apply to the addressed window (e.g. `selected-slice` on `3d`) |
| 406 | `TRANSFER_MODE_NOT_AVAILABLE` | Requested transfer mode not supported |
| 409 | `CIRCULAR_HIERARCHY_REFERENCE` | Target parent is a descendant of the node being reparented |
| 409 | `NODE_HAS_CHILDREN` | Cannot delete node with children |
| 415 | `UNSUPPORTED_FORMAT` | Data format not supported |
| 422 | `FILE_NOT_FOUND` | Referenced file path does not exist |
| 422 | `FILE_READ_ERROR` | Cannot read referenced file |
| 422 | `NO_GEOMETRY` | Node data has no usable time geometry |
| 429 | `RATE_LIMIT_EXCEEDED` | Too many requests |
| 500 | `INTERNAL_ERROR` | Unexpected server error |
| 500 | `SERIALIZATION_ERROR` | Failed to serialize data for transfer |
| 500 | `TIME_STEPPER_NOT_AVAILABLE` | Time stepper is not available |
| 503 | `DATASTORAGE_NOT_AVAILABLE` | DataStorage not connected to REST server |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No render window bridge callback registered (headless mode or Qt plugin not loaded) |
| 503 | `EDITOR_NOT_ACTIVE` | Addressed editor (e.g. StdMultiWidgetEditor) is not currently open in the workbench |
| 503 | `TIME_NAVIGATION_NOT_AVAILABLE` | TimeNavigationController is not available |

### 9.3 Validation Errors

For validation failures, include field-level details:

```json
{
  "error": {
    "code": "INVALID_REQUEST",
    "message": "Request validation failed",
    "status": 400,
    "validation_errors": [
      {
        "field": "name",
        "message": "Name cannot be empty"
      },
      {
        "field": "transfer.file_path",
        "message": "Path must be absolute"
      }
    ]
  }
}
```

---

## 10. Examples {#sec-10-examples}

### 10.1 Load Image and Set Properties

```python
import requests

BASE_URL = "http://localhost:8080/api/v1"
HEADERS = {"Authorization": "Bearer my-token"}

# Load image
response = requests.post(
    f"{BASE_URL}/datastorage/nodes",
    headers=HEADERS,
    json={
        "name": "CT_Scan",
        "transfer": {
            "file_path": "/data/ct.nrrd"
        }
    }
)
node = response.json()["data"]
node_uid = node["uid"]

# Set multiple properties (merge semantics)
requests.patch(
    f"{BASE_URL}/datastorage/nodes/{node_uid}/properties",
    headers=HEADERS,
    json={
        "visible": True,
        "opacity": 0.8,
        "color": {"type": "ColorProperty", "value": [1.0, 0.5, 0.0]}
    }
)
```

### 10.2 Find Node by Path and Get Data

```python
# Find node by path (always returns array)
response = requests.get(
    f"{BASE_URL}/datastorage/nodes",
    headers=HEADERS,
    params={"path": "/CT_Scan/Segmentation"}
)
result = response.json()
nodes = result["data"]
total = result["meta"]["total_count"]

if total == 0:
    print("No nodes found")
elif total == 1:
    node_uid = nodes[0]["uid"]
    
    # Get data as file reference
    response = requests.get(
        f"{BASE_URL}/datastorage/nodes/{node_uid}/data",
        headers={
            **HEADERS,
            "Accept": "application/json",
            "X-MITK-Transfer-Mode": "file-reference"
        }
    )
    data_info = response.json()
    file_path = data_info["transfer"]["file_path"]
    dimensions = data_info["data_metadata"]["dimensions"]
else:
    print(f"Ambiguous: {total} nodes match path")
    for node in nodes:
        print(f"  - {node['uid']}: {node['name']} ({node['data_type']})")
```

### 10.3 Add Segmentation as Child Node

```python
# Add segmentation as child of CT
response = requests.post(
    f"{BASE_URL}/datastorage/nodes/{ct_node_uid}/children",
    headers=HEADERS,
    json={
        "name": "Liver_Segmentation",
        "transfer": {
            "file_path": "/data/liver_seg.nrrd"
        }
    }
)
seg_node = response.json()["data"]
print(f"Created: {seg_node['path']}")  # /CT_Scan/Liver_Segmentation
```

### 10.4 List Visible Images

```python
response = requests.get(
    f"{BASE_URL}/datastorage/nodes",
    headers=HEADERS,
    params={
        "hierarchy": "toplevel",
        "data_type": "Image",
        "filter.visible": "true",
        "fields": "uid,name,data_type"
    }
)
for node in response.json()["data"]:
    print(f"{node['name']} ({node['data_type']})")
```

### 10.5 Rename a Node

```python
# Rename via property endpoint
requests.put(
    f"{BASE_URL}/datastorage/nodes/{node_uid}/properties/name",
    headers=HEADERS,
    json={"value": "CT_Scan_Renamed"}
)
```

### 10.6 Get All Properties for Specific Renderer

```python
response = requests.get(
    f"{BASE_URL}/datastorage/nodes/{node_uid}/properties",
    headers=HEADERS,
    params={
        "context": "stdmulti.widget0",
        "property_scope": "all"
    }
)
properties = response.json()["data"]["properties"]
```

### 10.7 Load Image, Configure Appearance, and Refresh Render Windows

A typical workflow: upload data, adjust properties, then trigger a render update and fit the views to the new data.

```python
import requests

BASE_URL = "http://localhost:8080/api/v1"
HEADERS = {"Authorization": "Bearer my-token"}

# 1. Upload image
resp = requests.post(
    f"{BASE_URL}/datastorage/nodes",
    headers=HEADERS,
    json={"name": "CT_Scan", "transfer": {"file_path": "/data/ct.nrrd"}}
)
uid = resp.json()["data"]["uid"]

# 2. Set appearance properties
requests.patch(
    f"{BASE_URL}/datastorage/nodes/{uid}/properties",
    headers=HEADERS,
    json={
        "visible": True,
        "opacity": 1.0,
        "color": {"type": "ColorProperty", "value": [1.0, 1.0, 1.0]}
    }
)

# 3. Trigger render update so the node becomes visible
requests.post(f"{BASE_URL}/rendering/update", headers=HEADERS)

# 4. Fit all views to the loaded image's geometry
requests.post(f"{BASE_URL}/rendering/reinit", headers=HEADERS, json={"uids": [uid]})
```

> **Note:** Steps 3 and 4 can be combined as needed. `POST /rendering/reinit` with no body fits views to all visible data at once; with `{"uids": ["..."]}` in the body it focuses on a single node's geometry; with multiple UIDs it fits the combined bounding box of all listed nodes.

---

## 11. Future Extensions {#sec-11-future-extensions}

### 11.1 Planned for Future Versions

| Feature | Target Version | Description |
|---------|---------------|-------------|
| Project management | v1.1 | Save/load projects |
| Shared memory transfer | v1.2 | Zero-copy data transfer |
| Batch operations | v1.3 | `/datastorage/nodes/batch` endpoint |
| Async operations | v1.3 | Long-running operations with task tracking |
| MxN multi-widget editor | TBD | Activate the `mxn` editor alias and add the `/rendering/editors/mxn/...` hierarchy |

### 11.2 Extension Points

The API is designed for extension:

- **New transfer modes:** Add to `X-MITK-Transfer-Mode` header values
- **New data formats:** Add to supported format list
- **New property types:** JSON structure supports arbitrary types
- **New filter operators:** Extend filter parameter syntax
- **New system fields:** Add to node response (backwards-compatible)

---

## Appendix A: Quick Reference

### Endpoints Summary

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/api/v1/info` | API info |
| `GET` | `/api/v1/health` | Health check |
| `GET` | `/api/v1/config/file-access` | File access configuration |
| `GET` | `/api/v1/datastorage/nodes` | List nodes |
| `POST` | `/api/v1/datastorage/nodes` | Create node |
| `GET` | `/api/v1/datastorage/nodes/{uid}` | Get node |
| `PATCH` | `/api/v1/datastorage/nodes/{uid}` | Reparent node |
| `DELETE` | `/api/v1/datastorage/nodes/{uid}` | Delete node |
| `GET` | `/api/v1/datastorage/nodes/{uid}/data` | Get node data |
| `PUT` | `/api/v1/datastorage/nodes/{uid}/data` | Replace node data |
| `GET` | `/api/v1/datastorage/nodes/{uid}/children` | List children |
| `POST` | `/api/v1/datastorage/nodes/{uid}/children` | Add child |
| `GET` | `/api/v1/datastorage/nodes/{uid}/properties` | Get properties |
| `PUT` | `/api/v1/datastorage/nodes/{uid}/properties` | Replace all properties |
| `PATCH` | `/api/v1/datastorage/nodes/{uid}/properties` | Update properties |
| `GET` | `/api/v1/datastorage/nodes/{uid}/properties/{name}` | Get property |
| `PUT` | `/api/v1/datastorage/nodes/{uid}/properties/{name}` | Set property |
| `DELETE` | `/api/v1/datastorage/nodes/{uid}/properties/{name}` | Delete property |
| `POST` | `/api/v1/rendering/update` | Trigger render window update |
| `POST` | `/api/v1/rendering/reinit` | Fit all views to visible data, or to one/multiple nodes when `uids` body field is given |
| `GET` | `/api/v1/rendering/selected-position` | Get the workbench's globally selected world position |
| `PUT` | `/api/v1/rendering/selected-position` | Set the workbench's globally selected world position |
| `GET` | `/api/v1/rendering/selected-time` | Get the workbench's globally selected time step |
| `PUT` | `/api/v1/rendering/selected-time` | Set the workbench's globally selected time step |
| `GET` | `/api/v1/rendering/screenshot` | Screenshot of the active editor's primary window |
| `GET` | `/api/v1/rendering/editors` | List render-window editors that expose a REST surface |
| `GET` | `/api/v1/rendering/editors/stdmulti` | StdMultiWidgetEditor metadata |
| `GET` | `/api/v1/rendering/editors/stdmulti/screenshot` | Composite screenshot of the StdMultiWidget editor |
| `GET` | `/api/v1/rendering/editors/stdmulti/windows` | List render windows of the StdMultiWidget editor |
| `GET` | `/api/v1/rendering/editors/stdmulti/windows/{id}` | Metadata of a single StdMultiWidget render window |
| `GET` | `/api/v1/rendering/editors/stdmulti/windows/{id}/screenshot` | Screenshot of a single StdMultiWidget render window |
| `GET` | `/api/v1/rendering/editors/stdmulti/windows/{id}/camera` | Get camera state of a StdMultiWidget render window |
| `PUT` | `/api/v1/rendering/editors/stdmulti/windows/{id}/camera` | Update camera state of a StdMultiWidget render window |
| `GET` | `/api/v1/rendering/editors/stdmulti/windows/{id}/selected-slice` | Get selected slice index of a 2D StdMultiWidget render window |
| `PUT` | `/api/v1/rendering/editors/stdmulti/windows/{id}/selected-slice` | Set selected slice index of a 2D StdMultiWidget render window |
| `GET` | `/api/v1/rendering/editors/mxn` | MxN multi-widget editor metadata |
| `GET` | `/api/v1/rendering/editors/mxn/screenshot` | Composite screenshot of the MxN editor canvas |
| `GET` | `/api/v1/rendering/editors/mxn/layout` | Get the current MxN layout (v2.0 document) |
| `PUT` | `/api/v1/rendering/editors/mxn/layout` | Apply a v2.0 layout document to the MxN editor (response body = freshly serialized layout) |
| `GET` | `/api/v1/rendering/editors/mxn/windows` | List MxN cells (`id`, optional display `name`, `kind`, `view_direction`, `links`) |
| `GET` | `/api/v1/rendering/editors/mxn/windows/{id}` | Metadata of a single MxN cell |
| `GET` | `/api/v1/rendering/editors/mxn/windows/{id}/screenshot` | Screenshot of a single MxN cell |
| `GET` | `/api/v1/rendering/editors/mxn/windows/{id}/camera` | Get camera state of an MxN cell |
| `PUT` | `/api/v1/rendering/editors/mxn/windows/{id}/camera` | Update camera state of an MxN cell |
| `GET` | `/api/v1/rendering/editors/mxn/windows/{id}/selected-slice` | Get selected slice index of a 2D MxN cell |
| `PUT` | `/api/v1/rendering/editors/mxn/windows/{id}/selected-slice` | Set selected slice index of a 2D MxN cell (`step` only) |
| `GET` | `/api/v1/rendering/editors/mxn/windows/{id}/selected-position` | Get the per-cell 3D world anchor of an MxN cell |
| `PUT` | `/api/v1/rendering/editors/mxn/windows/{id}/selected-position` | Set the per-cell 3D world anchor of an MxN cell |

### Query Parameters Summary

| Parameter | Applies To | Description |
|-----------|-----------|-------------|
| `limit` | Collections | Max results |
| `offset` | Collections | Pagination offset |
| `sort` | Collections | Sort field (prefix `-` for desc) |
| `fields` | All GET | Field selection |
| `hierarchy` | `/nodes` | `all` or `toplevel` |
| `path` | `/nodes` | Path-based lookup |
| `data_type` | `/nodes` | Filter by MITK type |
| `parent_uid` | `/nodes` | Filter by parent |
| `filter.*` | `/nodes` | Property filters (`*`, `?` wildcards) |
| `context` | `/nodes`, `/properties` | Renderer context |
| `property_scope` | `/nodes`, `/properties` | `all`, `node`, `data` |
| `content` | `/properties` | Include values (default: true) |
| `names` | `/properties` | Property name filter |
| `recursive` | DELETE `/nodes` | Delete children |

### Headers Summary

| Header | Applies To | Values | Description |
|--------|-----------|--------|-------------|
| `Content-Type` | POST/PUT/PATCH | `application/json`, `application/octet-stream` | Request body format |
| `Accept` | GET | `application/json`, `application/octet-stream` | Desired response format |
| `Authorization` | All | `Bearer {token}` | Authentication (when enabled) |
| `X-MITK-Transfer-Mode` | `/data` endpoints | `direct`, `file-reference` | Data transfer mode |

### HTTP Methods by Endpoint

All endpoints are relative to the base URL `/api/v1`.

| Endpoint | GET | POST | PUT | PATCH | DELETE |
|----------|-----|------|-----|-------|--------|
| `/info` | ✓ Info | — | — | — | — |
| `/health` | ✓ Health | — | — | — | — |
| `/config/file-access` | ✓ Config | — | — | — | — |
| `/datastorage/nodes` | ✓ List | ✓ Create | — | — | — |
| `/datastorage/nodes/{uid}` | ✓ Read | — | — | ✓ Reparent | ✓ Delete |
| `/datastorage/nodes/{uid}/data` | ✓ Read | — | ✓ Replace | — | — |
| `/datastorage/nodes/{uid}/children` | ✓ List | ✓ Create | — | — | — |
| `/datastorage/nodes/{uid}/properties` | ✓ Read | — | ✓ Replace all | ✓ Update | — |
| `/datastorage/nodes/{uid}/properties/{name}` | ✓ Read | — | ✓ Set | — | ✓ Delete |
| `/rendering/update` | — | ✓ Update | — | — | — |
| `/rendering/reinit` | — | ✓ Reinit (global or node-scoped) | — | — | — |
| `/rendering/selected-position` | ✓ Read | — | ✓ Set | — | — |
| `/rendering/selected-time` | ✓ Read | — | ✓ Set | — | — |
| `/rendering/screenshot` | ✓ Active editor | — | — | — | — |
| `/rendering/editors` | ✓ List | — | — | — | — |
| `/rendering/editors/stdmulti` | ✓ Metadata | — | — | — | — |
| `/rendering/editors/stdmulti/screenshot` | ✓ Composite | — | — | — | — |
| `/rendering/editors/stdmulti/windows` | ✓ List | — | — | — | — |
| `/rendering/editors/stdmulti/windows/{id}` | ✓ Read | — | — | — | — |
| `/rendering/editors/stdmulti/windows/{id}/screenshot` | ✓ Screenshot | — | — | — | — |
| `/rendering/editors/stdmulti/windows/{id}/camera` | ✓ Read | — | ✓ Update | — | — |
| `/rendering/editors/stdmulti/windows/{id}/selected-slice` | ✓ Read | — | ✓ Set | — | — |
| `/rendering/editors/mxn` | ✓ Metadata | — | — | — | — |
| `/rendering/editors/mxn/screenshot` | ✓ Composite | — | — | — | — |
| `/rendering/editors/mxn/layout` | ✓ Read | — | ✓ Apply | — | — |
| `/rendering/editors/mxn/windows` | ✓ List | — | — | — | — |
| `/rendering/editors/mxn/windows/{id}` | ✓ Read | — | — | — | — |
| `/rendering/editors/mxn/windows/{id}/screenshot` | ✓ Screenshot | — | — | — | — |
| `/rendering/editors/mxn/windows/{id}/camera` | ✓ Read | — | ✓ Update | — | — |
| `/rendering/editors/mxn/windows/{id}/selected-slice` | ✓ Read | — | ✓ Set | — | — |
| `/rendering/editors/mxn/windows/{id}/selected-position` | ✓ Read | — | ✓ Set | — | — |

---

## Appendix B: Server Implementation Notes

This appendix documents implementation details that clients may need to be aware of.

### B.1 Reserved Property Prefix

Properties with names starting with `restapi.` are reserved for internal server use and are subject to the following rules:

- **Filtered from responses:** Internal properties are automatically excluded from property listing responses (`GET /properties`)
- **Protected from modification:** Attempts to set or delete internal properties via the API will be rejected
- **Used for internal bookkeeping:** The server uses these properties for UID mapping, modification tracking, and other internal state management

**Examples of internal properties:**
- `restapi.uid` — Internal UID mapping
- `restapi.modified` — Modification flag
- `restapi.last_modification` — Last modification operation

> **Warning:** Clients should not rely on the existence or format of internal properties. Their implementation may change between server versions.

---

*End of Document*
