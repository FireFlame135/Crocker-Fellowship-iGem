import 'dart:async';
import 'dart:convert';
import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:shared_preferences/shared_preferences.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const ChompSafeApp());
}

class ChompSafeApp extends StatelessWidget {
  const ChompSafeApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ChompSafe',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: const Color(0xFF4A7C59)),
        scaffoldBackgroundColor: const Color(0xFFFAF3DD),
        useMaterial3: true,
      ),
      home: const ChompSafeHomePage(),
    );
  }
}

enum SafetyProfile {
  mostSafe,
  moreSafe,
  normal,
  lessSafe,
  leastSafe,
}

extension SafetyProfileDetails on SafetyProfile {
  String get label {
    switch (this) {
      case SafetyProfile.mostSafe:
        return 'Most safe (550)';
      case SafetyProfile.moreSafe:
        return 'More safe (625)';
      case SafetyProfile.normal:
        return 'Normal (700)';
      case SafetyProfile.lessSafe:
        return 'Less safe (775)';
      case SafetyProfile.leastSafe:
        return 'Least safe (850)';
    }
  }

  int get threshold {
    switch (this) {
      case SafetyProfile.mostSafe:
        return 550;
      case SafetyProfile.moreSafe:
        return 625;
      case SafetyProfile.normal:
        return 700;
      case SafetyProfile.lessSafe:
        return 775;
      case SafetyProfile.leastSafe:
        return 850;
    }
  }
}

class BleContract {
  static final Guid serviceUuid = Guid(
    'f0e8c930-6f61-4f8a-9f38-c224ce75f301',
  );
  static final Guid commandUuid = Guid(
    'f0e8c930-6f61-4f8a-9f38-c224ce75f303',
  );
  static final Guid statusUuid = Guid(
    'f0e8c930-6f61-4f8a-9f38-c224ce75f304',
  );
  static final Guid resultUuid = Guid(
    'f0e8c930-6f61-4f8a-9f38-c224ce75f305',
  );
  static final Guid configUuid = Guid(
    'f0e8c930-6f61-4f8a-9f38-c224ce75f306',
  );
}

class SensorReading {
  const SensorReading({required this.raw, required this.threshold});

  final int raw;
  final int threshold;
}

class BleScanGateway {
  BluetoothDevice? _device;
  BluetoothCharacteristic? _deviceInfoCharacteristic;
  BluetoothCharacteristic? _commandCharacteristic;
  BluetoothCharacteristic? _statusCharacteristic;
  BluetoothCharacteristic? _resultCharacteristic;
  BluetoothCharacteristic? _configCharacteristic;

  String _deviceInfo = 'Unknown device';

  String get deviceName => _device?.platformName ?? 'Unknown device';
  String get deviceInfo => _deviceInfo;

  Future<void> connect() async {
    if (_isReady) {
      return;
    }

    final bool supported = await FlutterBluePlus.isSupported;
    if (!supported) {
      throw Exception('Bluetooth is not supported on this device.');
    }

    final BluetoothAdapterState state = await FlutterBluePlus
        .adapterState
        .first;
    if (state != BluetoothAdapterState.on) {
      throw Exception('Bluetooth is turned off. Turn it on and try again.');
    }

    _device = await _discoverDevice();

    await _device!.connect(
      license: License.free,
      timeout: const Duration(seconds: 10),
      autoConnect: false,
    );

    final List<BluetoothService> services = await _device!.discoverServices();
    final BluetoothService service = services.firstWhere(
      (BluetoothService candidate) => candidate.uuid == BleContract.serviceUuid,
      orElse: () => throw Exception('ChompSafe BLE service not found.'),
    );

    _deviceInfoCharacteristic = _findCharacteristic(
      service,
      Guid('f0e8c930-6f61-4f8a-9f38-c224ce75f302'),
    );

    _commandCharacteristic = _findCharacteristic(
      service,
      BleContract.commandUuid,
    );
    _statusCharacteristic = _findCharacteristic(
      service,
      BleContract.statusUuid,
    );
    _resultCharacteristic = _findCharacteristic(
      service,
      BleContract.resultUuid,
    );
    _configCharacteristic = _findCharacteristic(
      service,
      BleContract.configUuid,
    );

    final List<int> infoBytes = await _deviceInfoCharacteristic!.read();
    _deviceInfo = utf8.decode(infoBytes, allowMalformed: true);

    await _statusCharacteristic!.setNotifyValue(true);
    await _resultCharacteristic!.setNotifyValue(true);
  }

