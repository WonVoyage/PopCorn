#include "Monster_Set.h"

// AsMonster_Set
//------------------------------------------------------------------------------------------------------------
AsMonster_Set::AsMonster_Set()
: Monster_Set_State(EMonster_Set_State::Idle), Current_Gate_Index(-1), Max_Alive_Monsters_Count(0), Border(0)
{
	memset(Monsters, 0, sizeof(AMonster *) * Max_Monsters_Count);
}
//------------------------------------------------------------------------------------------------------------
bool AsMonster_Set::Check_Hit(double next_x_pos, double next_y_pos, ABall_Object *ball)
{
	int i;

	for (i = 0; i < Max_Monsters_Count; i++)
		if (Monsters[i] != 0 && Monsters[i]->Check_Hit(next_x_pos, next_y_pos, ball) )
			return true;

	return false;
}
//------------------------------------------------------------------------------------------------------------
bool AsMonster_Set::Check_Hit(double next_x_pos, double next_y_pos)
{
	int i;

	for (i = 0; i < Max_Monsters_Count; i++)
		if (Monsters[i] != 0 && Monsters[i]->Check_Hit(next_x_pos, next_y_pos) )
			return true;

	return false;
}
//------------------------------------------------------------------------------------------------------------
bool AsMonster_Set::Check_Hit(RECT &rect)
{
	int i;

	for (i = 0; i < Max_Monsters_Count; i++)
		if (Monsters[i] != 0 && Monsters[i]->Check_Hit(rect) )
			return true;

	return false;
}
//------------------------------------------------------------------------------------------------------------
void AsMonster_Set::Act()
{
	int i;
	int current_alive_count;

	switch (Monster_Set_State)
	{
	case EMonster_Set_State::Idle:
		break;


	case EMonster_Set_State::Selecting_Next_Gate:
		// Считаем количество живых монстров
		current_alive_count = 0;

		for (i = 0; i < Max_Monsters_Count; i++)
			if (Monsters[i] != 0 && ! Monsters[i]->Is_Finished() )
				++current_alive_count;

		// Добавляем нового монстра, если можно
		if (current_alive_count < Max_Alive_Monsters_Count)
		{
			Current_Gate_Index = Border->Long_Open_Gate();
			Monster_Set_State = EMonster_Set_State::Waiting_Gate_Opening;
		}
		break;


	case EMonster_Set_State::Waiting_Gate_Opening:
		if (Border->Is_Gate_Opened(Current_Gate_Index) )
		{
			Emit_At_Gate(Current_Gate_Index);
			Monster_Set_State = EMonster_Set_State::Waiting_Gate_Closing;
		}
		break;


	case EMonster_Set_State::Waiting_Gate_Closing:
		if (Border->Is_Gate_Closed(Current_Gate_Index) )
			Monster_Set_State = EMonster_Set_State::Selecting_Next_Gate;

		break;


	default:
		AsConfig::Throw();
	}

	if (Monster_Set_State != EMonster_Set_State::Idle)
	{
		for (i = 0; i < Max_Monsters_Count; i++)
			if (Monsters[i] != 0 && Monsters[i]->Is_Finished() )
			{
				delete Monsters[i];
				Monsters[i] = 0;
			}
	}

	AGame_Objects_Set::Act();
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
	bool gate_is_left;
	int gate_x_pos, gate_y_pos;
	int monster_type;
	AMonster *monster = 0;

	if (gate_index < 0 || gate_index >= AsConfig::Gates_Count)
		AsConfig::Throw();

	for (i = 0; i < Max_Monsters_Count; i++)
	{
		if (Monsters[i] == 0)
		{
			monster_type = AsTools::Rand(2);

			if (monster_type == 0)
				monster = new AMonster_Eye();
			else
				monster = new AMonster_Comet();

			Monsters[i] = monster;
			break;
		}
	}

	if (monster == 0)
		return;  // Все монстры - заняты (уже на поле)

	Border->Get_Gate_Pos(gate_index, gate_x_pos, gate_y_pos);

	if (gate_index % 2 == 0)
		gate_is_left = true;
	else
		gate_is_left = false;

	if (! gate_is_left)
		gate_x_pos -= monster->Width - AGate::Width;

	monster->Activate(gate_x_pos, gate_y_pos + 1, gate_is_left);

	//monster->Destroy();
}
//------------------------------------------------------------------------------------------------------------
void AsMonster_Set::Activate(int max_alive_monsters_count)
{
	Monster_Set_State = EMonster_Set_State::Selecting_Next_Gate;
	Max_Alive_Monsters_Count = max_alive_monsters_count;
}
//------------------------------------------------------------------------------------------------------------
void AsMonster_Set::Destroy_All()
{
	int i;

	for (i = 0; i < Max_Monsters_Count; i++)
		if (Monsters[i] != 0)
			Monsters[i]->Destroy();

	Monster_Set_State = EMonster_Set_State::Idle;
}
//------------------------------------------------------------------------------------------------------------
bool AsMonster_Set::Get_Next_Game_Object(int &index, AGame_Object **game_obj)
{
	AMonster *monster;

	if (index < 0 || index >= Max_Monsters_Count)
		return false;

	while (index < Max_Monsters_Count)
	{
		monster = Monsters[index++];

		if (monster != 0)
		{
			*game_obj = monster;
			return true;
		}
	}

	return false;
}
//------------------------------------------------------------------------------------------------------------
