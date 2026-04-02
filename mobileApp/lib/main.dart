import 'dart:math';

import 'package:flutter/material.dart';
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
  bool _isLoadingPreferences = true;
  bool _isScanning = false;

  @override
  void initState() {
    super.initState();
    _loadAllergens();
  }

  Future<void> _loadAllergens() async {
    final SharedPreferences prefs = await SharedPreferences.getInstance();
    for (final String name in _allergenNames) {
      _allergens[name] = prefs.getBool('$_prefKeyPrefix$name') ?? false;
    }
    if (!mounted) {
      return;
    }
    setState(() {
      _isLoadingPreferences = false;
    });
  }

  Future<void> _setAllergen(String name, bool selected) async {
    setState(() {
      _allergens[name] = selected;
    });
    final SharedPreferences prefs = await SharedPreferences.getInstance();
    await prefs.setBool('$_prefKeyPrefix$name', selected);
  }

  Future<ScanResult> _runMockScan() async {
    await Future<void>.delayed(
      Duration(milliseconds: 1200 + _random.nextInt(900)),
    );

    final bool isSafe = _random.nextBool();
    String? culprit;
    if (!isSafe) {
      final List<String> selected = _allergens.entries
          .where((MapEntry<String, bool> entry) => entry.value)
          .map((MapEntry<String, bool> entry) => entry.key)
          .toList();
      if (selected.isNotEmpty) {
        culprit = selected[_random.nextInt(selected.length)];
      }
    }

    return ScanResult(isSafe: isSafe, culprit: culprit);
  }

  Future<void> _scanFood() async {
    if (_isScanning || _isLoadingPreferences) {
      return;
    }
    setState(() {
      _isScanning = true;
    });
    final ScanResult result = await _runMockScan();
    if (!mounted) {
      return;
    }
    setState(() {
      _isScanning = false;
    });
    await _showResultDialog(result);
  }

  Future<void> _showResultDialog(ScanResult result) async {
    final bool safe = result.isSafe;
    final Color titleColor = safe
        ? const Color(0xFF4A7C59)
        : const Color(0xFFD9534F);
    final String title = safe ? 'LIKELY SAFE' : 'WARNING: DO NOT EAT';
    final String subtitle = safe
        ? 'No contaminants detected.'
        : (result.culprit != null
              ? 'This food could potentially have: ${result.culprit}'
              : 'This food is likely UNSAFE.');
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
                  _AllergenCard(
                    allergenNames: _allergenNames,
                    allergens: _allergens,
                    onToggle: _setAllergen,
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
        RichText(
          text: const TextSpan(
            style: TextStyle(
              fontSize: 48,
              fontWeight: FontWeight.w700,
              letterSpacing: -1.0,
            ),
            children: <TextSpan>[
              TextSpan(text: 'All', style: TextStyle(color: Color(0xFF68B0AB))),
              TextSpan(text: 'Safe', style: TextStyle(color: Color(0xFF4B7C59))),
            ],
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

class ScanResult {
  const ScanResult({required this.isSafe, this.culprit});

  final bool isSafe;
  final String? culprit;
}