  Future<void> disconnect() async {
    if (_device != null) {
      await _device!.disconnect();
    }
    _device = null;
    _deviceInfoCharacteristic = null;
    _commandCharacteristic = null;
    _statusCharacteristic = null;
    _resultCharacteristic = null;
    _configCharacteristic = null;
    _deviceInfo = 'Unknown device';
  }

  Future<void> setThreshold(int threshold) async {
    await connect();

    final Map<String, Object> payload = <String, Object>{
      'threshold': threshold,
    };

    await _configCharacteristic!.write(
      utf8.encode(jsonEncode(payload)),
      withoutResponse: false,
    );
  }

  Future<SensorReading> scan() async {
    await connect();

    final Completer<SensorReading> readingCompleter = Completer<SensorReading>();
    late final StreamSubscription<List<int>> resultSubscription;

    resultSubscription = _resultCharacteristic!.lastValueStream.listen(
      (List<int> value) {
        final String payload = utf8.decode(value, allowMalformed: true).trim();
        if (payload.isEmpty) {
          return;
        }

        try {
          final Map<String, dynamic> decoded =
              jsonDecode(payload) as Map<String, dynamic>;

          if (decoded['raw'] is! num || decoded['threshold'] is! num) {
            return;
          }

          if (!readingCompleter.isCompleted) {
            readingCompleter.complete(
              SensorReading(
                raw: (decoded['raw'] as num).toInt(),
                threshold: (decoded['threshold'] as num).toInt(),
              ),
            );
          }
        } catch (_) {
          // Ignore non-JSON or partial payloads.
        }
      },
    );

    final Map<String, String> command = <String, String>{'cmd': 'SCAN'};
    await _commandCharacteristic!.write(
      utf8.encode(jsonEncode(command)),
      withoutResponse: false,
    );

    try {
      return await readingCompleter.future.timeout(const Duration(seconds: 12));
    } finally {
      await resultSubscription.cancel();
    }
  }

  Future<BluetoothDevice> _discoverDevice() async {
    final Completer<BluetoothDevice> deviceCompleter =
        Completer<BluetoothDevice>();

    late final StreamSubscription<List<ScanResult>> scanSubscription;
    scanSubscription = FlutterBluePlus.scanResults.listen((
      List<ScanResult> results,
    ) {
      for (final ScanResult result in results) {
        final String advName = result.advertisementData.advName;
        final String platformName = result.device.platformName;
        final String name = platformName.isNotEmpty ? platformName : advName;
        if (name.contains('ChompSafe')) {
          if (!deviceCompleter.isCompleted) {
            deviceCompleter.complete(result.device);
          }
          return;
        }
      }
    });

    try {
      await FlutterBluePlus.startScan(
        withServices: <Guid>[BleContract.serviceUuid],
        timeout: const Duration(seconds: 8),
      );
      return await deviceCompleter.future.timeout(const Duration(seconds: 10));
    } finally {
      await FlutterBluePlus.stopScan();
      await scanSubscription.cancel();
    }
  }

  BluetoothCharacteristic _findCharacteristic(
    BluetoothService service,
    Guid characteristicUuid,
  ) {
    return service.characteristics.firstWhere(
      (BluetoothCharacteristic candidate) => candidate.uuid == characteristicUuid,
      orElse: () =>
          throw Exception('Missing BLE characteristic: $characteristicUuid'),
    );
  }

  bool get _isReady {
    return _device != null &&
        _commandCharacteristic != null &&
        _statusCharacteristic != null &&
        _resultCharacteristic != null &&
        _configCharacteristic != null;
  }
}

class ChompSafeHomePage extends StatefulWidget {
  const ChompSafeHomePage({super.key});

  @override
  State<ChompSafeHomePage> createState() => _ChompSafeHomePageState();
}

class _ChompSafeHomePageState extends State<ChompSafeHomePage> {
  static const List<String> _allergenNames = <String>[
    'Milk',
    'Eggs',
    'Peanuts',
    'Tree Nuts',
    'Soy',
    'Wheat',
    'Fish',
    'Shellfish',
  ];
  static const String _prefKeyPrefix = 'allergen_';

