#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <GraphicsWindow.h>

#define EV_START_NEW_GAME	EV_CLASS_DEFINED + 1
#define EV_P1_TURN			EV_CLASS_DEFINED + 2
#define EV_P2_TURN			EV_CLASS_DEFINED + 3
#define EV_P1_END			EV_CLASS_DEFINED + 4
#define EV_P2_END			EV_CLASS_DEFINED + 5
#define EV_P2_LOOP			EV_CLASS_DEFINED + 6

typedef struct _GStone GStone;

typedef enum {PH_HOME_P1 = 0, PH_HOME_P2, PH_ROAD, PH_TARGET_P1, PH_TARGET_P2} PlaceHolderType;
typedef enum { GT_P1_STONE = 0, GT_P2_STONE } GStoneType;

typedef struct _PlaceHolder
{
	_PlaceHolder(PlaceHolderType t, int x, int y) : _type(t), _x(x), _y(y), _gstone(nullptr) {}

	const PlaceHolderType _type;
	const int _x, _y;

	GStone* _gstone;
} PlaceHolder;

typedef struct _GStone
{
	_GStone(GStoneType t, int homeIdx) : _type(t), _home_idx(homeIdx), _ph(nullptr), _ph_idx(-1), _selected(false) {}

	const GStoneType _type;
	const int _home_idx;

	PlaceHolder* _ph;
	int _ph_idx;
	bool _selected;
} GStone;

typedef struct _BestMove
{
	GStone* _stone;
	int _dice_num;
	int _priority;
} BestMove;

class MainWindow : public GraphicsWindow
{
	friend BOOL BaseWindow::Create(PCTSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, 
		int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu);

public:
	typedef enum {GS_INITIAL = 0, GS_NEW, 
		GS_TURN_P1, GS_TURN_P2, 
		GS_TURN_P1_HAS_6, GS_TURN_P2_HAS_6,
		GS_TURN_P1_MOVE, GS_TURN_P2_MOVE, 
		GS_FINISHED} GameStatus;

	MainWindow();
	virtual ~MainWindow();

	void InitDirect3D();

public:
	virtual LRESULT OnKeyDown_return();
	virtual LRESULT OnKeyDown_left_arrow();
	virtual LRESULT OnKeyDown_right_arrow();
	virtual LRESULT OnKeyDown_f1();
	virtual LRESULT OnChar_space();
	virtual LRESULT OnStartNewGame();

	virtual void OnProcessP1Msg(WPARAM wp, LPARAM lp);
	virtual void OnProcessP2Msg(WPARAM wp, LPARAM lp);

protected:
	std::array <std::unique_ptr<PlaceHolder>, 56> _placeholders;

	std::array <PlaceHolder*, 44> _PH_road_p1_arr;
	std::array <PlaceHolder*, 44> _PH_road_p2_arr;
	std::array <PlaceHolder*, 4> _PH_home_p1_arr;
	std::array <PlaceHolder*, 4> _PH_home_p2_arr;
	std::array <PlaceHolder*, 4> _PH_target_p1_arr;
	std::array <PlaceHolder*, 4> _PH_target_p2_arr;

	std::array <std::unique_ptr<GStone>, 4> _g_stones_p1, _g_stones_p2;
	GStone* _selected_stone;

	GameStatus _game_status = GS_INITIAL;
	int _dice_p1_num = 0;
	int _dice_p1_count = 0;
	int _dice_p2_num = 0;
	int _dice_p2_count = 0;

protected:
	void UpdateStonesPos();
	void UpdateStonesSelection();

	int GetRandomDiceNum();
	void StartNewGame();
	
	void StoneClicked(int idx);
	void MoveSelectedStone();
	
	int CheckP1_OneStoneAtHome();
	int CheckP2_OneStoneAtHome();
	int CheckP1_OneStoneOnRoad();
	int CheckP2_OneStoneOnRoad();
	bool CheckP1_FirstPlaceFree();
	bool CheckP2_FirstPlaceFree();

	bool ForceP1_MakeFirstPlaceFree();
	bool ForceP2_MakeFirstPlaceFree();
	bool CanP1_Move();
	bool CanP2_Move();
	
	bool CheckP1_Win();
	bool CheckP2_Win();
	void G_set_gstone_placeholder(GStone* gstone, PlaceHolder* ph, int idx);

	int GetP1_DiceCount();
	int GetP2_DiceCount();

	void CheckP1_WrongMove();
	void CheckP2_WrongMove(GStone* stone, int dice_num);
	int G_get_p2_move_priority(GStone* stone, int dice_num);
};

#endif /* _MAIN_WINDOW_H_ */
