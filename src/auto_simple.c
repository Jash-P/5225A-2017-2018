/* Functions */
void moveToTargetSimple(float y, float x, float ys, float xs, byte power, float dropEarly, tStopType stopType, bool slow)
{
	writeDebugStreamLine("Moving to %f %f from %f %f at %d", y, x, ys, xs, power);

	sPID pidY;
	pidInit(pidY, 0.12, 0.005, 0.0, 0.5, 1.5, -1, 1.0);

	gTargetLast.y = y;
	gTargetLast.x = x;

	// Create the line to follow
	sLine followLine;

	// Start points
	followLine.p1.y = ys;
	followLine.p1.x = xs;

	// End points
	followLine.p2.y = y;
	followLine.p2.x = x;

	float lineLength = getLengthOfLine(followLine);
	writeDebugStreamLine("Line length: %.2f", lineLength);
	float lineAngle = getAngleOfLine(followLine); // Get the angle of the line that we're following relative to the vertical
	float pidAngle = nearAngle(lineAngle - (power < 0 ? PI : 0), gPosition.a);
	writeDebugStreamLine("Line | Pid angle: %f | %f", radToDeg(lineAngle), radToDeg(pidAngle));

	// Current position relative to the ending point
	sVector currentPosVector;
	sPolar currentPosPolar;

	sCycleData cycle;
	initCycle(cycle, 10, "moveToTarget");

	float vel;
	float _sin = sin(lineAngle);
	float _cos = cos(lineAngle);

	word last = 0;

	if (!slow)
		setDrive(power, power);

	unsigned long timeStart = nPgmTime;
	do
	{
		currentPosVector.x = gPosition.x - x;
		currentPosVector.y = gPosition.y - y;
		vectorToPolar(currentPosVector, currentPosPolar);
		currentPosPolar.angle += lineAngle;
		polarToVector(currentPosPolar, currentPosVector);

		if (slow)
		{
			pidCalculate(pidY, 0, currentPosVector.y);
			word finalPower = round(power * abs(pidY.output));
			if (abs(finalPower) < 30) finalPower = 30 * sgn(finalPower);
			word delta = finalPower - last;
			LIM_TO_VAL_SET(delta, 5);
			finalPower = last += delta;
			setDrive(finalPower, finalPower);
		}

		vel = _sin * gVelocity.x + _cos * gVelocity.y;

		endCycle(cycle);
	} while (currentPosVector.y < -dropEarly - (vel * ((stopType & stopSoft) ? 0.138 : 0.098)));

	writeDebugStreamLine("%f %f", currentPosVector.y, vel);

	if (stopType & stopSoft)
	{
		setDrive(-6, -6);
		while (_sin * gVelocity.x + _cos * gVelocity.y > 15) sleep(1);
	}

	if (stopType & stopHarsh)
		applyHarshStop();
	else
		setDrive(0, 0);

	writeDebugStreamLine("Moved to %f %f from %f %f | %f %f %f", y, x, ys, xs, gPosition.y, gPosition.x, radToDeg(gPosition.a));
}

void moveToTargetSimple(float y, float x, byte power, float dropEarly, tStopType stopType, bool slow) { moveToTargetSimple(y, x, gPosition.y, gPosition.x, power, dropEarly, stopType, slow); }

void moveToTargetDisSimple(float a, float d, float ys, float xs, byte power, float dropEarly, tStopType stopType, bool slow)
{
	//sPolar polar;
	//sVector vector;
	//polar.angle = a;
	//polar.magnitude = d;
	//polarToVector(polar, vector);
	//moveToTargetSimple(vector.y + ys, vector.x + xs, ys, xs, power, dropEarly, stopType, slow);
	moveToTargetSimple(ys + d * cos(a), xs + d * sin(a), ys, xs, power, dropEarly, stopType, slow);
}

void moveToTargetDisSimple(float a, float d, byte power, float dropEarly, tStopType stopType, bool slow) { moveToTargetDisSimple(a, d, gPosition.y, gPosition.x, power, dropEarly, stopType, slow); }

