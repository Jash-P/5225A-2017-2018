/* Enumerations */
typedef enum _stopType
{
	stopNone =		0b00000000,
	stopSoft =		0b00000001,
	stopHarsh =		0b00000010
} tStopType;

/* Variables */
sVector gTargetLast;

/* Functions */
void moveToTargetSimple(float y, float x, float ys, float xs, byte power, float maxErrX, float decelEarly, byte decelPower, float dropEarly = 0, tStopType stopType = stopSoft | stopHarsh, bool slow = true);
void moveToTargetDisSimple(float a, float d, float ys, float xs, byte power, float maxErrX, float decelEarly, byte decelPower, float dropEarly = 0, tStopType stopType = stopSoft | stopHarsh, bool slow = true);
void turnToAngleNewAlg(float a, tTurnDir turnDir, float fullRatio, byte coastPower, float stopOffsetDeg, bool mogo = false);
void turnToTargetNewAlg(float y, float x, tTurnDir turnDir, float fullRatio, byte coastPower, float stopOffsetDeg, bool mogo = false, float offset = 0);
void sweepTurnToTarget(float y, float x, float a, float r, tTurnDir turnDir, byte power, bool slow = true);

MAKE_ASYNC_ONLY_MACHINE_5(autoSimple, ;,
11, (moveToTargetSimple, float, float, float, float, byte, float, float, byte, float, tStopType, bool), ;,
11, (moveToTargetDisSimple, float, float, float, float, byte, float, float, byte, float, tStopType, bool), ;,
6, (turnToAngleNewAlg, float, tTurnDir, float, byte, float, bool), ;,
8, (turnToTargetNewAlg, float, float, tTurnDir, float, byte, float, bool, float), ;,
7, (sweepTurnToTarget, float, float, float, float, tTurnDir, byte, bool), ;
)
