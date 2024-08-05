#include "pch.h"
#include "platform.h"


#include <MainWindow.h>

#include <Dice.h>
#include <Wheel.h>

MainWindow::MainWindow()
{
	// ROAD
	_placeholders[0] = std::make_unique<PlaceHolder>(PH_ROAD, 5, -1);
	_placeholders[1] = std::make_unique<PlaceHolder>(PH_ROAD, 4, -1);
	_placeholders[2] = std::make_unique<PlaceHolder>(PH_ROAD, 3, -1);
	_placeholders[3] = std::make_unique<PlaceHolder>(PH_ROAD, 2, -1);
	_placeholders[4] = std::make_unique<PlaceHolder>(PH_ROAD, 1, -1);

	_placeholders[5] = std::make_unique<PlaceHolder>(PH_ROAD, 1, -2);
	_placeholders[6] = std::make_unique<PlaceHolder>(PH_ROAD, 1, -3);
	_placeholders[7] = std::make_unique<PlaceHolder>(PH_ROAD, 1, -4);
	_placeholders[8] = std::make_unique<PlaceHolder>(PH_ROAD, 1, -5);

	_placeholders[9] = std::make_unique<PlaceHolder>(PH_ROAD, 0, -5);
	_placeholders[10] = std::make_unique<PlaceHolder>(PH_ROAD, -1, -5);

	_placeholders[11] = std::make_unique<PlaceHolder>(PH_ROAD, -1, -4);
	_placeholders[12] = std::make_unique<PlaceHolder>(PH_ROAD, -1, -3);
	_placeholders[13] = std::make_unique<PlaceHolder>(PH_ROAD, -1, -2);
	_placeholders[14] = std::make_unique<PlaceHolder>(PH_ROAD, -1, -1);

	_placeholders[15] = std::make_unique<PlaceHolder>(PH_ROAD, -2, -1);
	_placeholders[16] = std::make_unique<PlaceHolder>(PH_ROAD, -3, -1);
	_placeholders[17] = std::make_unique<PlaceHolder>(PH_ROAD, -4, -1);
	_placeholders[18] = std::make_unique<PlaceHolder>(PH_ROAD, -5, -1);

	_placeholders[19] = std::make_unique<PlaceHolder>(PH_ROAD, -5, 0);
	_placeholders[20] = std::make_unique<PlaceHolder>(PH_ROAD, -5, 1);

	_placeholders[21] = std::make_unique<PlaceHolder>(PH_ROAD, -4, 1);
	_placeholders[22] = std::make_unique<PlaceHolder>(PH_ROAD, -3, 1);
	_placeholders[23] = std::make_unique<PlaceHolder>(PH_ROAD, -2, 1);
	_placeholders[24] = std::make_unique<PlaceHolder>(PH_ROAD, -1, 1);

	_placeholders[25] = std::make_unique<PlaceHolder>(PH_ROAD, -1, 2);
	_placeholders[26] = std::make_unique<PlaceHolder>(PH_ROAD, -1, 3);
	_placeholders[27] = std::make_unique<PlaceHolder>(PH_ROAD, -1, 4);
	_placeholders[28] = std::make_unique<PlaceHolder>(PH_ROAD, -1, 5);

	_placeholders[29] = std::make_unique<PlaceHolder>(PH_ROAD, 0, 5);
	_placeholders[30] = std::make_unique<PlaceHolder>(PH_ROAD, 1, 5);

	_placeholders[31] = std::make_unique<PlaceHolder>(PH_ROAD, 1, 4);
	_placeholders[32] = std::make_unique<PlaceHolder>(PH_ROAD, 1, 3);
	_placeholders[33] = std::make_unique<PlaceHolder>(PH_ROAD, 1, 2);
	_placeholders[34] = std::make_unique<PlaceHolder>(PH_ROAD, 1, 1);

	_placeholders[35] = std::make_unique<PlaceHolder>(PH_ROAD, 2, 1);
	_placeholders[36] = std::make_unique<PlaceHolder>(PH_ROAD, 3, 1);
	_placeholders[37] = std::make_unique<PlaceHolder>(PH_ROAD, 4, 1);
	_placeholders[38] = std::make_unique<PlaceHolder>(PH_ROAD, 5, 1);

	_placeholders[39] = std::make_unique<PlaceHolder>(PH_ROAD, 5, 0);

	// HOME
	_placeholders[40] = std::make_unique<PlaceHolder>(PH_HOME_P1, 4, -4);
	_placeholders[41] = std::make_unique<PlaceHolder>(PH_HOME_P1, 5, -4);
	_placeholders[42] = std::make_unique<PlaceHolder>(PH_HOME_P1, 5, -5);
	_placeholders[43] = std::make_unique<PlaceHolder>(PH_HOME_P1, 4, -5);

	_placeholders[44] = std::make_unique<PlaceHolder>(PH_HOME_P2, -5, 5);
	_placeholders[45] = std::make_unique<PlaceHolder>(PH_HOME_P2, -4, 5);
	_placeholders[46] = std::make_unique<PlaceHolder>(PH_HOME_P2, -4, 4);
	_placeholders[47] = std::make_unique<PlaceHolder>(PH_HOME_P2, -5, 4);

	// TARGET
	_placeholders[48] = std::make_unique<PlaceHolder>(PH_TARGET_P1, 4, 0);
	_placeholders[49] = std::make_unique<PlaceHolder>(PH_TARGET_P1, 3, 0);
	_placeholders[50] = std::make_unique<PlaceHolder>(PH_TARGET_P1, 2, 0);
	_placeholders[51] = std::make_unique<PlaceHolder>(PH_TARGET_P1, 1, 0);

	_placeholders[52] = std::make_unique<PlaceHolder>(PH_TARGET_P2, -4, 0);
	_placeholders[53] = std::make_unique<PlaceHolder>(PH_TARGET_P2, -3, 0);
	_placeholders[54] = std::make_unique<PlaceHolder>(PH_TARGET_P2, -2, 0);
	_placeholders[55] = std::make_unique<PlaceHolder>(PH_TARGET_P2, -1, 0);

	// Initialize lists

	for (int ix = 0; ix < 40; ++ix)
	{
		_PH_road_p1_arr[ix] = _placeholders[ix].get();
	}

	for (int ix = 48; ix < 52; ++ix)
	{
		_PH_road_p1_arr[ix - 48 + 40] = _placeholders[ix].get(); 
		_PH_target_p1_arr[ix - 48] = _placeholders[ix].get();
	}

	for (int ix = 20; ix < 40; ++ix)
	{
		_PH_road_p2_arr[ix - 20] = _placeholders[ix].get();
	}

	for (int ix = 0; ix < 20; ++ix)
	{
		_PH_road_p2_arr[ix + 20] = _placeholders[ix].get();
	}

	for (int ix = 52; ix < 56; ++ix)
	{
		_PH_road_p2_arr[ix - 52 + 40] = _placeholders[ix].get();
		_PH_target_p2_arr[ix - 52] = _placeholders[ix].get();
	}

	for (int ix = 40; ix < 44; ++ix)
	{
		_PH_home_p1_arr[ix - 40] = _placeholders[ix].get();
	}

	for (int ix = 44; ix < 48; ++ix)
	{
		_PH_home_p2_arr[ix - 44] = _placeholders[ix].get();
	}

	// Set stones type
	for (int ix = 0; ix < 4; ++ix)
	{
		_g_stones_p1[ix] = std::make_unique<GStone>(GT_P1_STONE, ix);
		_g_stones_p2[ix] = std::make_unique<GStone>(GT_P2_STONE, ix);
	}

	// selection
	_selected_stone = nullptr;
}

