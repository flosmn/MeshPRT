#ifndef DIRECT_INPUT_H
#define DIRECT_INPUT_H

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class DirectInput
{
public:
    DirectInput(DWORD keyboardCoopFlags, DWORD mouseCoopFlags);
    ~DirectInput();

    void poll();
    bool keyDown(char key);
    bool mouseButtonDown(int button);
    float mouseDX();
    float mouseDY();
    float mouseDZ();

private:
    // Make private to prevent copying of members of this class.
    DirectInput(const DirectInput& rhs);
    DirectInput& operator=(const DirectInput& rhs);

private:
    IDirectInput8*       mDInput;

    IDirectInputDevice8* mKeyboard;
    char                 mKeyboardState[256];

    IDirectInputDevice8* mMouse;
    DIMOUSESTATE2        mMouseState;
};
extern DirectInput* gDInput;

#endif // DIRECT_INPUT_H
