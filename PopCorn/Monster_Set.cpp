#include "Monster_Set.h"

// AMonster
const double AMonster::Max_Cornea_Height = 11.0;
const double AMonster::Blink_Timeouts[Blink_Stages_Count] = { 0.4, 0.2, 0.8, 0.4, 0.4, 0.4, 0.8 };
const EEye_State AMonster::Blink_States[Blink_Stages_Count] = 
{
	EEye_State::Closed, EEye_State::Opening, EEye_State::Staring, EEye_State::Closing,
	EEye_State::Opening, EEye_State::Staring, EEye_State::Closing
};
//------------------------------------------------------------------------------------------------------------
AMonster::AMonster()
: Is_Active(false), Eye_State(EEye_State::Closed), X_Pos(0), Y_Pos(0), Cornea_Height(Max_Cornea_Height), Start_Blink_Timeout(0),
  Total_Animation_Timeout(0), Monster_Rect{}, Blink_Ticks{}
{
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Begin_Movement()
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Finish_Movement()
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Advance(double max_speed)
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
double AMonster::Get_Speed()
{
	return 0.0;  //!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Act()
{
	int i;
	int curr_tick_offset;
	int prev_tick;
	double ratio;

	if (! Is_Active)
		return;

	curr_tick_offset = (AsConfig::Current_Timer_Tick - Start_Blink_Timeout) % Total_Animation_Timeout;

	for (i = 0; i < Blink_Stages_Count; i++)
	{
		if (curr_tick_offset < Blink_Ticks[i])
		{
			Eye_State = Blink_States[i];
			break;
		}
	}

	if (i == 0)
		prev_tick = 0;
	else
		prev_tick = Blink_Ticks[i - 1];

	ratio = (double)(curr_tick_offset - prev_tick) / (double)(Blink_Ticks[i] - prev_tick);

	switch (Eye_State)
	{
	case EEye_State::Closed:
		Cornea_Height = 0.0;
		break;

	case EEye_State::Opening:
		Cornea_Height = Max_Cornea_Height * ratio;
		break;

	case EEye_State::Staring:
		Cornea_Height = Max_Cornea_Height;
		break;

	case EEye_State::Closing:
		Cornea_Height = Max_Cornea_Height * (1.0 - ratio);
		break;

	default:
		AsConfig::Throw();
	}

	AsTools::Invalidate_Rect(Monster_Rect);
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Clear(HDC hdc, RECT &paint_area)
{
	//!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Draw(HDC hdc, RECT &paint_area)
{
	const int scale = AsConfig::Global_Scale;
	const double d_scale = AsConfig::D_Global_Scale;
	const int half_scale = scale / 2;
	HRGN region;
	RECT intersection_rect;
	RECT rect, cornea_rect;

	if (! Is_Active)
		return;

	if (! IntersectRect(&intersection_rect, &paint_area, &Monster_Rect) )
		return;

	// 1. Рисуем фон
	// 1.1. Ограничиваем вывод фона
	rect = Monster_Rect;

	++rect.right;
	++rect.bottom;

	region = CreateEllipticRgnIndirect(&rect);
	SelectClipRgn(hdc, region);

	// 1.2. Тёмный фон
	AsTools::Ellipse(hdc, Monster_Rect, AsConfig::Monster_Dark_Red_Color);

	// 1.3. Красный фон
	rect = Monster_Rect;

	rect.left -= 2 * scale;
	rect.top -= 3 * scale;
	rect.right -= 2 * scale;
	rect.bottom -= 3 * scale;

	AsTools::Ellipse(hdc, rect, AsConfig::Red_Color);

	SelectClipRgn(hdc, 0);
	DeleteObject(region);

	// 2. Рисуем сам глаз
	if (Eye_State == EEye_State::Closed)
		return;

	// 2.1. Роговица
	cornea_rect = Monster_Rect;

	cornea_rect.left += scale + half_scale;
	cornea_rect.top += 2 * scale + (int)( (Max_Cornea_Height / 2.0 - Cornea_Height / 2.0) * d_scale);
	cornea_rect.right -= scale + half_scale;
	cornea_rect.bottom = cornea_rect.top + (int)(Cornea_Height * d_scale);

	// 2.2. Ограничиваем вывод внутренней части глаза
	region = CreateEllipticRgnIndirect(&cornea_rect);
	SelectClipRgn(hdc, region);

	AsTools::Ellipse(hdc, cornea_rect, AsConfig::Monster_Cornea_Color);

	// 2.3. Радужка
	rect = Monster_Rect;

	rect.left += 4 * scale + half_scale;
	rect.top += 4 * scale;
	rect.right -= 4 * scale + half_scale;
	rect.bottom -= 5 * scale;

	AsTools::Ellipse(hdc, rect, AsConfig::Monster_Iris_Color);

	// 2.4. Зрачок
	rect = Monster_Rect;

	rect.left += 7 * scale;
	rect.top += 6 * scale;
	rect.right = rect.left + 2 * scale;
	rect.bottom = rect.top + 3 * scale;

	AsTools::Rect(hdc, rect, AsConfig::BG_Color);

	SelectClipRgn(hdc, 0);
	DeleteObject(region);

	// 2.5. Обводим роговицу
	AsConfig::BG_Outline_Color.Select_Pen(hdc);

	Arc(hdc, cornea_rect.left, cornea_rect.top, cornea_rect.right - 1, cornea_rect.bottom - 1, 0, 0, 0, 0);
}
//------------------------------------------------------------------------------------------------------------
bool AMonster::Is_Finished()
{
	return false;  //!!! Надо сделать!
}
//------------------------------------------------------------------------------------------------------------
void AMonster::Activate(int x_pos, int y_pos)
{
	int i;
	int tick_offset;
	double curr_timeout = 0.0;
	const int scale = AsConfig::Global_Scale;

	Is_Active = true;

	X_Pos = x_pos + 10;
	Y_Pos = y_pos;

	Monster_Rect.left = X_Pos * scale;
	Monster_Rect.top = Y_Pos * scale;
	Monster_Rect.right = Monster_Rect.left + Width * scale;
	Monster_Rect.bottom = Monster_Rect.top + Height * scale;

	// Рассчитываем тики анимации
	curr_timeout;
	Start_Blink_Timeout = AsConfig::Current_Timer_Tick;

	for (i = 0; i < Blink_Stages_Count; i++)
	{
		curr_timeout += Blink_Timeouts[i];
		tick_offset = (int)( (double)AsConfig::FPS * curr_timeout);
		Blink_Ticks[i] = tick_offset;
	}

	Total_Animation_Timeout = tick_offset;
}
//------------------------------------------------------------------------------------------------------------




// AsMonster_Set
//------------------------------------------------------------------------------------------------------------
AsMonster_Set::AsMonster_Set()
: Border(0)
{
}
//------------------------------------------------------------------------------------------------------------
void AsMonster_Set::Init(AsBorder *border)
{
	Border = border;
}
//------------------------------------------------------------------------------------------------------------
void AsMonster_Set::Emit_At_Gate(int gate_index)
{
	int i;
	int gate_x_pos, gate_y_pos;
	AMonster *monster = 0;

	if (gate_index < 0 || gate_index >= AsConfig::Gates_Count)
		AsConfig::Throw();

	for (i = 0; i < Max_Monsters_Count; i++)
	{
		if (! Monsters[i].Is_Active)
		{
			monster = &Monsters[i];
			break;
		}
	}

	if (monster == 0)
		return;  // Все монстры - заняты (уже на поле)

	Border->Get_Gate_Pos(gate_index, gate_x_pos, gate_y_pos);

	monster->Activate(gate_x_pos, gate_y_pos);
}
//------------------------------------------------------------------------------------------------------------
bool AsMonster_Set::Get_Next_Game_Object(int &index, AGame_Object **game_obj)
{
	if (index < 0 || index >= AsConfig::Max_Balls_Count)
		return false;

	*game_obj = &Monsters[index++];

	return true;
}
//------------------------------------------------------------------------------------------------------------
