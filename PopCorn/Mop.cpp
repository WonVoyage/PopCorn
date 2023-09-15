#include "Mop.h"

// AsMop
//------------------------------------------------------------------------------------------------------------
AsMop::~AsMop()
{
	for (auto *indicator : Mop_Indicators)
		delete indicator;

	Mop_Indicators.erase(Mop_Indicators.begin(), Mop_Indicators.end() );

	for (auto *cylinder : Mop_Cylinders)
		delete cylinder;

	Mop_Cylinders.erase(Mop_Cylinders.begin(), Mop_Cylinders.end() );
}
//------------------------------------------------------------------------------------------------------------
AsMop::AsMop()
	: Acting(false), Y_Pos(0), Start_Tick(0)
{
	int i;
	int x_pos, y_pos;
	AMop_Indicator *indicator;
	AMop_Cylinder *cylinder;

	for (i = 0; i < 10; i++)
	{
		indicator = new AMop_Indicator(AsConfig::Level_X_Offset + 1 + i * 19, AsConfig::Level_Y_Offset + 1, i * 80);
		Mop_Indicators.push_back(indicator);
	}

	for (i = 0; i < 4; i++)
	{
		x_pos = AsConfig::Level_X_Offset + Width / 2 - 6 - i;
		y_pos = AsConfig::Level_Y_Offset + 7 + i * 5;

		cylinder = new AMop_Cylinder(x_pos, y_pos, 13 + i * 2, AMop_Cylinder::Max_Cylinder_Height[i]);
		Mop_Cylinders.push_back(cylinder);
	}
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Begin_Movement()
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Finish_Movement()
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Advance(double max_speed)
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
double AsMop::Get_Speed()
{
	return 0.0;  //!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Act()
{
	int time_offset;
	double ratio;

	if (! Acting)
		return;

	Prev_Mop_Rect = Mop_Rect;

	time_offset = AsConfig::Current_Timer_Tick - Start_Tick;

	if (time_offset <= Expansion_Timeout)
	{
		ratio = (double)time_offset / (double)Expansion_Timeout;

		for (auto *cylinder : Mop_Cylinders)
			cylinder->Set_Height_For(ratio);

		Set_Mop();
	}

	for (auto *indicator : Mop_Indicators)
		indicator->Act();

	AsTools::Invalidate_Rect(Mop_Rect);
	AsTools::Invalidate_Rect(Prev_Mop_Rect);
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Clear(HDC hdc, RECT &paint_area)
{
	RECT intersection_rect;

	if (! Acting)
		return;

	if (! IntersectRect(&intersection_rect, &paint_area, &Prev_Mop_Rect) )
		return;

	AsTools::Rect(hdc, Prev_Mop_Rect, AsConfig::BG_Color);

	for (auto *cylinder : Mop_Cylinders)
		cylinder->Clear(hdc, paint_area);
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Draw(HDC hdc, RECT &paint_area)
{
	if (! Acting)
		return;

	AsTools::Rect(hdc, AsConfig::Level_X_Offset, Y_Pos, Width, Height, AsConfig::Red_Color);

	for (auto *indicator : Mop_Indicators)
		indicator->Draw(hdc, paint_area);

	for (auto *cylinder : Mop_Cylinders)
		cylinder->Draw(hdc, paint_area);
}
//------------------------------------------------------------------------------------------------------------
bool AsMop::Is_Finished()
{
	return false;  //!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Erase_Level()
{
	Start_Tick = AsConfig::Current_Timer_Tick;
	Y_Pos = 172;
	Acting = true;

	Set_Mop();
}
//------------------------------------------------------------------------------------------------------------
void AsMop::Set_Mop()
{
	int i;
	int curr_y_pos = 0;
	int total_cylinder_height = 0;
	const int scale = AsConfig::Global_Scale;

	for (auto *cylinder : Mop_Cylinders)
		total_cylinder_height += cylinder->Get_Height();

	Y_Pos = AsConfig::Max_Y_Pos - total_cylinder_height - Height + 1;

	for (auto *indicator : Mop_Indicators)
		indicator->Set_Y_Pos(Y_Pos + 1);

	for (i = 0; i < (int)Mop_Cylinders.size(); i++)
	{
		Mop_Cylinders[i]->Set_Y_Pos(Y_Pos + Height + curr_y_pos);

		curr_y_pos += Mop_Cylinders[i]->Get_Height();
	}

	Mop_Rect.left = AsConfig::Level_X_Offset * scale;
	Mop_Rect.top = Y_Pos * scale;
	Mop_Rect.right = Mop_Rect.left + Width * scale;
	Mop_Rect.bottom = Mop_Rect.top + Height * scale;
}
//------------------------------------------------------------------------------------------------------------