MainWindow::~MainWindow()
{
}

void MainWindow::UpdateStonesPos()
{
	for (int ix = 0; ix < 4; ++ix)
		MoveStoneP1(&_Monkey->_stones_p1[ix],
			_g_stones_p1[ix]->_ph->_x,
			_g_stones_p1[ix]->_ph->_y);

	for (int ix = 0; ix < 4; ++ix)
		MoveStoneP2(&_Bird->_stones_p2[ix],
			_g_stones_p2[ix]->_ph->_x,
			_g_stones_p2[ix]->_ph->_y);
}

void MainWindow::UpdateStonesSelection()
{
	for (int ix = 0; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_selected)
		{
			_Monkey->_stones_p1[ix]._rItem[2]->NumFramesDirty = gNumFrameResources;
			_Monkey->_stones_p1[ix]._rItem[3]->NumFramesDirty = gNumFrameResources;
			_Monkey->_stones_p1[ix]._rItem[4]->NumFramesDirty = gNumFrameResources;

			_Monkey->_stones_p1[ix]._rItem[2]->Mat = _Materials["diskP10"].get();
			_Monkey->_stones_p1[ix]._rItem[3]->Mat = _Materials["diskP10"].get();
			_Monkey->_stones_p1[ix]._rItem[4]->Mat = _Materials["diskP10"].get();
		}
		else
		{
			_Monkey->_stones_p1[ix]._rItem[2]->NumFramesDirty = gNumFrameResources;
			_Monkey->_stones_p1[ix]._rItem[3]->NumFramesDirty = gNumFrameResources;
			_Monkey->_stones_p1[ix]._rItem[4]->NumFramesDirty = gNumFrameResources;

			_Monkey->_stones_p1[ix]._rItem[2]->Mat = _Materials["monkeyB0"].get();
			_Monkey->_stones_p1[ix]._rItem[3]->Mat = _Materials["monkeyB0"].get();
			_Monkey->_stones_p1[ix]._rItem[4]->Mat = _Materials["monkeyB0"].get();
		}
}

