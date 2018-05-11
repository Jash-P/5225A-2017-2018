void setupJoysticks()
{
	for (TVexJoysticks i = (TVexJoysticks)0; i < kNumbOfVexRFIndices; ++i)
	{
		gJoy[i].cur = gJoy[i].lst = 0;
		gJoy[i].deadzone = -1;
		gJoy[i].enabled = false;
		gJoy[i].partner = -1;
	}
}

void updateJoysticks()
{
	for (TVexJoysticks i = (TVexJoysticks)0; i < kNumbOfVexRFIndices; ++i)
		if (gJoy[i].enabled)
			updateJoystick(i);
}

void updateJoystick(TVexJoysticks joy)
{
	gJoy[joy].lst = gJoy[joy].cur;
	short val = vexRT[joy];
	if (gJoy[joy].partner != -1)
	{
		if (abs(val) > gJoy[joy].deadzone && !gJoy[joy].lst)
			gKiddieControl = false;
		if (gKiddieControl)
			val = vexRT[gJoy[joy].partner];
	}
	gJoy[joy].cur = abs(val) > gJoy[joy].deadzone ? val : 0;
}

void enableJoystick(TVexJoysticks joy, TVexJoysticks partner)
{
	gJoy[joy].enabled = true;
	gJoy[joy].partner = partner;
}