  final Random _random = Random();
  final Map<String, bool> _allergens = <String, bool>{};
  final BleScanGateway _bleGateway = BleScanGateway();

  bool _isLoadingPreferences = true;
  bool _isScanning = false;
  bool _isConnecting = false;
  bool _isBleConnected = false;
  bool _hasInitialConnectAttempted = false;
  bool _isAdvancedUserMode = false;
  bool _isWebMockFallback = kIsWeb;

  int _currentThreshold = SafetyProfile.normal.threshold;
  SafetyProfile _selectedProfile = SafetyProfile.normal;
  int? _lastRawSensorValue;
  String _connectionMessage =
      kIsWeb ? 'Web mode: BLE mock scan fallback enabled.' : 'Not connected';

  @override
  void initState() {
    super.initState();
    _loadPreferencesAndConnect();
  }

  @override
  void dispose() {
    _bleGateway.disconnect();
    super.dispose();
  }

  Future<void> _loadPreferencesAndConnect() async {
    final SharedPreferences prefs = await SharedPreferences.getInstance();
    for (final String name in _allergenNames) {
      _allergens[name] = prefs.getBool('$_prefKeyPrefix$name') ?? false;
    }
    _isAdvancedUserMode = prefs.getBool('advanced_mode') ?? false;
    if (!mounted) {
      return;
    }
    setState(() {
      _isLoadingPreferences = false;
    });

    if (!kIsWeb && !_hasInitialConnectAttempted) {
      _hasInitialConnectAttempted = true;
      unawaited(_connectBle());
    }
  }

  Future<void> _setAllergen(String name, bool selected) async {
    setState(() {
      _allergens[name] = selected;
    });
    final SharedPreferences prefs = await SharedPreferences.getInstance();
    await prefs.setBool('$_prefKeyPrefix$name', selected);
  }

  Future<void> _setAdvancedUserMode(bool selected) async {
    setState(() {
      _isAdvancedUserMode = selected;
    });
    final SharedPreferences prefs = await SharedPreferences.getInstance();
    await prefs.setBool('advanced_mode', selected);
  }

  Future<void> _connectBle() async {
    if (_isConnecting) {
      return;
    }

    setState(() {
      _isConnecting = true;
      _connectionMessage = 'Scanning for ChompSafe scanner...';
    });

    try {
      await _bleGateway.connect();
      await _bleGateway.setThreshold(_currentThreshold);
      if (!mounted) {
        return;
      }
      setState(() {
        _isBleConnected = true;
        _connectionMessage = 'Connected to ${_bleGateway.deviceName}';
      });
    } catch (error) {
      if (!mounted) {
        return;
      }
      setState(() {
        _isBleConnected = false;
        _connectionMessage = 'BLE error: $error';
        if (kIsWeb) {
          _isWebMockFallback = true;
        }
      });
    } finally {
      if (mounted) {
        setState(() {
          _isConnecting = false;
        });
      }
    }
  }

  Future<void> _setProfile(SafetyProfile? profile) async {
    if (profile == null) {
      return;
    }

    setState(() {
      _selectedProfile = profile;
      _currentThreshold = profile.threshold;
    });

    if (!_isBleConnected) {
      return;
    }

    try {
      await _bleGateway.setThreshold(_currentThreshold);
      if (!mounted) {
        return;
      }
      setState(() {
        _connectionMessage = 'Threshold synced (${_currentThreshold.toString()})';
      });
    } catch (error) {
      if (!mounted) {
        return;
      }
      setState(() {
        _connectionMessage = 'Threshold sync failed: $error';
      });
    }
  }

  Future<FoodScanResult> _runWebFallbackMockScan() async {
    await Future<void>.delayed(
      Duration(milliseconds: 1100 + _random.nextInt(700)),
    );

    final int raw = 500 + _random.nextInt(450);
    final bool safe = raw <= _currentThreshold;
    return FoodScanResult(
      isSafe: safe,
      culprit: _selectUnsafeCulpritIfNeeded(safe),
      rawSensorValue: raw,
      threshold: _currentThreshold,
    );
  }