void MainWindow::InitDirect3D()
{
	GraphicsWindow::InitDirect3D();

	srand((unsigned int)time(nullptr));
	rand();

	SetTextMessage(std::wstring(_T(APP_NAME) L"\r\n"
		L"You vs Bernd\r\n"));

	G_set_gstone_placeholder(_g_stones_p1[0].get(), _PH_road_p1_arr[40], 40);
	G_set_gstone_placeholder(_g_stones_p1[1].get(), _PH_home_p1_arr[_g_stones_p1[1]->_home_idx], _g_stones_p1[1]->_home_idx);
	G_set_gstone_placeholder(_g_stones_p1[2].get(), _PH_road_p1_arr[10], 10);
	G_set_gstone_placeholder(_g_stones_p1[3].get(), _PH_road_p1_arr[30], 30);

	G_set_gstone_placeholder(_g_stones_p2[0].get(), _PH_road_p2_arr[5], 5);
	G_set_gstone_placeholder(_g_stones_p2[1].get(), _PH_home_p2_arr[_g_stones_p2[1]->_home_idx], _g_stones_p2[1]->_home_idx);
	G_set_gstone_placeholder(_g_stones_p2[2].get(), _PH_road_p2_arr[22], 22);
	G_set_gstone_placeholder(_g_stones_p2[3].get(), _PH_home_p2_arr[_g_stones_p2[3]->_home_idx], _g_stones_p2[3]->_home_idx);

	_g_stones_p1[0]->_selected = true;
	_selected_stone = _g_stones_p1[0].get();

	UpdateStonesPos();
	UpdateStonesSelection();
}

LRESULT MainWindow::OnStartNewGame()
{
	StartNewGame();
	return 0;
}

LRESULT MainWindow::OnKeyDown_f1()
{
	StartNewGame();
	return 0;
}

LRESULT MainWindow::OnKeyDown_return()
{
	FireProcessP1Msg(EV_DICE1_CLICKED);
	return 0;
}

void MainWindow::OnProcessP1Msg(WPARAM wp, LPARAM lp)
{
	switch (wp)
	{
	case EV_DICE1_CLICKED:
	{
		switch (_game_status)
		{
		case GS_NEW:
		case GS_TURN_P1:
		case GS_TURN_P1_HAS_6:
		{
			if (_dice_p1_count >= 1)
			{
				if (_Dice->_dice_cube_p1._need_update)
					break;

				_dice_p1_num = GetRandomDiceNum();
				assert(_dice_p1_num >= 1 && _dice_p1_num <= 6);

				--_dice_p1_count;
				GRollDiceP1(_dice_p1_num);
			}
		}
		break;

		}
	}
	break;
	
	case EV_DICE1_JUST_ROLLED:
	{
		switch (_game_status)
		{
		case GS_NEW:
		{
			FireProcessP2Msg(EV_START_NEW_GAME);
		}
		break;

		case GS_TURN_P1:
		{
			int ix = CheckP1_OneStoneAtHome();

			if (ix < 4)
			{
				if (!CheckP1_FirstPlaceFree())
				{
					ForceP1_MakeFirstPlaceFree();

					if (_dice_p1_num == 6)
					{
						_game_status = GS_TURN_P1;
						_dice_p1_count = 1;

						std::wstring msg;
						msg += L"You have 6\r\n";
						msg += L"Click the cube - Roll the dice";
						SetTextMessage(msg);
					}
					else
					{
						_game_status = GS_TURN_P1;
						_dice_p1_count = 0;
					}

					goto CHECK_P1_TURN;
				}
			}

			if (!(ix < 4) && _dice_p1_num == 6)
			{
				if (CanP1_Move())
				{
					_game_status = GS_TURN_P1_MOVE;
					_dice_p1_count = 1;

					std::wstring msg;
					msg += L"Click the stone - move";
					SetTextMessage(msg);
				}
				else
				{
					_game_status = GS_TURN_P1;
					_dice_p1_count = 1;
				}

				goto CHECK_P1_TURN;
			}
			
			if (CheckP1_OneStoneOnRoad() < 4 && _dice_p1_num != 6)
			{
				if (CanP1_Move())
				{
					_game_status = GS_TURN_P1_MOVE;
					_dice_p1_count = 1;

					std::wstring msg;
					msg += L"Click the stone - move";
					SetTextMessage(msg);
				}
				else
				{
					_game_status = GS_TURN_P1;
					if(GetP1_DiceCount() == 1)
						_dice_p1_count =  0;
				}

				goto CHECK_P1_TURN;
			}
			
			if ((ix < 4) && (_dice_p1_num == 6))
			{
				G_set_gstone_placeholder(_g_stones_p1[ix].get(), _PH_road_p1_arr[0], 0);
				UpdateStonesPos();

				if (CheckP1_OneStoneAtHome() < 4)
				{
					_game_status = GS_TURN_P1_HAS_6;
					_dice_p1_count = 1;

					std::wstring msg;
					msg += L"You have 6\r\n";
					msg += L"Click the cube - Roll the dice";
					SetTextMessage(msg);
				}
				else
				{
					_game_status = GS_TURN_P1;
					_dice_p1_count = 1;

					std::wstring msg;
					msg += L"You have 6\r\n";
					msg += L"Click the cube - Roll the dice";
					SetTextMessage(msg);
				}

				goto CHECK_P1_TURN;
			}

CHECK_P1_TURN:

			if (_dice_p1_count == 0)
			{
				FireProcessP1Msg(EV_P1_END);
				break;
			}
		}
		break;

		case GS_TURN_P1_HAS_6:
		{
			if (_PH_road_p1_arr[_dice_p1_num]->_gstone != nullptr &&
				_PH_road_p1_arr[_dice_p1_num]->_gstone->_type == GT_P1_STONE)
			{
				G_set_gstone_placeholder(_PH_road_p1_arr[0]->_gstone,
					_PH_home_p1_arr[_PH_road_p1_arr[0]->_gstone->_home_idx],
					_PH_road_p1_arr[0]->_gstone->_home_idx);
			}
			else
			{
				G_set_gstone_placeholder(_PH_road_p1_arr[0]->_gstone, _PH_road_p1_arr[_dice_p1_num], _dice_p1_num);
			}

			UpdateStonesPos();

			if (_dice_p1_num == 6)
			{
				_game_status = GS_TURN_P1;
				_dice_p1_count = 1;
				
				std::wstring msg;
				msg += L"You have again 6\r\n";
				msg += L"Click the cube - Roll the dice";
				SetTextMessage(msg);
			}
			else
			{
				FireProcessP1Msg(EV_P1_END);
			}
		}
		break;

		}
	}
	break;
	
	case EV_STONE_P1_SELECTED:
		StoneClicked((int)lp);
		break;

	case EV_START_NEW_GAME:
	{
		// Just wait the user to roll the dice
		_dice_p1_count = 1;
	}
	break;

	case EV_P1_TURN:
	{
		_game_status = GS_TURN_P1;
		_dice_p1_count = GetP1_DiceCount();

		std::wstring msg;
		msg += L"Your turn!\r\nClick the cube - Roll the dice ";
		msg += L"(" + std::to_wstring(_dice_p1_count) + L") times";
		SetTextMessage(msg);
	}
	break;

	case EV_P1_END:
	{
		if (CheckP1_Win())
		{
			_game_status = GS_FINISHED;

			std::wstring msg;
			msg += L"You win!\r\n";
			SetTextMessage(msg);

			_Wheel->_WheelRItem->Mat = _Materials["win0"].get();
			_Wheel->_visible = true;
		}
		else
		{
			FireProcessP2Msg(EV_P2_TURN);
		}
	}
	break;

	}
}

