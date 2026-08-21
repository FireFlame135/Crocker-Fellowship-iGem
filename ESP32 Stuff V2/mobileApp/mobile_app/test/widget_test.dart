import 'package:flutter_test/flutter_test.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:mobile_app/main.dart';

void main() {
  testWidgets('AllSafe shell renders expected primary UI', (
    WidgetTester tester,
  ) async {
    SharedPreferences.setMockInitialValues(<String, Object>{});
    await tester.pumpWidget(const AllSafeApp());
    await tester.pumpAndSettle();

    expect(find.text('Select Your Allergens'), findsOneWidget);
    expect(find.text('SCAN FOOD'), findsOneWidget);
    expect(find.text('AllSafe Food Scanner v1.0'), findsOneWidget);
  });
}
