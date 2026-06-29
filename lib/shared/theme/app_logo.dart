import 'package:flutter/material.dart';

class AppLogo extends StatelessWidget {
  final double size;
  final bool showText;

  const AppLogo({
    super.key,
    this.size = 40.0,
    this.showText = false,
  });

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        SizedBox(
          width: size,
          height: size,
          child: CustomPaint(
            painter: _LogoPainter(),
          ),
        ),
        if (showText) ...[
          const SizedBox(width: 12),
          Text(
            'CodeStudio',
            style: TextStyle(
              fontSize: size * 0.5,
              fontWeight: FontWeight.bold,
              color: Colors.white,
              letterSpacing: -0.5,
            ),
          ),
          Text(
            'Recorder',
            style: TextStyle(
              fontSize: size * 0.5,
              fontWeight: FontWeight.w300,
              color: const Color(0xFFFF3B3B),
              letterSpacing: -0.5,
            ),
          ),
        ],
      ],
    );
  }
}

class _LogoPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final center = Offset(size.width / 2, size.height / 2);
    final radius = size.width / 2;

    // 1. Draw stylized hexagon background
    final bgPaint = Paint()
      ..color = const Color(0xFF1A1A1A)
      ..style = PaintingStyle.fill;
    
    final path = Path();
    for (int i = 0; i < 6; i++) {
      double angle = (i * 60) * (3.14159 / 180);
      double x = center.dx + radius * 0.9 * (i % 2 == 0 ? 1 : 0.85) * (i % 3 == 0 ? 1 : 1); // stylized
      // Hexagon math
      double hx = center.dx + radius * 0.9 * (i == 0 || i == 3 ? 1.0 : 0.8) * (i == 1 || i == 2 || i == 4 || i == 5 ? 1 : 1);
    }
    
    // Simpler modern design: Circle with bracket wings
    final circlePaint = Paint()
      ..color = const Color(0xFFFF3B3B)
      ..style = PaintingStyle.fill;
    
    canvas.drawCircle(center, radius * 0.35, circlePaint);

    final bracketPaint = Paint()
      ..color = Colors.white
      ..style = PaintingStyle.stroke
      ..strokeWidth = radius * 0.12
      ..strokeCap = StrokeCap.round;

    // Left bracket <
    final leftPath = Path()
      ..moveTo(center.dx - radius * 0.5, center.dy - radius * 0.45)
      ..lineTo(center.dx - radius * 0.85, center.dy)
      ..lineTo(center.dx - radius * 0.5, center.dy + radius * 0.45);
    
    canvas.drawPath(leftPath, bracketPaint);

    // Right bracket >
    final rightPath = Path()
      ..moveTo(center.dx + radius * 0.5, center.dy - radius * 0.45)
      ..lineTo(center.dx + radius * 0.85, center.dy)
      ..lineTo(center.dx + radius * 0.5, center.dy + radius * 0.45);
    
    canvas.drawPath(rightPath, bracketPaint);
    
    // Add a small pulse effect ring
    final pulsePaint = Paint()
      ..color = const Color(0xFFFF3B3B).withOpacity(0.3)
      ..style = PaintingStyle.stroke
      ..strokeWidth = radius * 0.05;
    
    canvas.drawCircle(center, radius * 0.5, pulsePaint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