void MainWindow::OnProcessP2Msg(WPARAM wp, LPARAM lp)
{
	switch (wp)
	{
	case EV_DICE2_JUST_ROLLED:
	{
		switch (_game_status)
		{
		case GS_NEW:
		{
			if (_dice_p1_num == _dice_p2_num)
			{
				_game_status = GS_NEW;
				_dice_p1_count = 1;

				std::wstring msg;
				msg += L"Equal!\r\nClick the cube - Roll the dice again";
				SetTextMessage(msg);
			}
			else if (_dice_p1_num > _dice_p2_num)
			{
				FireProcessP1Msg(EV_P1_TURN);
			}
			else if (_dice_p1_num < _dice_p2_num)
			{
				FireProcessP2Msg(EV_P2_TURN);
			}
		}
		break;

		case GS_TURN_P2:
		{
			int ix = CheckP2_OneStoneAtHome();

			if (ix < 4)
			{
				if (!CheckP2_FirstPlaceFree())
				{
					ForceP2_MakeFirstPlaceFree();

					if (_dice_p2_num == 6)
					{
						_game_status = GS_TURN_P2;
						_dice_p2_count = 1;
					}
					else
					{
						_game_status = GS_TURN_P2;
						_dice_p2_count = 0;
					}

					goto CHECK_P2_TURN;
				}
			}

			if (!(ix < 4) && _dice_p2_num == 6)
			{
				if (CanP2_Move())
				{
					if (CheckP2_Win())
					{
						FireProcessP2Msg(EV_P2_END);
						break;
					}
					else
					{
						_game_status = GS_TURN_P2;
						_dice_p2_count = 1;
					}
				}
				else
				{
					_game_status = GS_TURN_P2;
					_dice_p2_count = 1;
				}

				goto CHECK_P2_TURN;
			}

			if (CheckP2_OneStoneOnRoad() < 4 && _dice_p2_num != 6)
			{
				if (CanP2_Move())
				{
					_game_status = GS_TURN_P2;
					_dice_p2_count = 0;
				}

				goto CHECK_P2_TURN;
			}



			if ((ix < 4) && (_dice_p2_num == 6))
			{
				G_set_gstone_placeholder(_g_stones_p2[ix].get(), _PH_road_p2_arr[0], 0);
				UpdateStonesPos();

				if (CheckP2_OneStoneAtHome() < 4)
				{
					_game_status = GS_TURN_P2_HAS_6;
					_dice_p2_count = 1;
				}
				else
				{
					_game_status = GS_TURN_P2;
					_dice_p2_count = 1;
				}

				goto CHECK_P2_TURN;
			}

CHECK_P2_TURN:

			if (_dice_p2_count == 0)
			{
				FireProcessP2Msg(EV_P2_END);
			}
			else
				FireProcessP2Msg(EV_P2_LOOP);
		}
		break;

		case GS_TURN_P2_HAS_6:
		{
			if (_PH_road_p2_arr[_dice_p2_num]->_gstone != nullptr &&
				_PH_road_p2_arr[_dice_p2_num]->_gstone->_type == GT_P2_STONE)
			{
				G_set_gstone_placeholder(_PH_road_p2_arr[0]->_gstone,
					_PH_home_p2_arr[_PH_road_p2_arr[0]->_gstone->_home_idx],
					_PH_road_p2_arr[0]->_gstone->_home_idx);
			}
			else
			{
				G_set_gstone_placeholder(_PH_road_p2_arr[0]->_gstone, _PH_road_p2_arr[_dice_p2_num], _dice_p2_num);
			}

			UpdateStonesPos();

			if (_dice_p2_num == 6)
			{
				_dice_p2_count = 1;
				_game_status = GS_TURN_P2;

				_dice_p2_num = GetRandomDiceNum();
				GRollDiceP2(_dice_p2_num);
			}
			else
			{
				FireProcessP2Msg(EV_P2_END);
			}
		}
		break;

		}
	}
	break;

	case EV_P2_LOOP:
	{
		if (_dice_p2_count >= 1)
		{
			if (_Dice->_dice_cube_p2._need_update)
				break;

			_dice_p2_num = GetRandomDiceNum();
			assert(_dice_p2_num >= 1 && _dice_p2_num <= 6);

			--_dice_p2_count;
			GRollDiceP2(_dice_p2_num);
		}
		else
		{
			FireProcessP1Msg(EV_P1_TURN);
			break;
		}
	}
	break;

	case EV_START_NEW_GAME:
	{
		_dice_p2_count = 1;
		_dice_p2_num = GetRandomDiceNum();
		GRollDiceP2(_dice_p2_num);
	}
	break;

	case EV_P2_END:
	{
		if (CheckP2_Win())
		{
			_game_status = GS_FINISHED;

			std::wstring msg;
			msg += L"Game Over!\r\n";
			SetTextMessage(msg);

			_Wheel->_WheelRItem->Mat = _Materials["lost0"].get();
			_Wheel->_visible = true;
		}
		else
		{
			FireProcessP1Msg(EV_P1_TURN);
		}
	}
	break;

	case EV_P2_TURN:
	{
		_game_status = GS_TURN_P2;
		_dice_p2_count = GetP2_DiceCount();

		std::wstring msg(L"Bernd turn!");
		SetTextMessage(msg);

		FireProcessP2Msg(EV_P2_LOOP);
	}
	break;

	};
}