void turnToAngleRadSimple(float a, tTurnDir turnDir, byte left, byte right)
{
	writeDebugStreamLine("Turning to %f", radToDeg(a));
	sTurnState state;
	state.time = nPgmTime;
	state.lstTime = state.time;
	state.nextDebug = 0;
	state.input = gVelocity.a;
	state.power = state.error = state.integral = 0;

	left = abs(left);
	right = abs(right);

	if (turnDir == ch)
		if (a < gPosition.a) turnDir = ccw; else turnDir = cw;

	switch (turnDir)
	{
	case cw:
		a = gPosition.a + fmod(a - gPosition.a, PI * 2);
		writeDebugStreamLine("%f", a);
		setDrive(left, -right);
		while (gPosition.a < a - gVelocity.a * 0.6) sleep(1);
		writeDebugStreamLine("%f", gVelocity.a);
		//
		state.target = 0.900;
		while (gPosition.a < a + degToRad(-5.3 + (state.target - gVelocity.a) * 0.3))
			turnSimpleInternalCw(a, state);
		setDrive(-15, 15);
		sleep(150);
		setDrive(0, 0);
		break;
	case ccw:
		a = gPosition.a - fmod(gPosition.a - a, PI * 2);
		writeDebugStreamLine("%f", a);
		setDrive(-left, right);
		while (gPosition.a > a - gVelocity.a * 0.6) sleep(1);
		writeDebugStreamLine("%f", gVelocity.a);
		//
		state.target = -0.900;
		while (gPosition.a > a - degToRad(-5.3 + (state.target - gVelocity.a) * 0.3))
			turnSimpleInternalCcw(a, state);
		setDrive(15, -15);
		sleep(150);
		setDrive(0, 0);
		break;
	}

	writeDebugStreamLine("Turned to %f | %f %f %f", radToDeg(a), gPosition.y, gPosition.x, radToDeg(gPosition.a));
}

void turnToAngleSimple(float a, tTurnDir turnDir, byte left, byte right)
{
	turnToAngleRadSimple(degToRad(a), turnDir, left, right);
}

void turnToTargetSimple(float y, float x, tTurnDir turnDir, byte left, byte right, float offset)
{
	writeDebugStreamLine("Turning to %f %f", y, x);
	sTurnState state;
	state.time = nPgmTime;
	state.lstTime = state.time;
	state.nextDebug = 0;
	state.input = gVelocity.a;
	state.power = state.error = state.integral = 0;

	left = abs(left);
	right = abs(right);

	float a = atan2(x - gPosition.x, y - gPosition.y) + offset;

	if (turnDir == ch)
		if (fmod(a - gPosition.a, PI * 2) > PI) turnDir = ccw; else turnDir = cw;

	switch (turnDir)
	{
	case cw:
		a = gPosition.a + fmod(a - gPosition.a, PI * 2);
		writeDebugStreamLine("%f", a);
		setDrive(left, -right, true);
		while (gPosition.a < a - gVelocity.a * 0.6)
		{
			a = gPosition.a + fmod(atan2(x - gPosition.x, y - gPosition.y) + offset - gPosition.a, PI * 2);
			sleep(1);
		}
		writeDebugStreamLine("%f", gVelocity.a);
		//
		state.target = 0.900;
		while (gPosition.a < a + degToRad(-5.3 + (state.target - gVelocity.a) * 0.3))
		{
			a = gPosition.a + fmod(atan2(x - gPosition.x, y - gPosition.y) + offset - gPosition.a, PI * 2);
			turnSimpleInternalCw(a, state);
		}
		setDrive(-15, 15);
		sleep(150);
		setDrive(0, 0);
		break;
	case ccw:
		a = gPosition.a - fmod(gPosition.a - a, PI * 2);
		writeDebugStreamLine("%f", a);
		setDrive(-left, right, true);
		while (gPosition.a > a - gVelocity.a * 0.6)
		{
			a = gPosition.a - fmod(gPosition.a - atan2(x - gPosition.x, y - gPosition.y) - offset, PI * 2);
			sleep(1);
		}
		writeDebugStreamLine("%f", gVelocity.a);
		//
		state.target = -0.900;
		while (gPosition.a > a - degToRad(-5.3 + (state.target - gVelocity.a) * 0.3))
		{
			a = gPosition.a - fmod(gPosition.a - atan2(x - gPosition.x, y - gPosition.y) - offset, PI * 2);
			turnSimpleInternalCcw(a, state);
		}
		setDrive(15, -15);
		sleep(150);
		setDrive(0, 0);
		break;
	}

	writeDebugStreamLine("Turned to %f %f | %f %f %f", y, x, gPosition.y, gPosition.x, radToDeg(gPosition.a));
}

void turnSimpleInternalCw(float a, sTurnState& state)
{
	unsigned long deltaTime = state.time - state.lstTime;
	float vel = gVelocity.a;

	const float kP = 17, kI = 0.05;

	if (deltaTime >= 1)
	{
		state.input = vel;

		state.lstError = state.error;
		state.error = state.target - state.input;

		state.integral += state.error * deltaTime;
		if (state.integral < -8) state.integral = 0;

		state.power = state.error * kP + state.integral * kI;

		state.power += 26;

		if (state.power < 0) state.power /= 6.0;

		if (state.power > 50) state.power = 50;
		if (state.power < -5) state.power = -5;

		setDrive(state.power, -state.power);

		if (state.time >= state.nextDebug)
		{
			writeDebugStreamLine("%f %f %f", state.power, state.error, state.integral);
			state.nextDebug = state.time + 20;

		}

		state.lstTime = state.time;
	}

	sleep(1);

	state.time = nPgmTime;
}