  String? _selectUnsafeCulpritIfNeeded(bool safe) {
    if (safe) {
      return null;
    }

    final List<String> selected = _allergens.entries
        .where((MapEntry<String, bool> entry) => entry.value)
        .map((MapEntry<String, bool> entry) => entry.key)
        .toList();

    if (selected.isEmpty) {
      return null;
    }

    return selected[_random.nextInt(selected.length)];
  }

  Future<void> _scanFood() async {
    if (_isScanning || _isLoadingPreferences) {
      return;
    }

    setState(() {
      _isScanning = true;
    });

    try {
      late final FoodScanResult result;

      if (!_isBleConnected) {
        await _connectBle();
      }

      if (_isBleConnected) {
        await _bleGateway.setThreshold(_currentThreshold);
        final SensorReading reading = await _bleGateway.scan();
        result = FoodScanResult(
          isSafe: reading.raw <= reading.threshold,
          culprit: _selectUnsafeCulpritIfNeeded(reading.raw <= reading.threshold),
          rawSensorValue: reading.raw,
          threshold: reading.threshold,
        );

        if (mounted) {
          setState(() {
            _lastRawSensorValue = reading.raw;
            _connectionMessage = 'Last reading synced from ${_bleGateway.deviceName}';
          });
        }
      } else if (kIsWeb) {
        _isWebMockFallback = true;
        _connectionMessage = 'Web Bluetooth unavailable. Mock scan mode is active.';
        result = await _runWebFallbackMockScan();
      } else {
        throw Exception('ChompSafe scanner is not connected.');
      }

      if (!mounted) {
        return;
      }

      await _showResultDialog(result);
    } catch (error) {
      if (!mounted) {
        return;
      }
      setState(() {
        _connectionMessage = 'Scan failed: $error';
      });
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Scan failed: $error')),
      );
    } finally {
      if (mounted) {
        setState(() {
          _isScanning = false;
        });
      }
    }
  }

  Future<void> _showResultDialog(FoodScanResult result) async {
    final bool safe = result.isSafe;
    final Color titleColor = safe
        ? const Color(0xFF4A7C59)
        : const Color(0xFFD9534F);
    final String title = safe ? 'LIKELY SAFE' : 'WARNING: DO NOT EAT';

    String detailLine = '';
    if (_isAdvancedUserMode &&
      result.rawSensorValue != null &&
      result.threshold != null) {
      detailLine =
          '\nRaw: ${result.rawSensorValue}  Threshold: ${result.threshold}';
    }

    final String subtitle = safe
        ? 'No contaminants detected.$detailLine'
        : (result.culprit != null
              ? 'This food could potentially have: ${result.culprit}$detailLine'
              : 'This food is likely UNSAFE.$detailLine');
    final String icon = safe ? '✅' : '⚠️';

    await showDialog<void>(
      context: context,
      barrierColor: Colors.black.withValues(alpha: 0.55),
      builder: (BuildContext context) {
        return Dialog(
          backgroundColor: Colors.white,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(20),
          ),
          insetPadding: const EdgeInsets.symmetric(horizontal: 28),
          child: Padding(
            padding: const EdgeInsets.fromLTRB(24, 28, 24, 20),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: <Widget>[
                Text(icon, style: const TextStyle(fontSize: 58)),
                const SizedBox(height: 12),
                Text(
                  title,
                  textAlign: TextAlign.center,
                  style: TextStyle(
                    fontSize: 26,
                    height: 1.2,
                    fontWeight: FontWeight.w800,
                    color: titleColor,
                  ),
                ),
                const SizedBox(height: 10),
                Text(
                  subtitle,
                  textAlign: TextAlign.center,
                  style: const TextStyle(
                    fontSize: 18,
                    height: 1.35,
                    color: Color(0xFF666666),
                  ),
                ),
                const SizedBox(height: 24),
                SizedBox(
                  width: double.infinity,
                  child: FilledButton(
                    style: FilledButton.styleFrom(
                      backgroundColor: const Color(0xFFE0E0E0),
                      foregroundColor: const Color(0xFF333333),
                      textStyle: const TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(10),
                      ),
                    ),
                    onPressed: () => Navigator.of(context).pop(),
                    child: const Text('CLOSE'),
                  ),
                ),
              ],
            ),
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoadingPreferences) {
      return const Scaffold(
        body: Center(child: CircularProgressIndicator()),
      );
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text('ChompSafe'),
        backgroundColor: const Color(0xFF4A7C59),
        foregroundColor: Colors.white,
        iconTheme: const IconThemeData(color: Colors.white),
      ),
      drawer: _SettingsDrawer(
        connectionMessage: _connectionMessage,
        selectedProfile: _selectedProfile,
        onProfileChanged: _setProfile,
        isWebMode: kIsWeb,
        isBleConnected: _isBleConnected,
        isConnecting: _isConnecting,
        onConnectPressed: _connectBle,
        threshold: _currentThreshold,
        lastRawSensorValue: _lastRawSensorValue,
        deviceName: _bleGateway.deviceName,
        deviceInfo: _bleGateway.deviceInfo,
        isAdvancedUserMode: _isAdvancedUserMode,
        onAdvancedUserModeChanged: _setAdvancedUserMode,
        allergens: _allergens,
        allergenNames: _allergenNames,
        onToggleAllergen: _setAllergen,
        isWebMockFallback: _isWebMockFallback,
      ),
      body: SafeArea(
        child: Center(
          child: SingleChildScrollView(
            padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 18),
            child: ConstrainedBox(
              constraints: const BoxConstraints(maxWidth: 430),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: <Widget>[
                  const SizedBox(height: 8),
                  const _ChompSafeHeader(),
                  const SizedBox(height: 12),
                  _StatusSummaryCard(
                    connectionMessage: _connectionMessage,
                    threshold: _currentThreshold,
                    isWebMockFallback: _isWebMockFallback,
                  ),
                  const SizedBox(height: 16),
                  FilledButton(
                    onPressed: _isScanning ? null : _scanFood,
                    style: FilledButton.styleFrom(
                      backgroundColor: const Color(0xFF4A7C59),
                      disabledBackgroundColor: const Color(0xFF8DA595),
                      foregroundColor: Colors.white,
                      textStyle: const TextStyle(
                        fontSize: 20,
                        fontWeight: FontWeight.bold,
                      ),
                      padding: const EdgeInsets.symmetric(vertical: 18),
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12),
                      ),
                    ),
                    child: Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: <Widget>[
                        if (_isScanning) ...<Widget>[
                          const SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(
                              strokeWidth: 2.2,
                              color: Colors.white,
                            ),
                          ),
                          const SizedBox(width: 10),
                        ],
                        Text(_isScanning ? 'SCANNING...' : 'SCAN FOOD'),
                      ],
                    ),
                  ),
                  const SizedBox(height: 20),
                  const Text(
                    'ChompSafe Food Scanner v1.0',
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      fontSize: 12,
                      color: Color(0xFF8FC0A9),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class _StatusSummaryCard extends StatelessWidget {
  const _StatusSummaryCard({
    required this.connectionMessage,
    required this.threshold,
    required this.isWebMockFallback,
  });

  final String connectionMessage;
  final int threshold;
  final bool isWebMockFallback;

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(15),
        border: Border.all(color: const Color(0xFF8FC0A9), width: 2),
      ),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            const Text(
              'Current Status',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
                color: Color(0xFF4A7C59),
              ),
            ),
            const SizedBox(height: 8),
            Text(
              connectionMessage,
              style: const TextStyle(fontSize: 14, color: Color(0xFF4B4B4B)),
            ),
            const SizedBox(height: 4),
            Text(
              'Threshold: $threshold',
              style: const TextStyle(fontSize: 13, color: Color(0xFF666666)),
            ),
            if (isWebMockFallback) ...<Widget>[
              const SizedBox(height: 8),
              Container(
                width: double.infinity,
                padding: const EdgeInsets.all(10),
                decoration: BoxDecoration(
                  color: const Color(0xFFFFF3CD),
                  borderRadius: BorderRadius.circular(10),
                  border: Border.all(color: const Color(0xFFFFE29A)),
                ),
                child: const Text(
                  'Web Bluetooth is not active right now. Mock scan mode is being used so the app still works in the browser.',
                  style: TextStyle(fontSize: 12, color: Color(0xFF5A4D00)),
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }
}

class _SettingsDrawer extends StatelessWidget {
  const _SettingsDrawer({
    required this.connectionMessage,
    required this.selectedProfile,
    required this.onProfileChanged,
    required this.isWebMode,
    required this.isBleConnected,
    required this.isConnecting,
    required this.onConnectPressed,
    required this.threshold,
    required this.lastRawSensorValue,
    required this.deviceName,
    required this.deviceInfo,
    required this.isAdvancedUserMode,
    required this.onAdvancedUserModeChanged,
    required this.allergens,
    required this.allergenNames,
    required this.onToggleAllergen,
    required this.isWebMockFallback,
  });

  final String connectionMessage;
  final SafetyProfile selectedProfile;
  final Future<void> Function(SafetyProfile?) onProfileChanged;
  final bool isWebMode;
  final bool isBleConnected;
  final bool isConnecting;
  final Future<void> Function() onConnectPressed;
  final int threshold;
  final int? lastRawSensorValue;
  final String deviceName;
  final String deviceInfo;
  final bool isAdvancedUserMode;
  final Future<void> Function(bool selected) onAdvancedUserModeChanged;
  final Map<String, bool> allergens;
  final List<String> allergenNames;
  final Future<void> Function(String name, bool selected) onToggleAllergen;
  final bool isWebMockFallback;

  @override
  Widget build(BuildContext context) {
    return Drawer(
      child: SafeArea(
        child: ListView(
          padding: const EdgeInsets.fromLTRB(16, 12, 16, 24),
          children: <Widget>[
            const Text(
              'ChompSafe Settings',
              style: TextStyle(
                fontSize: 22,
                fontWeight: FontWeight.bold,
                color: Color(0xFF4A7C59),
              ),
            ),
            const SizedBox(height: 8),
            Text(
              connectionMessage,
              style: const TextStyle(fontSize: 13, color: Color(0xFF4B4B4B)),
            ),
            if (isWebMockFallback) ...<Widget>[
              const SizedBox(height: 8),
              const Text(
                'Mock mode is active because web Bluetooth is unavailable in this browser session.',
                style: TextStyle(fontSize: 12, color: Color(0xFF8A6D00)),
              ),
            ],
            const SizedBox(height: 16),
            SwitchListTile(
              contentPadding: EdgeInsets.zero,
              value: isAdvancedUserMode,
              title: const Text('Advanced user mode'),
              subtitle: const Text('Show raw sensor value and exact threshold.'),
              onChanged: onAdvancedUserModeChanged,
            ),
            const Divider(),
            _ConnectionCard(
              connectionMessage: connectionMessage,
              selectedProfile: selectedProfile,
              onProfileChanged: onProfileChanged,
              isWebMode: isWebMode,
              isBleConnected: isBleConnected,
              isConnecting: isConnecting,
              onConnectPressed: onConnectPressed,
              threshold: threshold,
              lastRawSensorValue: lastRawSensorValue,
              deviceName: deviceName,
              deviceInfo: deviceInfo,
            ),
            const SizedBox(height: 12),
            _AllergenCard(
              allergenNames: allergenNames,
              allergens: allergens,
              onToggle: onToggleAllergen,
            ),
          ],
        ),
      ),
    );
  }
}

class _ConnectionCard extends StatelessWidget {
  const _ConnectionCard({
    required this.connectionMessage,
    required this.selectedProfile,
    required this.onProfileChanged,
    required this.isWebMode,
    required this.isBleConnected,
    required this.isConnecting,
    required this.onConnectPressed,
    required this.threshold,
    required this.lastRawSensorValue,
    required this.deviceName,
    required this.deviceInfo,
  });

  final String connectionMessage;
  final SafetyProfile selectedProfile;
  final Future<void> Function(SafetyProfile?) onProfileChanged;
  final bool isWebMode;
  final bool isBleConnected;
  final bool isConnecting;
  final Future<void> Function() onConnectPressed;
  final int threshold;
  final int? lastRawSensorValue;
  final String deviceName;
  final String deviceInfo;

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(15),
        border: Border.all(color: const Color(0xFF8FC0A9), width: 2),
      ),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 14, 16, 14),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            const Text(
              'Scanner Connection',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
                color: Color(0xFF4A7C59),
              ),
            ),
            const SizedBox(height: 10),
            Text(
              connectionMessage,
              style: const TextStyle(fontSize: 14, color: Color(0xFF4B4B4B)),
            ),
            const SizedBox(height: 8),
            Text(
              'Device: $deviceName',
              style: const TextStyle(fontSize: 13, color: Color(0xFF666666)),
            ),
            Text(
              deviceInfo,
              style: const TextStyle(fontSize: 13, color: Color(0xFF666666)),
            ),
            const SizedBox(height: 10),
            DropdownButtonFormField<SafetyProfile>(
              initialValue: selectedProfile,
              decoration: const InputDecoration(
                labelText: 'Safety profile',
                border: OutlineInputBorder(),
              ),
              items: SafetyProfile.values
                  .map(
                    (SafetyProfile profile) => DropdownMenuItem<SafetyProfile>(
                      value: profile,
                      child: Text(profile.label),
                    ),
                  )
                  .toList(),
              onChanged: (SafetyProfile? profile) {
                onProfileChanged(profile);
              },
            ),
            const SizedBox(height: 8),
            Text(
              'Active threshold: $threshold',
              style: const TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.w600,
                color: Color(0xFF555555),
              ),
            ),
            if (lastRawSensorValue != null)
              Text(
                'Last raw reading: $lastRawSensorValue',
                style: const TextStyle(
                  fontSize: 13,
                  color: Color(0xFF666666),
                ),
              ),
            const SizedBox(height: 12),
            if (!isWebMode)
              SizedBox(
                width: double.infinity,
                child: OutlinedButton(
                  onPressed: isConnecting ? null : onConnectPressed,
                  child: Text(
                    isConnecting
                        ? 'CONNECTING...'
                        : (isBleConnected
                              ? 'RECONNECT SCANNER'
                              : 'CONNECT SCANNER'),
                  ),
                ),
              )
            else
              const Text(
                'Web currently uses mock scan mode so Android BLE remains stable.',
                style: TextStyle(fontSize: 13, color: Color(0xFF666666)),
              ),
          ],
        ),
      ),
    );
  }
}