void MainWindow::StartNewGame()
{
	_game_status = GS_NEW;
	_Wheel->_visible = false;

	SetTextMessage(std::wstring(L"Whose turn in the game?\r\nClick the cube - Roll the dice"));

	for (int ix = 0; ix < 4; ++ix)
	{
		G_set_gstone_placeholder(_g_stones_p1[ix].get(), _PH_home_p1_arr[_g_stones_p1[ix]->_home_idx], ix);
		G_set_gstone_placeholder(_g_stones_p2[ix].get(), _PH_home_p2_arr[_g_stones_p2[ix]->_home_idx], ix);
	}

	//G_set_gstone_placeholder(_g_stones_p1[0].get(), _PH_road_p1_arr[34], 34);
	//G_set_gstone_placeholder(_g_stones_p1[1].get(), _PH_road_p1_arr[41], 41);
	//G_set_gstone_placeholder(_g_stones_p1[2].get(), _PH_road_p1_arr[43], 43);
	//G_set_gstone_placeholder(_g_stones_p1[3].get(), _PH_road_p1_arr[42], 42);
	
	//G_set_gstone_placeholder(_g_stones_p2[0].get(), _PH_road_p2_arr[42], 42);
	//G_set_gstone_placeholder(_g_stones_p2[1].get(), _PH_road_p2_arr[43], 43);
	//G_set_gstone_placeholder(_g_stones_p2[2].get(), _PH_road_p2_arr[41], 41);
	//G_set_gstone_placeholder(_g_stones_p2[3].get(), _PH_road_p2_arr[34], 34);
	

	UpdateStonesPos();
	FireProcessP1Msg(EV_START_NEW_GAME);
}

int MainWindow::CheckP1_OneStoneAtHome()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_ph->_type == PH_HOME_P1)
			return ix;

	return ix;
}

int MainWindow::CheckP2_OneStoneAtHome()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p2[ix]->_ph->_type == PH_HOME_P2)
			return ix;

	return ix;
}

int MainWindow::CheckP1_OneStoneOnRoad()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_ph->_type == PH_ROAD || 
			_g_stones_p1[ix]->_ph->_type == PH_TARGET_P1)
			return ix;

	return ix;
}

int MainWindow::CheckP2_OneStoneOnRoad()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p2[ix]->_ph->_type == PH_ROAD ||
			_g_stones_p2[ix]->_ph->_type == PH_TARGET_P2 )
			return ix;

	return ix;
}

bool MainWindow::CheckP1_FirstPlaceFree()
{
	if (_PH_road_p1_arr[0]->_gstone != nullptr &&
		_PH_road_p1_arr[0]->_gstone->_type == GT_P1_STONE)
		return false;

	return true;
}

bool MainWindow::CheckP2_FirstPlaceFree()
{
	if (_PH_road_p2_arr[0]->_gstone != nullptr &&
		_PH_road_p2_arr[0]->_gstone->_type == GT_P2_STONE)
		return false;

	return true;
}