void turnSimpleInternalCcw(float a, sTurnState& state)
{
	unsigned long deltaTime = state.time - state.lstTime;
	float vel = gVelocity.a;

	const float kP = 17, kI = 0.05;

	if (deltaTime >= 1)
	{
		state.input = vel;

		state.lstError = state.error;
		state.error = state.target - state.input;

		state.integral += state.error * deltaTime;
		if (state.integral > 8) state.integral = 0;

		state.power = state.error * kP + state.integral * kI;

		state.power -= 26;

		if (state.power > 0) state.power /= 6.0;

		if (state.power < -50) state.power = -50;
		if (state.power > 5) state.power = 5;

		setDrive(state.power, -state.power);

		if (state.time >= state.nextDebug)
		{
			writeDebugStreamLine("%f %f %f", state.power, state.error, state.integral);
			state.nextDebug = state.time + 20;

		}

		state.lstTime = state.time;
	}

	sleep(1);

	state.time = nPgmTime;
}

void turnToAngleStupid(float a, tTurnDir turnDir)
{
	if (turnDir == ch)
		if (a < gPosition.a) turnDir = ccw; else turnDir = cw;

	switch (turnDir)
	{
	case cw:
		a = gPosition.a + fmod(a - gPosition.a, PI * 2);
		setDrive(30, -30);
		while (gPosition.a < a - 0.04) sleep(5);
		break;
	case ccw:
		a = gPosition.a - fmod(gPosition.a - a, PI * 2);
		setDrive(-30, 30);
		while (gPosition.a > a + 0.04) sleep(5);
		break;
	}
	applyHarshStop();
}

void turnToTargetStupid(float y, float x, tTurnDir turnDir, float offset)
{
	float a;
	if (turnDir == ch)
		if (fmod(atan2(x - gPosition.x, y - gPosition.y) + offset - gPosition.a, PI * 2) > PI) turnDir = ccw; else turnDir = cw;

	switch (turnDir)
	{
	case cw:
		writeDebugStreamLine("CW %d %d | %d", y, x, a);
		setDrive(25, -25);
		do
		{
			a = gPosition.a + fmod(atan2(x - gPosition.x, y - gPosition.y) - gPosition.a + offset, PI * 2);
			sleep(5);
		} while (gPosition.a < a - 0.04);
		break;
	case ccw:
		writeDebugStreamLine("CCW %d %d | %d", y, x, a);
		setDrive(-25, 25);
		do
		{
			a = gPosition.a - fmod(gPosition.a - atan2(x - gPosition.x, y - gPosition.y) - offset, PI * 2);
			sleep(5);
		} while (gPosition.a > a + 0.04);
		break;
	}
	applyHarshStop();
	writeDebugStreamLine("Turned to %f %f | %f | %f %f %f", y, x, radToDeg(a), gPosition.y, gPosition.x, radToDeg(gPosition.a));
}

void turnToAngleNewRad (float a, tTurnDir turnDir)
{
	writeDebugStreamLine("Turning to %f", radToDeg(a));

	const float startPower = 35;

	if (turnDir == cw)
	{
		a = gPosition.a + fmod(a - gPosition.a, PI * 2);
		setDrive(startPower, -startPower);
	}
	else
	{
		a = gPosition.a - fmod(gPosition.a - a, PI * 2);
		setDrive(-startPower, startPower);
	}

	float targetVel;
	float output;
	float integral = 0;

	bool isLong = fabs(gPosition.a - a) >= PI;

	float kP_vel = isLong ? 4.5 : 3.5;
	float kP_pwr = isLong ? 27 : 30;
	float kI_pwr = isLong ? 0.003 : 0.004;
	float kD_pwr = isLong ? 0.5 : 0.7;

	float lstErr;

	float lstErrPos;

	unsigned long time = nPgmTime;
	unsigned long lstTime = time;
	unsigned long startTime = time;

	while (fabs(gPosition.a - a) > 0.02)
	{
		time = nPgmTime;
		unsigned long deltaTime = time - lstTime;

		float errPos = a - gPosition.a;
		errPos = sgn(errPos) * sqrt(fabs(errPos));
		targetVel = kP_vel * errPos;

		if (deltaTime >= 1)
		{
			float err = targetVel - gVelocity.a;
			integral += deltaTime * err;
			output = kP_pwr * err + kI_pwr * integral + kD_pwr * (lstErr - err) / deltaTime;
			LIM_TO_VAL_SET(output, isLong ? 127 : 50);
			if (errPos > 0 && lstErrPos < 0) playSound(soundUpwardTones);
			if (errPos < 0 && lstErrPos > 0) playSound(soundDownwardTones);
			lstTime = time;
			lstErr = err;
			lstErrPos = errPos;

			if (time - startTime > 100 || fabs(output) > startPower)
				setDrive(output, -output);
		}
		sleep(1);
	}

	applyHarshStop();

	writeDebugStreamLine("Turned to %f | %f %f %f", radToDeg(a), gPosition.y, gPosition.x, radToDeg(gPosition.a));
}

void turnToAngleNew(float a, tTurnDir turnDir)
{
	turnToAngleNewRad(degToRad(a), turnDir);
}
