﻿#include "Engine.h"

// AsEngine
//------------------------------------------------------------------------------------------------------------
AsEngine::AsEngine()
: Game_State(EGS_Lost_Ball), Rest_Distance(0.0), Life_Count(AsConfig::Initial_Life_Count)
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
	Advance_Movers();

	if (Ball_Set.All_Balls_Are_Lost() )
	{// Потеряли все мячики

		Game_State = EGS_Lost_Ball;
		Level.Stop();
		Platform.Set_State(EPS_Pre_Meltdown);
	}
	else
		Ball_Set.Accelerate();

	if (Ball_Set.Is_Test_Finished() )
		Game_State = EGS_Test_Ball;
}
//------------------------------------------------------------------------------------------------------------
void AsEngine::Advance_Movers()
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

	case ELT_I:  // "Инверсия"
		Ball_Set.Inverse_Balls();
		break;

	case ELT_C:  // "Скорость"
		Ball_Set.Reset_Speed();
		break;

	//case ELT_M:  // "Монстры"

	case ELT_G:  // "Жизнь"
		if (Life_Count < AsConfig::Max_Life_Count)
			++Life_Count;  //!!! Отобразить на индикаторе!
		break;

	//case ELT_K:  // "Клей"
	//case ELT_W:  // "Шире"

	case ELT_T:  // "Три"
		Ball_Set.Triple_Balls();
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