bool MainWindow::ForceP1_MakeFirstPlaceFree()
{
	for (int iy = 0; iy < 44 - _dice_p1_num; ++iy)
	{
		if (_PH_road_p1_arr[iy]->_gstone != nullptr &&
			_PH_road_p1_arr[iy]->_gstone->_type == GT_P1_STONE)
		{
			if (_PH_road_p1_arr[iy + _dice_p1_num]->_gstone == nullptr ||
				(_PH_road_p1_arr[iy + _dice_p1_num]->_gstone != nullptr &&
					_PH_road_p1_arr[iy + _dice_p1_num]->_gstone->_type == GT_P2_STONE))
			{
				G_set_gstone_placeholder(_PH_road_p1_arr[iy]->_gstone,
					_PH_road_p1_arr[iy + _dice_p1_num], iy + _dice_p1_num);
				UpdateStonesPos();
				return true;
			}
		}
	}

	return false;
}

bool MainWindow::ForceP2_MakeFirstPlaceFree()
{
	for (int iy = 0; iy < 44 - _dice_p2_num; ++iy)
	{
		if (_PH_road_p2_arr[iy]->_gstone != nullptr &&
			_PH_road_p2_arr[iy]->_gstone->_type == GT_P2_STONE)
		{
			if (_PH_road_p2_arr[iy + _dice_p2_num]->_gstone == nullptr ||
				(_PH_road_p2_arr[iy + _dice_p2_num]->_gstone != nullptr &&
					_PH_road_p2_arr[iy + _dice_p2_num]->_gstone->_type == GT_P1_STONE))
			{
				G_set_gstone_placeholder(_PH_road_p2_arr[iy]->_gstone,
					_PH_road_p2_arr[iy + _dice_p2_num], iy + _dice_p2_num);
				UpdateStonesPos();
				return true;
			}
		}
	}

	return false;
}

bool MainWindow::CanP1_Move()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_ph->_type == PH_ROAD ||
			_g_stones_p1[ix]->_ph->_type == PH_TARGET_P1)
		{
			if (_g_stones_p1[ix]->_ph_idx + _dice_p1_num < 44)
				if (_PH_road_p1_arr[_g_stones_p1[ix]->_ph_idx + _dice_p1_num]->_gstone == nullptr ||
					(_PH_road_p1_arr[_g_stones_p1[ix]->_ph_idx + _dice_p1_num]->_gstone != nullptr &&
						_PH_road_p1_arr[_g_stones_p1[ix]->_ph_idx + _dice_p1_num]->_gstone->_type != GT_P1_STONE))
				{
					bool canMove = true;

					if (_g_stones_p1[ix]->_ph_idx < 40)
					{
						if (_g_stones_p1[ix]->_ph_idx + _dice_p1_num > 40)
							for (int iy = 40; iy < _g_stones_p1[ix]->_ph_idx + _dice_p1_num; ++iy)
								if (_PH_road_p1_arr[iy]->_gstone != nullptr)
								{
									canMove = false;
									break;
								}
					}
					else
					{
						for (int iy = _g_stones_p1[ix]->_ph_idx + 1; iy < _g_stones_p1[ix]->_ph_idx + _dice_p1_num; ++iy)
							if (_PH_road_p1_arr[iy]->_gstone != nullptr)
							{
								canMove = false;
								break;
							}
					}

					if(canMove)
						return true;
				}
		}

	return false;
}

bool MainWindow::CanP2_Move()
{
	std::vector<BestMove> moveList;

	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p2[ix]->_ph->_type == PH_ROAD ||
			_g_stones_p2[ix]->_ph->_type == PH_TARGET_P2)
		{
			if (_g_stones_p2[ix]->_ph_idx + _dice_p2_num < 44)
				if (_PH_road_p2_arr[_g_stones_p2[ix]->_ph_idx + _dice_p2_num]->_gstone == nullptr ||
					(_PH_road_p2_arr[_g_stones_p2[ix]->_ph_idx + _dice_p2_num]->_gstone != nullptr &&
						_PH_road_p2_arr[_g_stones_p2[ix]->_ph_idx + _dice_p2_num]->_gstone->_type != GT_P2_STONE))
				{
					bool canMove = true;

					if (_g_stones_p2[ix]->_ph_idx < 40)
					{
						if (_g_stones_p2[ix]->_ph_idx + _dice_p2_num > 40)
							for (int iy = 40; iy < _g_stones_p2[ix]->_ph_idx + _dice_p2_num; ++iy)
								if (_PH_road_p2_arr[iy]->_gstone != nullptr)
								{
									canMove = false;
									break;
								}
					}
					else
					{
						for (int iy = _g_stones_p2[ix]->_ph_idx + 1; iy < _g_stones_p2[ix]->_ph_idx + _dice_p2_num; ++iy)
							if (_PH_road_p2_arr[iy]->_gstone != nullptr)
							{
								canMove = false;
								break;
							}
					}

					if(canMove)
					{
						moveList.push_back({ _g_stones_p2[ix].get(),
							_dice_p2_num,
							G_get_p2_move_priority(_g_stones_p2[ix].get(), _dice_p2_num) });
					}
				}
		}

	if (moveList.size())
	{
		BestMove bestMove = moveList[0];

		for (BestMove b : moveList)
		{
			if (b._priority > bestMove._priority)
				bestMove = b;
		}

		if (_PH_road_p2_arr[bestMove._stone->_ph_idx + bestMove._dice_num]->_gstone == nullptr ||
			(_PH_road_p2_arr[bestMove._stone->_ph_idx + bestMove._dice_num]->_gstone != nullptr &&
				_PH_road_p2_arr[bestMove._stone->_ph_idx + bestMove._dice_num]->_gstone->_type != GT_P1_STONE))
			CheckP2_WrongMove(bestMove._stone, bestMove._dice_num);

		G_set_gstone_placeholder(bestMove._stone,
			_PH_road_p2_arr[bestMove._stone->_ph_idx + bestMove._dice_num],
			bestMove._stone->_ph_idx + bestMove._dice_num);
		UpdateStonesPos();
	}

	return moveList.size() > 0;
}

