﻿#include "Engine.h"

// AsBall_Set
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Begin_Movement()
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		Balls[i].Begin_Movement();
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Finish_Movement()
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		Balls[i].Finish_Movement();
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Advance(double max_speed)
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		Balls[i].Advance(max_speed);
}
//------------------------------------------------------------------------------------------------------------
double AsBall_Set::Get_Speed()
{
	int i;
	double curr_speed, max_speed = 0.0;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
	{
		curr_speed = Balls[i].Get_Speed();

		if (curr_speed > max_speed)
			max_speed = curr_speed;
	}

	return max_speed;
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Draw(HDC hdc, RECT &paint_area)
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		Balls[i].Draw(hdc, paint_area);
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Release_From_Platform(double platform_x_pos)
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		if (Balls[i].Get_State() == EBS_On_Platform)
			Balls[i].Set_State(EBS_Normal, platform_x_pos, AsConfig::Start_Ball_Y_Pos);
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Set_On_Platform(double platform_x_pos)
{
	int i;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
		Balls[i].Set_State(EBS_On_Platform, platform_x_pos, AsConfig::Start_Ball_Y_Pos);

	//for (; i < AsConfig::Max_Balls_Count; i++)
	//	Balls[i].Set_State(EBS_Disabled);
}
//------------------------------------------------------------------------------------------------------------
bool AsBall_Set::All_Balls_Are_Lost()
{
	int i;
	int active_balls_count = 0;
	int lost_balls_count = 0;

	for (i = 0; i < AsConfig::Max_Balls_Count; i++)
	{
		if (Balls[i].Get_State() == EBS_Disabled)
			continue;

		++active_balls_count;

		if (Balls[i].Get_State() == EBS_Lost)
		{
			++lost_balls_count;
			continue;
		}
	}

	if (active_balls_count == lost_balls_count)
		return true;
	else
		return false;
}
//------------------------------------------------------------------------------------------------------------
void AsBall_Set::Set_For_Test()
{
	Balls[0].Set_For_Test();  // В повторяющихся тестах участвует только 0-й мячик
}
//------------------------------------------------------------------------------------------------------------
bool AsBall_Set::Is_Test_Finished()
{
	return Balls[0].Is_Test_Finished();  // В повторяющихся тестах участвует только 0-й мячик
}
//------------------------------------------------------------------------------------------------------------




// AsEngine
//------------------------------------------------------------------------------------------------------------
AsEngine::AsEngine()
: Game_State(EGS_Lost_Ball), Rest_Distance(0.0)
{
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Init_Engine(HWND hwnd)
{// Настройка игры при старте

	SYSTEMTIME sys_time;
	FILETIME file_time;

	GetSystemTime(&sys_time);
	SystemTimeToFileTime(&sys_time, &file_time);

	srand(file_time.dwLowDateTime);

	AsConfig::Hwnd = hwnd;

	AActive_Brick_Red_Blue::Setup_Colors();

	Level.Init();

	AFalling_Letter::Init();

	ABall::Add_Hit_Checker(&Border);
	ABall::Add_Hit_Checker(&Level);
	ABall::Add_Hit_Checker(&Platform);

	Level.Set_Current_Level(AsLevel::Level_01);

	//Ball.Set_State(EBS_Normal, Platform.X_Pos + Platform.Width / 2);
	//Platform.Set_State(EPS_Normal);

	Platform.Redraw_Platform();

	SetTimer(AsConfig::Hwnd, Timer_ID, 1000 / AsConfig::FPS, 0);

	memset(Movers, 0, sizeof(Movers) );
	Movers[0] = &Platform;
	Movers[1] = &Ball_Set;
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Draw_Frame(HDC hdc, RECT &paint_area)
{// Отрисовка экрана игры

	SetGraphicsMode(hdc, GM_ADVANCED);

	Level.Draw(hdc, paint_area);
	Border.Draw(hdc, paint_area);
	Platform.Draw(hdc, paint_area);
	Ball_Set.Draw(hdc, paint_area);
}
//------------------------------------------------------------------------------------------------------------
int AsEngine::On_Key(EKey_Type key_type, bool key_down)
{
	switch (key_type)
	{
	case EKT_Left:
		Platform.Move(true, key_down);
		break;


	case EKT_Right:
		Platform.Move(false, key_down);
		break;


	case EKT_Space:
		if (key_down)
			if (Platform.Get_State() == EPS_Ready)
			{
				Ball_Set.Release_From_Platform(Platform.Get_Middle_Pos() );
				Platform.Set_State(EPS_Normal);
			}
		break;
	}

	return 0;
}
//------------------------------------------------------------------------------------------------------------
int AsEngine::On_Timer()
{
	++AsConfig::Current_Timer_Tick;

	switch (Game_State)
	{
	case EGS_Test_Ball:
		Ball_Set.Set_For_Test();
		Game_State = EGS_Play_Level;
		break;


	case EGS_Play_Level:
		Play_Level();
		break;


	case EGS_Lost_Ball:
		if (Platform.Get_State() == EPS_Missing)
		{
			Game_State = EGS_Restart_Level;
			Platform.Set_State(EPS_Roll_In);
		}
		break;


	case EGS_Restart_Level:
		if (Platform.Get_State() == EPS_Ready)
		{
			Game_State = EGS_Play_Level;
			Ball_Set.Set_On_Platform(Platform.Get_Middle_Pos() );
		}
		break;
	}

	Act();

	return 0;
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Play_Level()
{
	Advance_Mover();

	if (Ball_Set.All_Balls_Are_Lost() )
	{// Потеряли все мячики

		Game_State = EGS_Lost_Ball;
		Level.Stop();
		Platform.Set_State(EPS_Meltdown);
	}

	if (Ball_Set.Is_Test_Finished() )
		Game_State = EGS_Test_Ball;
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Advance_Mover()
{
	int i;
	double curr_speed, max_speed = 0.0;
	//double ball_x, ball_y;

	// 1. Получаем максимальную скорость движущихся объектов
	for (i = 0; i < AsConfig::Max_Movers_Count; i++)
	{
		if (Movers[i] != 0)
		{
			Movers[i]->Begin_Movement();

			curr_speed = fabs(Movers[i]->Get_Speed() );

			if (curr_speed > max_speed)
				max_speed = curr_speed;
		}
	}


	// 2. Смещаем все движущиеся объекты
	Rest_Distance += max_speed;

	while (Rest_Distance > 0.0)
	{
		for (i = 0; i < AsConfig::Max_Movers_Count; i++)
			if (Movers[i] != 0)
				Movers[i]->Advance(max_speed);

		//Platform.Advance(max_speed);
		Rest_Distance -= AsConfig::Moving_Step_Size;
	}


	//for (i = 0; i < AsConfig::Max_Movers_Count; i++)
	//{
	//	Ball_Set.Balls[i].Get_Center(ball_x, ball_y);

	//	if (ball_x >= Platform.X_Pos + 1 && ball_x <= Platform.X_Pos + Platform.Width - 1)
	//		if (ball_y >= AsConfig::Platform_Y_Pos + 1 && ball_y <= AsConfig::Platform_Y_Pos + 5)
	//			int yy = 0;
	//}

	// 3. Заканчиваем все движения на этом кадре
	for (i = 0; i < AsConfig::Max_Movers_Count; i++)
		if (Movers[i] != 0)
			Movers[i]->Finish_Movement();
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Act()
{
	int index = 0;
	AFalling_Letter *falling_letter;

	Platform.Act();
	Level.Act();

	while (Level.Get_Next_Falling_Letter(index, &falling_letter) )
	{
		if (Platform.Hit_By(falling_letter) )
			On_Falling_Letter(falling_letter);
	}
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::On_Falling_Letter(AFalling_Letter *falling_letter)
{
	switch (falling_letter->Letter_Type)
	{
	//case ELT_O:  // "Отмена"
	//case ELT_I:  // "Инверсия"
	//case ELT_C:  // "Скорость"
	//case ELT_M:  // "Монстры"
	//case ELT_G:  // "Жизнь"
	//case ELT_K:  // "Клей"
	//case ELT_W:  // "Шире"

	case ELT_T:  // "Три"
		break;

	//case ELT_L:  // "Лазер"
	//case ELT_P:  // "Пол"
	//case ELT_Plus:  // Переход на следующий уровень
	default:
		AsConfig::Throw();
	}

	falling_letter->Finalize();
}
//------------------------------------------------------------------------------------------------------------
