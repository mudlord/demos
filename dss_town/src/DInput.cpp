//-----------------------------------------------------------------------------
// Module: Direct input support
// Author: hitchhikr
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "DInput.h"

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
LPDIRECTINPUT8A IDirectInputDevice;
LPDIRECTINPUTDEVICE8 KeyboardDevice;
LPDIRECTINPUTDEVICE8 MouseDevice;
BYTE *g_Keys;
BYTE *g_Keys_No_Repeat;
int Kb_Ack;
int Ms_Ack;
DIMOUSESTATE g_Mouse;
int Mouse_X;
int Mouse_Y;
int Mouse_Z;
int Mouse_Left_Button;
int Mouse_Middle_Button;
int Mouse_Right_Button;

//-----------------------------------------------------------------------------
// Name: Get_DirectInput
// Desc: Open directinput & obtain the keyboard & the mouse
// Out: DINPUT_OK (notification)
//      DINPUT_ERR_DINPUT (critical error)
//      DINPUT_ERR_KEYBOARD (notification)
//      DINPUT_ERR_MOUSE (notification)
//-----------------------------------------------------------------------------
int Get_DirectInput(HINSTANCE hInstance, HWND hWnd) {
    int Error_Value = DINPUT_OK;

    if(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **) &IDirectInputDevice, NULL) != DI_OK) {
        return(DINPUT_ERR_DINPUT);
    }
			
	if(IDirectInputDevice->CreateDevice(GUID_SysKeyboard, &KeyboardDevice, NULL) != DI_OK) {
        Error_Value |= DINPUT_ERR_KEYBOARD;
    } else {
        if(KeyboardDevice->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
            Error_Value |= DINPUT_ERR_KEYBOARD;
        } else {
            if(KeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) != DI_OK) {
                Error_Value |= DINPUT_ERR_KEYBOARD;
            } else {
                g_Keys = (BYTE *) malloc(256);
                memset(g_Keys, 0, 256);
                g_Keys_No_Repeat = (BYTE *) malloc(256);
                memset(g_Keys, 0, 256);
            }
        }
    }

    if(IDirectInputDevice->CreateDevice(GUID_SysMouse, &MouseDevice, NULL) != DI_OK) {
        Error_Value |= DINPUT_ERR_MOUSE;
    } else {
        if(MouseDevice->SetDataFormat(&c_dfDIMouse) != DI_OK) {
            Error_Value |= DINPUT_ERR_MOUSE;
        } else {
            if(MouseDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) != DI_OK) {
                Error_Value |= DINPUT_ERR_MOUSE;

            }
        }
    }
    return(Error_Value);
}

//-----------------------------------------------------------------------------
// Name: Release_DirectInput
// Desc: Free the allocated resources
//-----------------------------------------------------------------------------
void Release_DirectInput(void) {
	if(MouseDevice) MouseDevice->Release();
	if(KeyboardDevice) KeyboardDevice->Release();
	if(g_Keys_No_Repeat) free(g_Keys_No_Repeat);
	if(g_Keys) free(g_Keys);
	if(IDirectInputDevice) IDirectInputDevice->Release();
    MouseDevice = NULL;
    KeyboardDevice = NULL;
    g_Keys_No_Repeat = NULL;
    g_Keys = NULL;
    IDirectInputDevice = NULL;
}

//-----------------------------------------------------------------------------
// Name: Clear_Keys_Buffer
// Desc: Flush the keyboard buffer
//-----------------------------------------------------------------------------
void Clear_Keys_Buffer(void) {
    memset(g_Keys, 0, 256);
}

//-----------------------------------------------------------------------------
// Name: Clear_Mouse_Buffer
// Desc: Flush the mouse buffer
//-----------------------------------------------------------------------------
void Clear_Mouse_Buffer(void) {
    memset(&g_Mouse, 0, sizeof(DIMOUSESTATE));
}