class _ChompSafeHeader extends StatelessWidget {
  const _ChompSafeHeader();

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        const Icon(
          Icons.shield_moon_rounded,
          size: 72,
          color: Color(0xFF68B0AB),
        ),
        const SizedBox(height: 6),
        const Text(
          'ChompSafe',
          textAlign: TextAlign.center,
          style: TextStyle(
            fontSize: 48,
            fontWeight: FontWeight.w700,
            letterSpacing: -1.0,
            color: Color(0xFF4A7C59),
          ),
        ),
      ],
    );
  }
}

class _AllergenCard extends StatelessWidget {
  const _AllergenCard({
    required this.allergenNames,
    required this.allergens,
    required this.onToggle,
  });

  final List<String> allergenNames;
  final Map<String, bool> allergens;
  final Future<void> Function(String name, bool selected) onToggle;

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(15),
        border: Border.all(color: const Color(0xFF8FC0A9), width: 2),
        boxShadow: const <BoxShadow>[
          BoxShadow(
            color: Color(0x1A4A7C59),
            blurRadius: 10,
            offset: Offset(0, 4),
          ),
        ],
      ),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 14, 16, 8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            const Text(
              'Select Your Allergens',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
                color: Color(0xFF4A7C59),
              ),
            ),
            const SizedBox(height: 8),
            const Divider(
              color: Color(0xFFC8D5B9),
              thickness: 2,
              height: 8,
            ),
            ...allergenNames.map((String name) {
              final bool selected = allergens[name] ?? false;
              return Container(
                decoration: const BoxDecoration(
                  border: Border(
                    bottom: BorderSide(color: Color(0xFFF0F0F0), width: 1),
                  ),
                ),
                child: CheckboxListTile(
                  controlAffinity: ListTileControlAffinity.leading,
                  activeColor: const Color(0xFF4A7C59),
                  dense: false,
                  contentPadding: const EdgeInsets.symmetric(horizontal: 2),
                  value: selected,
                  title: Text(
                    name,
                    style: const TextStyle(
                      fontSize: 18,
                      color: Color(0xFF555555),
                    ),
                  ),
                  onChanged: (bool? value) {
                    onToggle(name, value ?? false);
                  },
                ),
              );
            }),
          ],
        ),
      ),
    );
  }
}

class FoodScanResult {
  const FoodScanResult({
    required this.isSafe,
    this.culprit,
    this.rawSensorValue,
    this.threshold,
  });

  final bool isSafe;
  final String? culprit;
  final int? rawSensorValue;
  final int? threshold;
}