void MainWindow::MoveSelectedStone()
{
	if (_game_status == GS_TURN_P1_MOVE)
	{
		if (_selected_stone->_ph->_type == PH_ROAD ||
			_selected_stone->_ph->_type == PH_TARGET_P1)
			if (_selected_stone->_ph_idx + _dice_p1_num < 44)
			{
				if (_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone == nullptr ||
					(_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone != nullptr &&
						_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone->_type != GT_P1_STONE))
				{
					bool canMove = true;

					if (_selected_stone->_ph_idx < 40)
					{
						if (_selected_stone->_ph_idx + _dice_p1_num > 40)
							for (int ix = 40; ix < _selected_stone->_ph_idx + _dice_p1_num; ++ix)
								if (_PH_road_p1_arr[ix]->_gstone != nullptr)
								{
									canMove = false;
									break;
								}
					}
					else
					{
						for (int ix = _selected_stone->_ph_idx + 1; ix < _selected_stone->_ph_idx + _dice_p1_num; ++ix)
							if (_PH_road_p1_arr[ix]->_gstone != nullptr)
							{
								canMove = false;
								break;
							}
					}


					if (canMove)
					{
						if (_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone == nullptr ||
							(_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone != nullptr &&
								_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num]->_gstone->_type != GT_P2_STONE))
							CheckP1_WrongMove();

						G_set_gstone_placeholder(_selected_stone,
							_PH_road_p1_arr[_selected_stone->_ph_idx + _dice_p1_num],
							_selected_stone->_ph_idx + _dice_p1_num);


						UpdateStonesPos();

						if (_dice_p1_num != 6)
							FireProcessP1Msg(EV_P1_END);
						else
						{
							if (CheckP1_Win())
							{
								FireProcessP1Msg(EV_P1_END);
							}
							else
							{
								_game_status = GS_TURN_P1;
								_dice_p1_count = 1;

								std::wstring msg;
								msg += L"You have 6\r\n";
								msg += L"Click the cube - Roll the dice";
								SetTextMessage(msg);
							}
						}
					}
				}
			}
	}
}

int MainWindow::GetRandomDiceNum()
{
	return (rand() % (7 - 1)) + 1;
}

void MainWindow::StoneClicked(int idx)
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_selected)
		{
			_g_stones_p1[ix]->_selected = false;
			break;
		}

	_g_stones_p1[idx]->_selected = true;
	_selected_stone = _g_stones_p1[idx].get();
	
	UpdateStonesSelection();

	MoveSelectedStone();
}


void MainWindow::G_set_gstone_placeholder(GStone* gstone, PlaceHolder* ph, int idx)
{
	if (gstone->_ph != nullptr)
	{
		gstone->_ph->_gstone = nullptr;
		gstone->_ph = nullptr;
	}

	if (ph->_gstone != nullptr)
	{
		if (gstone->_type == GT_P1_STONE &&
			ph->_gstone->_type == GT_P2_STONE)
		{
			// move p2 stone to home
			G_set_gstone_placeholder(ph->_gstone,
				_PH_home_p2_arr[ph->_gstone->_home_idx],
				ph->_gstone->_home_idx);
		}
		else if (gstone->_type == GT_P2_STONE &&
			ph->_gstone->_type == GT_P1_STONE)
		{
			// move p1 stone to home
			G_set_gstone_placeholder(ph->_gstone,
				_PH_home_p1_arr[ph->_gstone->_home_idx],
				ph->_gstone->_home_idx);
		}
		else
		{
			printf("G_set_gstone_placehlder: error\r\n");
		}
	}

	gstone->_ph = ph;
	ph->_gstone = gstone;
	gstone->_ph_idx = idx;
}

int MainWindow::GetP1_DiceCount()
{
	if (_g_stones_p1[0]->_ph->_type == PH_ROAD ||
		_g_stones_p1[1]->_ph->_type == PH_ROAD ||
		_g_stones_p1[2]->_ph->_type == PH_ROAD ||
		_g_stones_p1[3]->_ph->_type == PH_ROAD)
	{
		return 1;
	}

	for (int ix = 0; ix < 3; ++ix)
	{
		if (_PH_target_p1_arr[ix]->_gstone != nullptr)
		{
			for (int iy = ix + 1; iy < 4; ++iy)
			{
				if (_PH_target_p1_arr[iy]->_gstone == nullptr)
				{
					return 1;
				}
			}
		}
	}

	return 3;
}

int MainWindow::GetP2_DiceCount()
{
	if (_g_stones_p2[0]->_ph->_type == PH_ROAD ||
		_g_stones_p2[1]->_ph->_type == PH_ROAD ||
		_g_stones_p2[2]->_ph->_type == PH_ROAD ||
		_g_stones_p2[3]->_ph->_type == PH_ROAD)
	{
		return 1;
	}

	for (int ix = 0; ix < 3; ++ix)
	{
		if (_PH_target_p2_arr[ix]->_gstone != nullptr)
		{
			for (int iy = ix + 1; iy < 4; ++iy)
			{
				if (_PH_target_p2_arr[iy]->_gstone == nullptr)
				{
					return 1;
				}
			}
		}
	}

	return 3;
}