//-----------------------------------------------------------------------------
// Name: Keyboard_Read
// Desc: Read the datas from keyboard
//-----------------------------------------------------------------------------
int Keyboard_Read(void) {
    if(KeyboardDevice) {
	    KeyboardDevice->Poll();
        if(KeyboardDevice->GetDeviceState(256, g_Keys) != DI_OK) {
			if(KeyboardDevice->Acquire() != DI_OK) {
				Clear_Keys_Buffer();
				return(0);
            }
			KeyboardDevice->Poll();
			if(KeyboardDevice->GetDeviceState(256, g_Keys) != DI_OK) {
				Clear_Keys_Buffer();
				return(FALSE);
            }
		    return(TRUE);
        }
    }
    return(FALSE);
}

//-----------------------------------------------------------------------------
// Name: Mouse_Read
// Desc: Read the datas from mouse
//-----------------------------------------------------------------------------
int Mouse_Read(void) {
    if(MouseDevice) {
		MouseDevice->Poll();
		if(MouseDevice->GetDeviceState(sizeof DIMOUSESTATE, &g_Mouse) != DI_OK) {
			if(MouseDevice->Acquire() != DI_OK) {
				Clear_Mouse_Buffer();
				return(FALSE);
			}
			MouseDevice->Poll();
			if(MouseDevice->GetDeviceState(sizeof DIMOUSESTATE, &g_Mouse) != DI_OK) {
				Clear_Mouse_Buffer();
                return(FALSE);
			}
        }
		Mouse_X = g_Mouse.lX;
		Mouse_Y = g_Mouse.lY;
		Mouse_Z = g_Mouse.lZ;
		if(g_Mouse.rgbButtons[0]) Mouse_Left_Button = 1;
        else Mouse_Left_Button = 0;
		if(g_Mouse.rgbButtons[1]) Mouse_Right_Button = 1;
        else Mouse_Right_Button = 0;
		if(g_Mouse.rgbButtons[2]) Mouse_Middle_Button = 1;
        else Mouse_Middle_Button = 0;
        return(TRUE);
    }
    return(FALSE);
}

//-----------------------------------------------------------------------------
// Name: Input_LostFocus
// Desc: Release input
//-----------------------------------------------------------------------------
void Input_LostFocus(void) {
    if(KeyboardDevice) {
        if(Kb_Ack) {
            Clear_Keys_Buffer();
            KeyboardDevice->Unacquire();
            Kb_Ack = FALSE;
        }
    }
    if(MouseDevice) {
        if(Ms_Ack) {
            Clear_Mouse_Buffer();
            MouseDevice->Unacquire();
            Ms_Ack = FALSE;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: Input_GotFocus
// Desc: Restore input
//-----------------------------------------------------------------------------
void Input_GotFocus(void) {
    if(KeyboardDevice) {
        if(!Kb_Ack) {
            KeyboardDevice->Acquire();
            Clear_Keys_Buffer();
            Kb_Ack = TRUE;
        }
    }
    if(MouseDevice) {
        if(!Ms_Ack) {
            Clear_Mouse_Buffer();
            MouseDevice->Acquire();
            Ms_Ack = TRUE;
        }
    }
}

// ------------------------------------------------------
// Name: DInput_Clamp_Mouse
// Desc: Make the mouse coordinates fit inside a given range
void DInput_Clamp_Mouse(int Speed, int *Coord, int Min, int Max) {
    int Cur_Speed = Speed;

	if(*Coord <= Min) {
		*Coord = Min;
		if(Speed < 0) Cur_Speed = 0;
    }
	if(*Coord >= Max) {
		*Coord = Max;
		if(Speed > 0) Cur_Speed = 0;
    }
    *Coord += Cur_Speed;
	if(*Coord <= Min) *Coord = Min;
	if(*Coord >= Max) *Coord = Max;
}

//-----------------------------------------------------------------------------
// Name: DInput_Get_Key_NoRepeat
// Desc: Return the state of a key only one time
//-----------------------------------------------------------------------------
int DInput_Get_Key_NoRepeat(int Key) {
    if(g_Keys[Key]) {
		if(!g_Keys_No_Repeat[Key]) {
			g_Keys_No_Repeat[Key]++;
            return(TRUE);
        }
        return(FALSE);
    }
    g_Keys_No_Repeat[Key] = 0;
    return(FALSE);
}

//-----------------------------------------------------------------------------
// Name: DInput_Get_Key
// Desc: Return the state of a key
//-----------------------------------------------------------------------------
int DInput_Get_Key(int Key) {
    return(g_Keys[Key]);
}
