﻿#include "Ball.h"

//------------------------------------------------------------------------------------------------------------
bool AHit_Checker::Hit_Circle_On_Line(double y, double next_x_pos, double left_x, double right_x, double radius, double &x)
{// Проверяет пересечение горизонтального отрезка (проходящего от left_x до right_x через y) с окружностью радиусом radius

	double min_x, max_x;

	// x * x + y * y = R * R
	// x = sqrt(R * R - y * y)
	// y = sqrt(R * R - x * x)

	if (y > radius)
		return false;

	x = sqrt(radius * radius - y * y);

	max_x = next_x_pos + x;
	min_x = next_x_pos - x;

	if (max_x >= left_x && max_x <= right_x  ||  min_x >= left_x && min_x <= right_x)
		return true;
	else
		return false;
}
//------------------------------------------------------------------------------------------------------------




// ABall
const double ABall::Start_Ball_Y_Pos = 184.0;
const double ABall::Radius = 2.0 - 0.5 / AsConfig::Global_Scale;
int ABall::Hit_Checkers_Count = 0;
AHit_Checker *ABall::Hit_Checkers[] = {};
//------------------------------------------------------------------------------------------------------------
ABall::ABall()
: Ball_State(EBS_Normal), Prev_Ball_State(EBS_Normal), Center_X_Pos(0.0), Center_Y_Pos(Start_Ball_Y_Pos), Ball_Speed(0.0),
  Rest_Distance(0.0), Ball_Direction(0), Testing_Is_Active(false), Test_Iteration(0), Ball_Rect{}, Prev_Ball_Rect{}
{
	//Set_State(EBS_Normal, 0);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Draw(HDC hdc, RECT &paint_area)
{
	RECT intersection_rect;

	if ( (Ball_State == EBS_Teleporting || Ball_State == EBS_Lost) && Ball_State == Prev_Ball_State)
		return;

	// 1. Очищаем фон
	if (IntersectRect(&intersection_rect, &paint_area, &Prev_Ball_Rect) )
	{
		AsConfig::BG_Color.Select(hdc);
		Ellipse(hdc, Prev_Ball_Rect.left, Prev_Ball_Rect.top, Prev_Ball_Rect.right - 1, Prev_Ball_Rect.bottom - 1);
	}

	switch (Ball_State)
	{
	case EBS_On_Parachute:
		Draw_Parachute(hdc, paint_area);
		break;

	case EBS_Off_Parachute:
		Clear_Parachute(hdc);
		Set_State(EBS_Normal, Center_X_Pos, Center_Y_Pos);
		break;

	case EBS_Lost:
		if (Prev_Ball_State == EBS_On_Parachute)
			Clear_Parachute(hdc);
		return;

	case EBS_Teleporting:
		return;
	}

	// 2. Рисуем шарик
	if (IntersectRect(&intersection_rect, &paint_area, &Ball_Rect) )
	{
		AsConfig::White_Color.Select(hdc);
		Ellipse(hdc, Ball_Rect.left, Ball_Rect.top, Ball_Rect.right - 1, Ball_Rect.bottom - 1);
	}
}
//------------------------------------------------------------------------------------------------------------
void ABall::Draw_Teleporting(HDC hdc, int step)
{
	int top_y = Ball_Rect.top + step / 2;
	int low_y = Ball_Rect.bottom - step / 2 - 1;

	if (top_y >= low_y)
		return;

	AsConfig::White_Color.Select(hdc);
	Ellipse(hdc, Ball_Rect.left, top_y, Ball_Rect.right - 1, low_y);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Move()
{
	int i;
	bool got_hit;
	double next_x_pos, next_y_pos;

	if (Ball_State == EBS_Lost || Ball_State == EBS_On_Platform || Ball_State == EBS_Teleporting)
		return;

	Prev_Ball_Rect = Ball_Rect;
	Rest_Distance += Ball_Speed;

	while (Rest_Distance >= AsConfig::Moving_Step_Size)
	{
		got_hit = false;

		next_x_pos = Center_X_Pos + AsConfig::Moving_Step_Size * cos(Ball_Direction);
		next_y_pos = Center_Y_Pos - AsConfig::Moving_Step_Size * sin(Ball_Direction);

		// Корректируем позицию при отражении:
		for (i = 0; i < Hit_Checkers_Count; i++)
			got_hit |= Hit_Checkers[i]->Check_Hit(next_x_pos, next_y_pos, this);

		if (! got_hit)
		{
			// Мячик продолжит движение, если не взаимодействовал с другими объектами
			Rest_Distance -= AsConfig::Moving_Step_Size;

			Center_X_Pos = next_x_pos;
			Center_Y_Pos = next_y_pos;

			if (Testing_Is_Active)
				Rest_Test_Distance -= AsConfig::Moving_Step_Size;
		}

		if (Ball_State == EBS_Lost)
			break;
	}

	Redraw_Ball();

	if (Ball_State == EBS_On_Parachute)
	{
		Prev_Parachute_Rect = Parachute_Rect;

		Parachute_Rect.bottom = Ball_Rect.bottom;
		Parachute_Rect.top = Parachute_Rect.bottom - Parachute_Size * AsConfig::Global_Scale;

		Redraw_Parachute();
	}
}
//------------------------------------------------------------------------------------------------------------
void ABall::Set_For_Test()
{
	Testing_Is_Active = true;
	Rest_Test_Distance = 50.0;

	Set_State(EBS_Normal, 130 + Test_Iteration, 90);
	Ball_Direction = M_PI_4;
	Ball_Speed = 3.0;

	++Test_Iteration;
}
//------------------------------------------------------------------------------------------------------------
bool ABall::Is_Test_Finished()
{
	if (Testing_Is_Active)
	{
		if (Rest_Test_Distance <= 0.0)
		{
			Testing_Is_Active = false;
			Set_State(EBS_Lost, 0);
			return true;
		}
	}

	return false;
}
//------------------------------------------------------------------------------------------------------------
EBall_State ABall::Get_State()
{
	return Ball_State;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Set_State(EBall_State new_state, double x_pos, double y_pos)
{
	switch (new_state)
	{
	case EBS_Normal:
		Center_X_Pos = x_pos;
		Center_Y_Pos = y_pos;
		Ball_Speed = 3.0;
		Rest_Distance = 0.0;
		Ball_Direction = M_PI_4;
		Redraw_Ball();
		break;


	case EBS_Lost:
		if (! (Ball_State == EBS_Normal || Ball_State == EBS_On_Parachute) )
			AsConfig::Throw();  // Только из этих состояний можно потерять мячик!

		Ball_Speed = 0.0;
		Redraw_Ball();
		Redraw_Parachute();
		break;


	case EBS_On_Platform:
		Center_X_Pos = x_pos;
		Center_Y_Pos = y_pos;
		Ball_Speed = 0.0;
		Rest_Distance = 0.0;
		Ball_Direction = M_PI_4;
		Redraw_Ball();
		break;


	case EBS_On_Parachute:
		AsConfig::Throw();  // Для постановки на парашют нужно вызвать специальный метод Set_On_Parachute()
		break;


	case EBS_Off_Parachute:
		if (Ball_State != EBS_On_Parachute)
			AsConfig::Throw();  // В это состояние можно перейти только из EBS_On_Parachute!

		Ball_Speed = 0.0;
		Rest_Distance = 0.0;
		Redraw_Ball();
		Redraw_Parachute();
		break;


	case EBS_Teleporting:
		if (! (Ball_State == EBS_Normal || Ball_State == EBS_On_Parachute || Ball_State == EBS_Teleporting) )
			AsConfig::Throw();  // Только из этих состояний можно войти в телепорт!

		Center_X_Pos = x_pos;
		Center_Y_Pos = y_pos;
		Ball_Speed = 0.0;
		Rest_Distance = 0.0;
		Redraw_Ball();

		if (Ball_State == EBS_On_Parachute)
			Redraw_Parachute();
		break;


	default:
		AsConfig::Throw();
	}

	Prev_Ball_State = Ball_State;
	Ball_State = new_state;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Get_Center(double &x_pos, double &y_pos)
{
	x_pos = Center_X_Pos;
	y_pos = Center_Y_Pos;
}
//------------------------------------------------------------------------------------------------------------
double ABall::Get_Direction()
{
	return Ball_Direction;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Set_Direction(double new_direction)
{
	const double pi_2 = 2.0 * M_PI;

	while (new_direction > pi_2)
		new_direction -= pi_2;

	while (new_direction < 0.0)
		new_direction += pi_2;

	Ball_Direction = new_direction;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Reflect(bool from_horizontal)
{
	if (from_horizontal)
		Set_Direction(-Ball_Direction);
	else
		Set_Direction(M_PI - Ball_Direction);
}
//------------------------------------------------------------------------------------------------------------
bool ABall::Is_Moving_Up()
{
	if (Ball_Direction >= 0.0 && Ball_Direction < M_PI)
		return true;
	else
		return false;
}
//------------------------------------------------------------------------------------------------------------
bool ABall::Is_Moving_Left()
{
	if (Ball_Direction > M_PI_2 && Ball_Direction < M_PI + M_PI_2)
		return true;
	else
		return false;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Set_On_Parachute(int brick_x, int brick_y)
{
	int cell_x = AsConfig::Level_X_Offset + brick_x * AsConfig::Cell_Width;
	int cell_y = AsConfig::Level_Y_Offset + brick_y * AsConfig::Cell_Height;

	Ball_Direction = M_PI + M_PI_2;
	Ball_Speed = 1.0;
	Ball_State = EBS_On_Parachute;

	Parachute_Rect.left = cell_x * AsConfig::Global_Scale;
	Parachute_Rect.top = cell_y * AsConfig::Global_Scale;
	Parachute_Rect.right = Parachute_Rect.left + Parachute_Size * AsConfig::Global_Scale;
	Parachute_Rect.bottom = Parachute_Rect.top + Parachute_Size * AsConfig::Global_Scale;

	Prev_Parachute_Rect = Parachute_Rect;

	Center_X_Pos = (double)(cell_x + AsConfig::Cell_Width / 2) - 1.0 / (double)AsConfig::Global_Scale;
	Center_Y_Pos = (double)(cell_y + Parachute_Size) - ABall::Radius * 2.0;

	Redraw_Parachute();
}
//------------------------------------------------------------------------------------------------------------
void ABall::Add_Hit_Checker(AHit_Checker *hit_checker)
{
	if (Hit_Checkers_Count >= sizeof(Hit_Checkers) / sizeof(Hit_Checkers[0]) )
		return;

	Hit_Checkers[Hit_Checkers_Count++] = hit_checker;
}
//------------------------------------------------------------------------------------------------------------
void ABall::Redraw_Ball()
{
	Ball_Rect.left = (int)((Center_X_Pos - Radius) * AsConfig::Global_Scale);
	Ball_Rect.top = (int)((Center_Y_Pos - Radius) * AsConfig::Global_Scale);
	Ball_Rect.right = (int)((Center_X_Pos + Radius) * AsConfig::Global_Scale);
	Ball_Rect.bottom = (int)((Center_Y_Pos + Radius) * AsConfig::Global_Scale);

	InvalidateRect(AsConfig::Hwnd, &Prev_Ball_Rect, FALSE);
	InvalidateRect(AsConfig::Hwnd, &Ball_Rect, FALSE);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Redraw_Parachute()
{
	InvalidateRect(AsConfig::Hwnd, &Prev_Parachute_Rect, FALSE);
	InvalidateRect(AsConfig::Hwnd, &Parachute_Rect, FALSE);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Draw_Parachute(HDC hdc, RECT &paint_area)
{
	const int scale = AsConfig::Global_Scale;
	int dome_height = (Parachute_Rect.bottom - Parachute_Rect.top) / 2;
	int arc_height = 4 * scale;
	int arc_x;
	int line_y;
	int ball_center_x, ball_center_y;
	RECT intersection_rect, sub_arc, other_arc;

	if (! IntersectRect(&intersection_rect, &paint_area, &Parachute_Rect) )
		return;

	Clear_Parachute(hdc);

	// 1. Купол
	AsConfig::Blue_Color.Select(hdc);
	Chord(hdc, Parachute_Rect.left, Parachute_Rect.top, Parachute_Rect.right - 1, Parachute_Rect.bottom - 1,
		Parachute_Rect.right, Parachute_Rect.top + dome_height - 1, Parachute_Rect.left, Parachute_Rect.top + dome_height - 1);

	// 2. Арки
	AsConfig::BG_Color.Select(hdc);
	arc_x = Parachute_Rect.left + 1;

	// 2.1. Левая
	sub_arc.left = arc_x;
	sub_arc.top = Parachute_Rect.top + dome_height - arc_height / 2;
	sub_arc.right = sub_arc.left + 3 * scale;
	sub_arc.bottom = sub_arc.top + 4 * scale;

	Ellipse(hdc, sub_arc.left, sub_arc.top, sub_arc.right - 1, sub_arc.bottom - 1);

	// 2.2. Средняя
	other_arc = sub_arc;

	other_arc.left = arc_x + 3 * scale + 1;
	other_arc.right = arc_x + 11 * scale;

	Ellipse(hdc, other_arc.left, other_arc.top, other_arc.right - 1, other_arc.bottom - 1);

	// 2.3. Правая
	other_arc = sub_arc;

	other_arc.left = arc_x + 11 * scale + 1;
	other_arc.right = arc_x + 14 * scale + 1;

	Ellipse(hdc, other_arc.left, other_arc.top, other_arc.right - 1, other_arc.bottom - 1);


	// 3. Стропы
	line_y = Parachute_Rect.top + dome_height;
	ball_center_x = (Parachute_Rect.left + Parachute_Rect.right) / 2;
	ball_center_y = Parachute_Rect.bottom - 2 * scale;

	AsConfig::White_Color.Select(hdc);

	MoveToEx(hdc, Parachute_Rect.left, line_y, 0);
	LineTo(hdc, ball_center_x, ball_center_y);

	MoveToEx(hdc, Parachute_Rect.left + 3 * scale + 1, line_y, 0);
	LineTo(hdc, ball_center_x, ball_center_y);

	MoveToEx(hdc, Parachute_Rect.right - 4 * scale + 1, line_y, 0);
	LineTo(hdc, ball_center_x, ball_center_y);

	MoveToEx(hdc, Parachute_Rect.right, line_y - 1, 0);
	LineTo(hdc, ball_center_x, ball_center_y);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Clear_Parachute(HDC hdc)
{// Очищаем фон

	AsConfig::BG_Color.Select(hdc);
	AsConfig::Round_Rect(hdc, Prev_Parachute_Rect);
}
//------------------------------------------------------------------------------------------------------------