bool MainWindow::CheckP1_Win()
{
	if (_PH_target_p1_arr[0]->_gstone != nullptr &&
		_PH_target_p1_arr[1]->_gstone != nullptr &&
		_PH_target_p1_arr[2]->_gstone != nullptr &&
		_PH_target_p1_arr[3]->_gstone != nullptr)
		return true;

	return false;
}

bool MainWindow::CheckP2_Win()
{
	if (_PH_target_p2_arr[0]->_gstone != nullptr &&
		_PH_target_p2_arr[1]->_gstone != nullptr &&
		_PH_target_p2_arr[2]->_gstone != nullptr &&
		_PH_target_p2_arr[3]->_gstone != nullptr)
		return true;

	return false;
}

LRESULT MainWindow::OnKeyDown_left_arrow()
{
	int ix = 0;
	for(; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_selected)
		{
			break;
		}

	if (ix < 3)
	{
		_g_stones_p1[ix]->_selected = false;
		_g_stones_p1[ix + 1]->_selected = true;
		_selected_stone = _g_stones_p1[ix + 1].get();
	}
	else
	{
		_g_stones_p1[ix]->_selected = false;
		_g_stones_p1[0]->_selected = true;
		_selected_stone = _g_stones_p1[0].get();
	}

	UpdateStonesSelection();
	return 0;
}

LRESULT MainWindow::OnKeyDown_right_arrow()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix]->_selected)
		{
			break;
		}

	if (ix > 0)
	{
		_g_stones_p1[ix]->_selected = false;
		_g_stones_p1[ix - 1]->_selected = true;
		_selected_stone = _g_stones_p1[ix - 1].get();
	}
	else
	{
		_g_stones_p1[ix]->_selected = false;
		_g_stones_p1[3]->_selected = true;
		_selected_stone = _g_stones_p1[3].get();
	}

	UpdateStonesSelection();
	return 0;
}

LRESULT MainWindow::OnChar_space()
{
	MoveSelectedStone();

	return 0;
}


void MainWindow::CheckP1_WrongMove()
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p1[ix].get() != _selected_stone &&
			_g_stones_p1[ix]->_ph->_type == PH_ROAD)
		{
			if (_g_stones_p1[ix]->_ph_idx + _dice_p1_num < 44)
				if (_PH_road_p1_arr[_g_stones_p1[ix]->_ph_idx + _dice_p1_num]->_gstone != nullptr &&
						_PH_road_p1_arr[_g_stones_p1[ix]->_ph_idx + _dice_p1_num]->_gstone->_type == GT_P2_STONE)
				{
					G_set_gstone_placeholder(_g_stones_p1[ix].get(),
						_PH_home_p1_arr[_g_stones_p1[ix]->_home_idx],
						_g_stones_p1[ix]->_home_idx);

					UpdateStonesPos();
					return;
				}
		}
}

void MainWindow::CheckP2_WrongMove(GStone* stone, int dice_num)
{
	int ix = 0;
	for (; ix < 4; ++ix)
		if (_g_stones_p2[ix].get() != stone &&
			_g_stones_p2[ix]->_ph->_type == PH_ROAD)
		{
			if (_g_stones_p2[ix]->_ph_idx + dice_num < 44)
				if (_PH_road_p2_arr[_g_stones_p2[ix]->_ph_idx + dice_num]->_gstone != nullptr &&
					_PH_road_p2_arr[_g_stones_p2[ix]->_ph_idx + dice_num]->_gstone->_type == GT_P1_STONE)
				{
					G_set_gstone_placeholder(_g_stones_p2[ix].get(),
						_PH_home_p2_arr[_g_stones_p2[ix]->_home_idx],
						_g_stones_p2[ix]->_home_idx);

					UpdateStonesPos();
					return;
				}
		}
}

int MainWindow::G_get_p2_move_priority(GStone* stone, int dice_num)
{
	if (_PH_road_p2_arr[stone->_ph_idx + dice_num]->_gstone != nullptr)
		return 1000;

	if (_PH_road_p2_arr[stone->_ph_idx]->_type == PH_TARGET_P2)
		return 900;

	if (_PH_road_p2_arr[stone->_ph_idx + dice_num]->_type == PH_TARGET_P2)
		return 800;

	if (stone->_ph_idx == 20)
		return 700;

	if (stone->_ph_idx > 20)
		return 600;

	if (stone->_ph_idx < 20 && stone->_ph_idx + dice_num < 20)
	{
		bool cross = false;

		for (int ix = stone->_ph_idx; ix < stone->_ph_idx + dice_num; ++ix)
			if (_PH_road_p2_arr[ix]->_gstone != nullptr &&
				_PH_road_p2_arr[ix]->_gstone->_type == GT_P1_STONE)
				cross = true;

		if (!cross)
			return 500;
	}

	if (stone->_ph_idx < 20 && stone->_ph_idx + dice_num < 20)
		return 400;

	return 100;
}
