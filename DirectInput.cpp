#include "d3dUtil.h"
#include "DirectInput.h"
#include "d3dApp.h"
#include "Window.h"

DirectInput* gDInput = 0;

DirectInput::DirectInput( DWORD keyboardCoopFlags,
                          DWORD mouseCoopFlags)
{
  HRESULT hr;

  ZeroMemory( mKeyboardState, sizeof(mKeyboardState) );
  ZeroMemory( &mMouseState, sizeof(mMouseState) );

  hr = DirectInput8Create(GlobalInstance(),
                          DIRECTINPUT_VERSION,
                          IID_IDirectInput8,
                          (void**)&mDInput,
                          0);
  PD( hr, L"create direct input 8" );

  hr = mDInput->CreateDevice( GUID_SysKeyboard, &mKeyboard, 0 ),
  PD( hr, L"create device" );

  hr = mKeyboard->SetDataFormat( &c_dfDIKeyboard );
  PD( hr, L"set data format" );

  hr = mKeyboard->SetCooperativeLevel(GlobalWindowHandle(),
                                      keyboardCoopFlags);
  PD( hr, L"set cooperative level" );

  //hr = mKeyboard->Acquire();
  //PD( hr, L"aquire" );

  hr = mDInput->CreateDevice( GUID_SysMouse, &mMouse, 0 );
  PD( hr, L"create device" );

  PD( mMouse->SetDataFormat( &c_dfDIMouse2 ),
      L"set data format");

  PD( mMouse->SetCooperativeLevel( GlobalWindowHandle(),
                                   mouseCoopFlags ),
      L"set cooperative level");

  //PD( mMouse->Acquire(),
  //    L"aquire");
}

DirectInput::~DirectInput()
{
  ReleaseCOM(mDInput)
  mKeyboard->Unacquire();
  mMouse->Unacquire();
  ReleaseCOM(mKeyboard)
  ReleaseCOM(mMouse)
}

void DirectInput::poll()
{
  // Poll keyboard.
  HRESULT hr = mKeyboard->GetDeviceState(sizeof(mKeyboardState), (void**)&mKeyboardState);
  if( FAILED(hr) )
  {
    // Keyboard lost, zero out keyboard data structure.
    ZeroMemory(mKeyboardState, sizeof(mKeyboardState));

    // Try to acquire for next time we poll.
    hr = mKeyboard->Acquire();
  }

  // Poll mouse.
  hr = mMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&mMouseState);
  if( FAILED(hr) )
  {
    // Mouse lost, zero out mouse data structure.
    ZeroMemory(&mMouseState, sizeof(mMouseState));

    // Try to acquire for next time we poll.
    hr = mMouse->Acquire();
  }
}

bool DirectInput::keyDown(char key)
{
  return (mKeyboardState[key] & 0x80) != 0;
}

bool DirectInput::mouseButtonDown(int button)
{
  return (mMouseState.rgbButtons[button] & 0x80) != 0;
}

float DirectInput::mouseDX()
{
  return (float)mMouseState.lX;
}

float DirectInput::mouseDY()
{
  return (float)mMouseState.lY;
}

float DirectInput::mouseDZ()
{
  return (float)mMouseState.lZ;
}
