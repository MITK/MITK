# MITK Workbench REST API Specification {#MITKRESTAPISpec}

**Document Version:** 1.1
**Date:** February 21, 2026

---

## Table of Contents

1. [Overview](#1-overview)
2. [Design Principles](#2-design-principles)
3. [Base URL and Versioning](#3-base-url-and-versioning)
4. [Authentication](#4-authentication)
5. [Common Patterns](#5-common-patterns)
6. [Node Identification Strategy](#6-node-identification-strategy)
7. [Data Transfer Modes](#7-data-transfer-modes)
8. [API Endpoints](#8-api-endpoints)
   - [Discovery and Health](#81-discovery-and-health)
   - [Data Storage](#82-data-storage---nodes)
     - [Nodes](#821-data-storage---nodes)
     - [Node Data Payload](#822-node-data-payload)
     - [Node Children](#823-node-children)
     - [Node Properties](#824-node-properties)
   - [Rendering](#83-rendering)
9. [Error Handling](#9-error-handling)
10. [Examples](#10-examples)
11. [Future Extensions](#11-future-extensions)

---

## 1. Overview

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

## 2. Design Principles

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

## 3. Base URL and Versioning

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
GET /api/v1/
```

Returns API metadata including supported versions and deprecation notices.

---

## 4. Authentication

### Phase 1: API Token Authentication

```http
GET /api/v1/datastorage/nodes
X-MITK-API-Token: {token}
```

### Future: OAuth2/OIDC

For Kaapana integration and enterprise deployments.

### Development Mode

Authentication can be disabled for local development:

```yaml
security:
  enabled: false
```

---

## 5. Common Patterns

### 5.1 Request Headers

| Header | Required | Description |
|--------|----------|-------------|
| `Content-Type` | For POST/PUT/PATCH | `application/json` for metadata |
| `Accept` | Optional | Desired response format |
| `X-MITK-API-Token` | When auth enabled | Authentication token |
| `X-MITK-Transfer-Mode` | For data endpoints | `direct`, `file-reference`, `shared-memory` |

### 5.2 Standard Response Envelope

All successful responses use a consistent structure:

```json
{
  "data": { ... },
  "meta": {  }
}
```

> **Note:** The `timestamp` field contains the DataStorage modification time (an ITK timestamp obtained via `GetMTime()`). This is an ever-increasing integer valid within the current session that indicates when the data storage was last modified.

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
?data_type=mitk::Image&parent_uid=node-001
```

**Property filters** use the `filter.` prefix:

```
?filter.visible=true&filter.name=CT*
```

**Combined example:**
```http
GET /api/v1/datastorage/nodes?data_type=mitk::Image&filter.visible=true
```

Supported filter operators:
- Exact match: `filter.name=CT_Scan`
- Not equal: `filter.visible!=true`
- Wildcard `*` (zero or more characters): `filter.name=CT*`, `filter.name=*Scan`, `filter.name=CT*2024*Scan`
- Wildcard `?` (exactly one character): `filter.name=Node_0?`, `filter.name=????`

**Supported system field filters:**
- `data_type` — MITK class name (e.g., `mitk::Image`, `mitk::LabelSetImage`, `mitk::Surface`)
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

---

## 6. Node Identification Strategy

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
      "data_type": "mitk::LabelSetImage",
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
      "data_type": "mitk::LabelSetImage",
      "timestamp": 1705661400
    },
    {
      "uid": "def-456",
      "name": "Segmentation",
      "path": "/CT_Scan/Segmentation",
      "parent_uid": "node-001",
      "data_type": "mitk::LabelSetImage",
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
  "data_type": "mitk::Image",
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
| `data_type` | string | MITK data class (e.g., `mitk::Image`) |
| `children_count` | integer | Number of child nodes |
| `timestamp` | integer | ITK modified time (ever-increasing integer within session; higher values are newer) |

> **Note:** The `timestamp` is an ITK modification time — an ever-increasing integer valid only within the current session. Higher values indicate more recent modifications. It is not persisted across sessions and cannot be compared across different sessions.

---

## 7. Data Transfer Modes

Large data (images, meshes, segmentations) can be transferred via three modes, supporting different performance requirements.

### 7.1 Mode Selection

Clients indicate preferred mode via header:

```http
X-MITK-Transfer-Mode: direct | file-reference | shared-memory
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

### 7.3 Mode: File Reference

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

### 7.4 Mode: Shared Memory (Future)

Zero-copy transfer via shared memory.

**Request:**
```http
GET /api/v1/datastorage/nodes/{uid}/data
Accept: application/json
X-MITK-Transfer-Mode: shared-memory
```

**Response:**
```json
{
  "transfer": {
    "mode": "shared-memory",
    "format": "arrow",
    "size_bytes": 125829120,
    "shm_name": "/mitk-shm-abc123",
    "dtype": "int16",
    "shape": [512, 512, 256],
    "strides": [2, 1024, 524288]
  },
  "data_metadata": {
    "dimensions": [512, 512, 256],
    "spacing": [0.5, 0.5, 1.0],
    "origin": [0.0, 0.0, 0.0],
    "pixel_type": "int16"
  }
}
```

**Use case:** Maximum performance, co-located processes, real-time applications.

**Shared memory lifetime:** References are valid for the lifetime of the MITK Workbench process. Clients must handle access failures gracefully.

> **Note:** The `data_metadata` section is consistent across all transfer modes and provides informative metadata about the data content.

### 7.5 Response Structure

All JSON data transfer responses follow the same structure with two clearly separated sections:

| Section | Purpose | Content |
|---------|---------|---------|
| `transfer` | Transfer-specific information | Mode, format, location, size |
| `data_metadata` | Data-specific information | Type-dependent fields (e.g., dimensions, spacing for images) |

The `transfer` section varies by mode. The `data_metadata` section is consistent across all modes but its contents depend on the data type:

- **Images**: `dimensions`, `spacing`, `origin`, `pixel_type`
- **Surfaces**: `points_count`, `cells_count`, `bounds`
- **Point Sets**: `points_count`, `bounds`

### 7.6 Mode Negotiation

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

### 7.7 Upload Data Transfer

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

## 8. API Endpoints

### 8.1 Discovery and Health

#### GET /api/v1/

API information and capabilities.

**Response:**
```json
{
  "data": {
    "name": "MITK Workbench REST API",
    "api_version": "v1",
    "mitk_version": "2024.12",
    "capabilities": {
      "transfer_modes": ["direct", "file-reference"],
      "authentication": ["api-token"]
    },
    "documentation_url": "https://docs.mitk.org/api/v1"
  }
}
```

> **Note:** The `capabilities.events` section with `zeromq_endpoint` is planned for a future version when ZeroMQ event notification support is implemented. Currently, this field is not included in the response.
```

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

### 8.2 Data Storage
#### 8.2.1 Nodes

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
GET /api/v1/datastorage/nodes?hierarchy=toplevel&data_type=mitk::Image&filter.visible=true
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
      "data_type": "mitk::Image",
      "children_count": 2,
      "timestamp": 1705659600
    },
    {
      "uid": "node-002",
      "name": "MRI_T1",
      "path": "/MRI_T1",
      "parent_uid": null,
      "data_type": "mitk::Image",
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
      "prev": "/api/v1/datastorage/nodes?data_type=mitk::Image&limit=50&offset=0",
      "next": "/api/v1/datastorage/nodes?data_type=mitk::Image&limit=50&offset=100"
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
    "data_type": "mitk::Image",
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

> **Content-Type:** Use `application/json` for file-reference mode (JSON body with optional `transfer.file_path`) or `application/octet-stream` for direct transfer mode (binary body). Requests with unsupported Content-Type values will receive a 415 Unsupported Media Type response. Omitting Content-Type entirely creates a node with default values only (no data, auto-generated name).

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
    "data_type": "mitk::Image",
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
    "data_type": "mitk::Image",
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

#### 8.2.2 Node Data Payload

##### GET /api/v1/datastorage/nodes/{uid}/data

Retrieve the actual data (image, mesh, etc.) of a node.

**Headers:**

| Header | Values | Description |
|--------|--------|-------------|
| `Accept` | `application/octet-stream`, `application/json` | Response format |
| `X-MITK-Transfer-Mode` | `direct`, `file-reference`, `shared-memory` | Transfer mode |

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

**Response:** See [Section 7.3](#73-mode-file-reference)

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
    "data_type": "mitk::Image",
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
    "data_type": "mitk::Image",
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

#### 8.2.3 Node Children

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
      "data_type": "mitk::LabelSetImage",
      "children_count": 0,
      "timestamp": 1705661400
    },
    {
      "uid": "node-001-mask",
      "name": "Mask",
      "path": "/CT_Scan/Mask",
      "parent_uid": "node-001",
      "data_type": "mitk::Image",
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
    "data_type": "mitk::LabelSetImage",
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

#### 8.2.4 Node Properties

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

### 8.3 Rendering

Rendering endpoints control how MITK Workbench render windows refresh and orient themselves. They are deliberately separate from data and property endpoints: callers can batch multiple mutations (upload data, set properties) and then trigger a single render update, avoiding per-change flicker.

All rendering calls are dispatched to the main/UI thread by the server — callers do not need to account for threading.

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

Requires the Qt workbench plugin to be running and the StdMultiWidgetEditor to be open (503 otherwise). If no input geometry is available, `bounds.min` and `bounds.max` are `null`.

**Response 200 (`application/json`):**

```json
{
  "position": [10.0, 20.0, 30.0],
  "bounds": {
    "min": [-50.0, -50.0, -50.0],
    "max": [50.0, 50.0, 50.0]
  }
}
```

**Error responses:**

| Status | Code | Description |
|--------|------|-------------|
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | StdMultiWidgetEditor is not open or workbench plugin not connected |
| 500 | `INTERNAL_ERROR` | Unexpected error reading crosshair or geometry state |

---

#### PUT /api/v1/rendering/selected-position

Moves the crosshair to the given 3D world position via `IRenderWindowPart::SetSelectedPosition()` on the StdMultiWidgetEditor, which updates all synchronized views.

Requires the Qt workbench plugin to be running and the StdMultiWidgetEditor to be open (503 otherwise).

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
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | StdMultiWidgetEditor is not open or workbench plugin not connected |
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
| 422 | `RENDERING_ERROR` | Unexpected rendering framework error |

---

#### GET /api/v1/rendering/screenshot

Captures a screenshot of the active application window. Requires the Qt workbench plugin to be running and the screenshot provider to be connected.

**Query parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `format` | string | `"png"` | Image encoding: `"png"` or `"jpeg"` |
| `width` | integer | (native) | Output width in pixels; must be positive |
| `height` | integer | (native) | Output height in pixels; must be positive |

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
| 400 | `INVALID_REQUEST` | Invalid `format` value, non-positive or non-integer `width`/`height`, or only one of `width`/`height` provided |
| 500 | `INTERNAL_ERROR` | Screenshot capture failed |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | No screenshot provider connected (headless mode or Qt plugin not loaded) |

---

## 9. Error Handling

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
| 400 | `INVALID_PROPERTY_VALUE` | Property value validation failed |
| 400 | `PROPERTY_PROTECTED` | Cannot modify/delete protected property |
| 401 | `UNAUTHORIZED` | Missing or invalid authentication |
| 403 | `ACCESS_DENIED` | Client IP not permitted to access the server |
| 403 | `FILE_ACCESS_DENIED` | Requested file path is outside the allowed directories |
| 404 | `NODE_NOT_FOUND` | Node with given UID does not exist |
| 404 | `PROPERTY_NOT_FOUND` | Property does not exist on node |
| 404 | `NO_DATA` | Node exists but has no data attached (422 in `/rendering` endpoints) |
| 406 | `TRANSFER_MODE_NOT_AVAILABLE` | Requested transfer mode not supported |
| 409 | `CIRCULAR_HIERARCHY_REFERENCE` | Target parent is a descendant of the node being reparented |
| 409 | `NODE_HAS_CHILDREN` | Cannot delete node with children |
| 409 | `NAME_CONFLICT` | Node name conflict in same parent |
| 413 | `PAYLOAD_TOO_LARGE` | Request body exceeds size limit |
| 415 | `UNSUPPORTED_FORMAT` | Data format not supported |
| 422 | `FILE_NOT_FOUND` | Referenced file path does not exist |
| 422 | `FILE_READ_ERROR` | Cannot read referenced file |
| 422 | `NO_GEOMETRY` | Node data has no usable time geometry |
| 429 | `RATE_LIMIT_EXCEEDED` | Too many requests |
| 500 | `INTERNAL_ERROR` | Unexpected server error |
| 500 | `SERIALIZATION_ERROR` | Failed to serialize data for transfer |
| 503 | `DATASTORAGE_NOT_AVAILABLE` | DataStorage not connected to REST server |
| 503 | `RENDER_WINDOW_NOT_AVAILABLE` | Qt workbench plugin not loaded or StdMultiWidgetEditor not open |
| 503 | `SERVICE_UNAVAILABLE` | Server temporarily unavailable |

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

## 10. Examples

### 10.1 Load Image and Set Properties

```python
import requests

BASE_URL = "http://localhost:8080/api/v1"
HEADERS = {"X-MITK-API-Token": "my-token"}

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
        "data_type": "mitk::Image",
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

## 11. Future Extensions

### 11.1 Planned for Future Versions

| Feature | Target Version | Description |
|---------|---------------|-------------|
| Project management | v1.1 | Save/load projects |
| Shared memory transfer | v1.2 | Zero-copy data transfer |
| Batch operations | v1.3 | `/datastorage/nodes/batch` endpoint |
| Async operations | v1.3 | Long-running operations with task tracking |

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
| `GET` | `/api/v1/` | API info |
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
| `X-MITK-API-Token` | All | Token string | Authentication (when enabled) |
| `X-MITK-Transfer-Mode` | `/data` endpoints | `direct`, `file-reference`, `shared-memory` | Data transfer mode |

### HTTP Methods by Endpoint

All endpoints are relative to the base URL `/api/v1`.

| Endpoint | GET | POST | PUT | PATCH | DELETE |
|----------|-----|------|-----|-------|--------|
| `/` | ✓ Info | — | — | — | — |
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
