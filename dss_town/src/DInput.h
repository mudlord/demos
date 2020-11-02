//-----------------------------------------------------------------------------
// Module: Direct input support
// Author: hitchhikr
//-----------------------------------------------------------------------------

#ifndef _DINPUT_H_
#define _DINPUT_H_

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define DINPUT_OK 0
#define DINPUT_ERR_DINPUT 1
#define DINPUT_ERR_KEYBOARD 2
#define DINPUT_ERR_MOUSE 4

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
extern int Mouse_X;
extern int Mouse_Y;
extern int Mouse_Z;
extern int Mouse_Left_Button;
extern int Mouse_Middle_Button;
extern int Mouse_Right_Button;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
int Get_DirectInput(HINSTANCE hInstance, HWND hWnd);
void Release_DirectInput(void);
int Keyboard_Read(void);
int Mouse_Read(void);
void DInput_Clamp_Mouse(int Speed, int *Coord, int Min, int Max);
int DInput_Get_Key_NoRepeat(int Key);
int DInput_Get_Key(int Key);
void Input_LostFocus(void);
void Input_GotFocus(void);

#endif
