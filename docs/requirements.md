# Requirements

## Scope
This document defines requirements for the migration from the legacy Arduino-hosted web UI to a Flutter app, with BLE integration planned for a later phase.

- **Current focus**: frontend parity in Flutter (`main.dart`) using a mock scan service.
- **Future focus**: Seeed XIAO ESP32-S3 firmware as BLE GATT server + Flutter BLE client.

Legacy Arduino WiFi/web-server code remains reference behavior, not active feature scope.

## Frontend parity requirements (current phase)
- **APP-FE-001 Allergen list**: App SHALL present exactly these allergens:
  - Milk, Eggs, Peanuts, Tree Nuts, Soy, Wheat, Fish, Shellfish
- **APP-FE-002 Persisted selection**: App SHALL persist allergen checkbox state across relaunch.
- **APP-FE-003 Scan CTA behavior**: App SHALL expose a `SCAN FOOD` action with loading/disabled state while scan is running.
- **APP-FE-004 Result dialog**: App SHALL show result modal with:
  - safe state title/message
  - unsafe state title/message
  - close action
- **APP-FE-005 Unsafe culprit messaging**: For unsafe outcomes, app SHOULD reference one selected allergen when available; otherwise show a generic unsafe message.

## Bridge requirements (BLE readiness without BLE implementation yet)
- **APP-BRIDGE-001 Scan boundary**: Scan logic SHALL be isolated behind an app boundary (gateway/service function) rather than embedded across UI widgets.
- **APP-BRIDGE-002 Replaceability**: Replacing mock scan with BLE scan SHALL require no major UI rewrite.
- **APP-BRIDGE-003 Async-safe UI**: App SHALL handle scan completion only if the screen is still mounted (avoid stale updates/crashes).

## BLE integration requirements (future phase)
### Device
- **DEV-001 Advertising**: Device SHALL advertise over BLE with a recognizable name (e.g. `AllSafe-XXXX`).
- **DEV-002 Connect**: Device SHALL accept BLE connection from mobile app.
- **DEV-003 Device info**: Device SHALL expose firmware version + device identifier via readable characteristic.
- **DEV-004 Scan command**: Device SHALL accept `SCAN` command via writable characteristic.
- **DEV-005 Status and result**: Device SHALL publish status (`IDLE`, `SCANNING`, `RESULT_READY`, `ERROR`) and scan result payload.
- **DEV-006 Config**: Device SHALL support read/write configuration (threshold and calibration parameters).
- **DEV-007 Persistence**: Device SHALL persist config in NVS across power cycles.
- **DEV-008 History**: Device SHALL store bounded scan history and provide app read access.

### Protocol
- **BLE-001 Profile stability**: UUIDs and payload schema SHALL be documented and versioned.
- **BLE-002 MTU-aware transfer**: History transfer SHALL support chunking/paging.
- **BLE-003 Error model**: Errors SHALL be represented consistently.

### Mobile platform
- **PLAT-001 Android permissions**: App SHALL request required Bluetooth permissions with clear UX.
- **PLAT-002 iOS privacy strings**: App SHALL include required BLE usage descriptions and permission flow handling.
- **PLAT-003 Offline operation**: App-device operation SHALL work without internet.

## Acceptance criteria (current phase)
- Relaunching app restores previous allergen selections.
- Tapping scan disables the scan button and shows loading state.
- Scan completion shows safe/unsafe dialog with expected text.
- Unsafe result uses selected allergen in message when one or more are selected.
- Mock scan path can be swapped with another implementation via a single boundary.

## Out of scope (this phase)
- Live BLE connectivity
- Cloud sync/accounts
- OTA firmware updates
- Advanced ML classification beyond threshold-style prototype logic
