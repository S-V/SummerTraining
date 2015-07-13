/*
=============================================================================
	File:	Driver.cpp
	Desc:	
	References:
	Taking Advantage of High-Definition Mouse Movement
	http://msdn.microsoft.com/en-us/library/windows/desktop/ee418864%28v=vs.85%29.aspx
	Developing mouse controls (DirectX and C++)
	http://msdn.microsoft.com/en-us/library/windows/apps/hh994925.aspx
	Raw Input
	http://msdn.microsoft.com/en-us/library/ms645536%28VS.85%29.aspx
	Handling Multiple Mice with Raw Input
	http://asawicki.info/news_1533_handling_multiple_mice_with_raw_input.html
	Input for Modern PC Games
	http://www.altdevblogaday.com/2011/04/25/input-for-modern-pc-games/
	Properly handling keyboard input
	http://molecularmusings.wordpress.com/2011/09/05/properly-handling-keyboard-input/
=============================================================================
*/
#if defined(TB_USE_SDL)
#include <Driver/Driver_PCH.h>
#pragma hdrstop
#include <Core/Event.h>
#include <Core/Client.h>
#include <Driver/Driver.h>

#define USE_DEFERRED_EVENT_PROCESSING	(0)

#if 1
	#define DBG_MSG(...)
#else
	#define DBG_MSG(...)\
		DBGOUT(__VA_ARGS__)
#endif

enum {
	DEFAULT_SCREEN_WIDTH = 1024,
	DEFAULT_SCREEN_HEIGHT = 768
};

// https://gist.github.com/shakesoda/11238165
struct SDL_TimeData {
	Uint64 then;
	Uint64 now;
	Uint64 frequency;
	double deltaSeconds;
};
class SDL_Timer {
public:
	SDL_Timer() {
		m_time.now = SDL_GetPerformanceCounter();
		m_time.then = m_time.now;
		m_time.frequency = SDL_GetPerformanceFrequency();
	}
	// Note: delta is since last touch OR peek
	const SDL_TimeData& touch() {
		m_time.then = m_time.now;
		m_time.now = SDL_GetPerformanceCounter();
		m_time.deltaSeconds = double(m_time.now - m_time.then) / double(m_time.frequency);
		return m_time;
	}
	// Note: delta is since last touch
	const SDL_TimeData& peek() {
		m_time.now = SDL_GetPerformanceCounter();
		m_time.deltaSeconds = double(m_time.now - m_time.then) / double(m_time.frequency);
		return m_time;
	}
protected:
	SDL_TimeData m_time;
};

extern void HACK_ImGui_ProcessTextInput( const char* text );

mxSWIPED("from tesseract/sauerbraten");
namespace WindowsDriver
{
	static Uint64 g_lastFrameMicroseconds = 0;
	static double g_lastFrameDeltaSeconds = 0;

	static int g_screenWidth = 0;
	static int g_screenHeight = 0;
	static SDL_Window* g_window = NULL;	// the main window

	static bool g_bHasFocus = false;
	static bool g_bRequestedExit = false;

	static const bool bProcessKeyboardInput = true;
	// if false, process mouse events won't be processed.
	static const bool bProcessMouseButtons = true;
	static const bool bProcessMouseMotion = true;

	// While the mouse is in relative mode, the cursor is hidden,
	// and the driver will try to report continuous motion in the current window.
	// Only relative motion events will be delivered, the mouse position will not change.
	static bool bRelativeMouseMode = false;

	// used to restore prev mouse cursor position when releasing mouse capture
	static Int2	g_savedMousePosition = { 0, 0 };

	// currently held key modifiers (e.g. Ctrl, Alt, Shift)
	static Uint16 g_modifiers = 0;

	//Uint32 g_mouseState = 0;

	static Sint32	g_keycodes[256];
	static UINT8	g_keyboard[MAX_KEYS];	// keyboard state (pressed/released)

	static inline EKeyCode GetKeyCode( const SDL_Keysym& keysym )
	{
		for( int i = 0; i < mxCOUNT_OF(g_keycodes); i++ )
		{
			if( keysym.sym == g_keycodes[i] ) {
				return (EKeyCode) i;
			}
		}
		ptERROR("Bad SDL keycode: '%d'\n",keysym.sym);
		return KEY_Unknown;
	}

	static void ProcessEvent( const SystemEvent& _event, UINT32 states );
	static ERet DoSetRelativeMouseMode( bool enable );

	Settings::Settings()
	{
		name = "Unnamed";
		screenWidth = DEFAULT_SCREEN_WIDTH;
		screenHeight = DEFAULT_SCREEN_HEIGHT;
		fullScreen = false;
		resizable = false;
	}
	void InitKeycodeTable()
	{
		mxZERO_OUT(g_keycodes);

		g_keycodes[KEY_Unknown] = SDLK_UNKNOWN;

		g_keycodes[KEY_Escape] = SDLK_ESCAPE;
		g_keycodes[KEY_Backspace] = SDLK_BACKSPACE;
		g_keycodes[KEY_Tab] = SDLK_TAB;
		g_keycodes[KEY_Enter] = SDLK_RETURN;
		g_keycodes[KEY_Space] = SDLK_SPACE;

		g_keycodes[KEY_LShift] = SDLK_LSHIFT;
		g_keycodes[KEY_RShift] = SDLK_RSHIFT;
		g_keycodes[KEY_LCtrl] = SDLK_LCTRL;
		g_keycodes[KEY_RCtrl] = SDLK_RCTRL;
		g_keycodes[KEY_LAlt] = SDLK_LALT;
		g_keycodes[KEY_RAlt] = SDLK_RALT;

		g_keycodes[KEY_LWin] = SDLK_LGUI;
		g_keycodes[KEY_RWin] = SDLK_RGUI;
		g_keycodes[KEY_Apps] = SDLK_APPLICATION;

		g_keycodes[KEY_Pause] = SDLK_PAUSE;
		g_keycodes[KEY_Capslock] = SDLK_CAPSLOCK;
		g_keycodes[KEY_Numlock] = SDLK_NUMLOCKCLEAR;
		g_keycodes[KEY_Scrolllock] = SDLK_SCROLLLOCK;
		g_keycodes[KEY_PrintScreen] = SDLK_PRINTSCREEN;

		g_keycodes[KEY_PgUp] = SDLK_PAGEUP;
		g_keycodes[KEY_PgDn] = SDLK_PAGEDOWN;
		g_keycodes[KEY_Home] = SDLK_HOME;
		g_keycodes[KEY_End] = SDLK_END;
		g_keycodes[KEY_Insert] = SDLK_INSERT;
		g_keycodes[KEY_Delete] = SDLK_DELETE;

		g_keycodes[KEY_Left] = SDLK_LEFT;
		g_keycodes[KEY_Up] = SDLK_UP;
		g_keycodes[KEY_Right] = SDLK_RIGHT;
		g_keycodes[KEY_Down] = SDLK_DOWN;

		g_keycodes[KEY_0] = SDLK_0;
		g_keycodes[KEY_1] = SDLK_1;
		g_keycodes[KEY_2] = SDLK_2;
		g_keycodes[KEY_3] = SDLK_3;
		g_keycodes[KEY_4] = SDLK_4;
		g_keycodes[KEY_5] = SDLK_5;
		g_keycodes[KEY_6] = SDLK_6;
		g_keycodes[KEY_7] = SDLK_7;
		g_keycodes[KEY_8] = SDLK_8;
		g_keycodes[KEY_9] = SDLK_9;

		g_keycodes[KEY_A] = SDLK_a;
		g_keycodes[KEY_B] = SDLK_b;
		g_keycodes[KEY_C] = SDLK_c;
		g_keycodes[KEY_D] = SDLK_d;
		g_keycodes[KEY_E] = SDLK_e;
		g_keycodes[KEY_F] = SDLK_f;
		g_keycodes[KEY_G] = SDLK_g;
		g_keycodes[KEY_H] = SDLK_h;
		g_keycodes[KEY_I] = SDLK_i;
		g_keycodes[KEY_J] = SDLK_j;
		g_keycodes[KEY_K] = SDLK_k;
		g_keycodes[KEY_L] = SDLK_l;
		g_keycodes[KEY_M] = SDLK_m;
		g_keycodes[KEY_N] = SDLK_n;
		g_keycodes[KEY_O] = SDLK_o;
		g_keycodes[KEY_P] = SDLK_p;
		g_keycodes[KEY_Q] = SDLK_q;
		g_keycodes[KEY_R] = SDLK_r;
		g_keycodes[KEY_S] = SDLK_s;
		g_keycodes[KEY_T] = SDLK_t;
		g_keycodes[KEY_U] = SDLK_u;
		g_keycodes[KEY_V] = SDLK_v;
		g_keycodes[KEY_W] = SDLK_w;
		g_keycodes[KEY_X] = SDLK_x;
		g_keycodes[KEY_Y] = SDLK_y;
		g_keycodes[KEY_Z] = SDLK_z;

		g_keycodes[KEY_Grave] = SDLK_BACKQUOTE;

		g_keycodes[KEY_Minus] = SDLK_MINUS;
		g_keycodes[KEY_Equals] = SDLK_EQUALS;
		g_keycodes[KEY_Backslash] = SDLK_BACKSLASH;
		g_keycodes[KEY_LBracket] = SDLK_LEFTBRACKET;
		g_keycodes[KEY_RBracket] = SDLK_RIGHTBRACKET;
		g_keycodes[KEY_Semicolon] = SDLK_SEMICOLON;
		g_keycodes[KEY_Apostrophe] = SDLK_QUOTE;
		g_keycodes[KEY_Comma] = SDLK_COMMA;
		g_keycodes[KEY_Period] = SDLK_PERIOD;
		g_keycodes[KEY_Slash] = SDLK_SLASH;

		g_keycodes[KEY_Plus] = SDLK_PLUS;
		g_keycodes[KEY_Minus] = SDLK_MINUS;
		g_keycodes[KEY_Asterisk] = SDLK_ASTERISK;
		g_keycodes[KEY_Equals] = SDLK_EQUALS;

		g_keycodes[KEY_Numpad0] = SDLK_KP_0;
		g_keycodes[KEY_Numpad1] = SDLK_KP_1;
		g_keycodes[KEY_Numpad2] = SDLK_KP_2;
		g_keycodes[KEY_Numpad3] = SDLK_KP_3;
		g_keycodes[KEY_Numpad4] = SDLK_KP_4;
		g_keycodes[KEY_Numpad5] = SDLK_KP_5;
		g_keycodes[KEY_Numpad6] = SDLK_KP_6;
		g_keycodes[KEY_Numpad7] = SDLK_KP_7;
		g_keycodes[KEY_Numpad8] = SDLK_KP_8;
		g_keycodes[KEY_Numpad9] = SDLK_KP_9;

		//g_keycodes[SDLK_ASTERISK] = KEY_Multiply;
		//g_keycodes[SDLK_DIVIDE] = KEY_Divide;		
		//g_keycodes[SDLK_KP_PLUS] = ?;

		g_keycodes[KEY_F1] = SDLK_F1;
		g_keycodes[KEY_F2] = SDLK_F2;
		g_keycodes[KEY_F3] = SDLK_F3;
		g_keycodes[KEY_F4] = SDLK_F4;
		g_keycodes[KEY_F5] = SDLK_F5;
		g_keycodes[KEY_F6] = SDLK_F6;
		g_keycodes[KEY_F7] = SDLK_F7;
		g_keycodes[KEY_F8] = SDLK_F8;
		g_keycodes[KEY_F9] = SDLK_F9;
		g_keycodes[KEY_F10] = SDLK_F10;
		g_keycodes[KEY_F11] = SDLK_F11;
		g_keycodes[KEY_F12] = SDLK_F12;
	}
	ERet Initialize(const Settings& options)
	{
		InitKeycodeTable();
		mxZERO_OUT(g_keyboard);

		SDL_Init(SDL_INIT_EVERYTHING);

		g_lastFrameMicroseconds = mxGetTimeInMicroseconds();
		g_lastFrameDeltaSeconds = 0.0;

		SDL_DisplayMode desktop;
		if(SDL_GetDesktopDisplayMode(0, &desktop) < 0) {
			ptERROR("SDL_GetDesktopDisplayMode() failed: %s", SDL_GetError());
			return ERR_UNKNOWN_ERROR;
		}

		int windowFlags = 0;

		windowFlags |= SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
		//windowFlags |= SDL_WINDOW_OPENGL;

		if( options.resizable ) {
			windowFlags |= SDL_WINDOW_RESIZABLE;
		}

		int windowX = SDL_WINDOWPOS_UNDEFINED;
		int windowY = SDL_WINDOWPOS_UNDEFINED;
		int windowWidth = options.screenWidth;
		int windowHeight = options.screenHeight;

		windowX = SDL_WINDOWPOS_CENTERED;
		windowY = SDL_WINDOWPOS_CENTERED;

		g_window = SDL_CreateWindow(
			options.name,
			windowX, windowY, windowWidth, windowHeight,
			windowFlags
		);
		if(!g_window) {
			ptERROR("SDL_CreateWindow() failed: %s", SDL_GetError());
			return ERR_UNKNOWN_ERROR;
		}

		SDL_GetWindowSize( g_window, &g_screenWidth, &g_screenHeight );

		return ALL_OK;
	}
	void Shutdown()
	{
		SDL_Quit();
	}
	static UINT32 Convert_SDL_Key_Modifiers( UINT16 keyMod )
	{
		UINT32 modifiers = 0;
		if( keyMod & KMOD_LSHIFT ) {
			modifiers |= KeyModifier_LShift;
		}
		if( keyMod & KMOD_RSHIFT ) {
			modifiers |= KeyModifier_RShift;
		}
		if( keyMod & KMOD_LCTRL ) {
			modifiers |= KeyModifier_LCtrl;
		}
		if( keyMod & KMOD_RCTRL ) {
			modifiers |= KeyModifier_RCtrl;
		}
		if( keyMod & KMOD_LALT ) {
			modifiers |= KeyModifier_LAlt;
		}
		if( keyMod & KMOD_RALT ) {
			modifiers |= KeyModifier_RAlt;
		}
		if( keyMod & KMOD_LGUI ) {
			modifiers |= KeyModifier_LWin;
		}
		if( keyMod & KMOD_RGUI ) {
			modifiers |= KeyModifier_RWin;
		}
		if( keyMod & KMOD_NUM ) {
			modifiers |= KeyModifier_NumLock;
		}
		if( keyMod & KMOD_CAPS ) {
			modifiers |= KeyModifier_CapsLock;
		}
		if( keyMod & KMOD_MODE ) {
			//the AltGr key is down
		}
		return modifiers;
	}
	static UINT32 Get_SDL_Key_Modifiers( const Uint8* keyboardState )
	{
		UINT32 modifiers = 0;
		if( keyboardState[ KMOD_LSHIFT ] ) {
			modifiers |= KeyModifier_LShift;
		}
		if( keyboardState[ KMOD_RSHIFT ] ) {
			modifiers |= KeyModifier_RShift;
		}
		if( keyboardState[ KMOD_LCTRL ] ) {
			modifiers |= KeyModifier_LCtrl;
		}
		if( keyboardState[ KMOD_RCTRL ] ) {
			modifiers |= KeyModifier_RCtrl;
		}
		if( keyboardState[ KMOD_LALT ] ) {
			modifiers |= KeyModifier_LAlt;
		}
		if( keyboardState[ KMOD_RALT ] ) {
			modifiers |= KeyModifier_RAlt;
		}
		if( keyboardState[ KMOD_LGUI ] ) {
			modifiers |= KeyModifier_LWin;
		}
		if( keyboardState[ KMOD_RGUI ] ) {
			modifiers |= KeyModifier_RWin;
		}
		if( keyboardState[ KMOD_NUM ] ) {
			modifiers |= KeyModifier_NumLock;
		}
		if( keyboardState[ KMOD_CAPS ] ) {
			modifiers |= KeyModifier_CapsLock;
		}
		if( keyboardState[ KMOD_MODE ] ) {
			//the AltGr key is down
		}
		return modifiers;
	}
	static EMouseButton SDL_ConvertMouseButton( UINT8 button ) {
		switch(button)
		{
		case SDL_BUTTON_LEFT :		return EMouseButton::MouseButton_Left;
		case SDL_BUTTON_MIDDLE :	return EMouseButton::MouseButton_Middle;
		case SDL_BUTTON_RIGHT :		return EMouseButton::MouseButton_Right;
		case SDL_BUTTON_X1 :		return EMouseButton::MouseButton_4;
		case SDL_BUTTON_X2 :		return EMouseButton::MouseButton_5;
			mxNO_SWITCH_DEFAULT;
		}
		return EMouseButton::MouseButton_Left;
	}
	static UINT32 SDL_ConvertMouseButtonMask( Uint32 mouseState ) {
		UINT32 buttonMask = 0;
		if( mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) ) {
			buttonMask |= BIT(EMouseButton::MouseButton_Left);
		}
		if( mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE) ) {
			buttonMask |= BIT(EMouseButton::MouseButton_Right);
		}
		if( mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) ) {
			buttonMask |= BIT(EMouseButton::MouseButton_Middle);
		}
		if( mouseState & SDL_BUTTON(SDL_BUTTON_X1) ) {
			buttonMask |= BIT(EMouseButton::MouseButton_X1);
		}
		if( mouseState & SDL_BUTTON(SDL_BUTTON_X2) ) {
			buttonMask |= BIT(EMouseButton::MouseButton_X2);
		}
		return buttonMask;
	}

	void PostEvent( const SystemEvent& _event, UINT32 _states )
	{
#if USE_DEFERRED_EVENT_PROCESSING
		EventSystem::PostEvent( newEvent );
#else
		ProcessEvent( _event, _states );
#endif
	}

	void GenerateSystemEvents( UINT32 states )
	{
		SDL_PumpEvents();

		SDL_Event event;

		bool bMouseMoved = false;
		while( SDL_PollEvent( &event ) )
		{
			//newEvent.timestamp = event.common.timestamp;

			switch(event.type)
			{
			case SDL_QUIT:
				{
					SystemEvent newEvent;
					newEvent.type = SysEvent_Window;
					newEvent.window.type = WinEvent_Exiting;

					PostEvent( newEvent, states );
				}
				return;

			//case SDL_TEXTINPUT:
			//	{
			//		uchar buf[SDL_TEXTINPUTEVENT_TEXT_SIZE+1];
			//		size_t len = decodeutf8(buf, sizeof(buf)-1, (const uchar *)event.text.text, strlen(event.text.text));
			//		if(len > 0) { buf[len] = '\0'; processtextinput((const char *)buf, len); }
			//		break;
			//	}
			case SDL_TEXTINPUT:
				/* Add new text onto the end of our text */
				//strcat(text, event.text.text);
				{
					const SDL_TextInputEvent& textInput = event.text;
					//DBGOUT("Text: %s",textInput.text);
					HACK_ImGui_ProcessTextInput(textInput.text);
				}
				break;
			//case SDL_TEXTEDITING:
			//	/*
			//	Update the composition text.
			//	Update the cursor position.
			//	Update the selection length (if any).
			//	*/
			//	composition = event.edit.text;
			//	cursor = event.edit.start;
			//	selection_len = event.edit.length;
			//	break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if( bProcessKeyboardInput && g_bHasFocus )
				{
					const SDL_KeyboardEvent& keyboard = event.key;
					const bool isPressed = (keyboard.state == SDL_PRESSED);
					const EKeyCode key = GetKeyCode( keyboard.keysym );
					g_modifiers = Convert_SDL_Key_Modifiers(keyboard.keysym.mod);
					g_keyboard[key] = isPressed;

					SystemEvent newEvent;
					newEvent.type = SysEvent_Key;
					newEvent.button.key = key;
					newEvent.button.action = isPressed ? (keyboard.repeat ? IA_Repeated : IA_Pressed) : IA_Released;
					newEvent.button.modifiers = g_modifiers;

					PostEvent( newEvent, states );
				}
				break;

			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					RequestExit();
					break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
					DBG_MSG("SDL_WINDOWEVENT_FOCUS_GAINED");
					g_bHasFocus = true;
					break;
				case SDL_WINDOWEVENT_ENTER:
					DBG_MSG("SDL_WINDOWEVENT_ENTER");
					g_bHasFocus = true;
					break;

				case SDL_WINDOWEVENT_LEAVE:
					DBG_MSG("SDL_WINDOWEVENT_LEAVE");
				case SDL_WINDOWEVENT_FOCUS_LOST:
					DBG_MSG("SDL_WINDOWEVENT_FOCUS_LOST");
					g_bHasFocus = false;
					break;

				case SDL_WINDOWEVENT_MINIMIZED:
					DBG_MSG("SDL_WINDOWEVENT_MINIMIZED");
					g_bHasFocus = false;
					break;

				case SDL_WINDOWEVENT_MAXIMIZED:
					DBG_MSG("SDL_WINDOWEVENT_MAXIMIZED");
				case SDL_WINDOWEVENT_RESTORED:
					DBG_MSG("SDL_WINDOWEVENT_RESTORED");
					g_bHasFocus = false;
					break;

				case SDL_WINDOWEVENT_RESIZED:
					DBG_MSG("SDL_WINDOWEVENT_RESIZED");
					break;

				//case SDL_WINDOWEVENT_SIZE_CHANGED:
				//	SDL_GetWindowSize(screen, &g_screenWidth, &g_screenHeight);
				//	if(!(SDL_GetWindowFlags(screen) & SDL_WINDOW_FULLSCREEN))
				//	{
				//		scr_w = clamp(g_screenWidth, SCR_MINW, SCR_MAXW);
				//		scr_h = clamp(g_screenHeight, SCR_MINH, SCR_MAXH);
				//	}
				//	gl_resize();
				//	break;
				}
				break;

			case SDL_MOUSEMOTION:
				if( bProcessMouseMotion && g_bHasFocus )
				{
					SystemEvent newEvent;
					newEvent.type = SysEvent_MouseMove;
					newEvent.motion.x = event.motion.x;
					newEvent.motion.y = event.motion.y;
					newEvent.motion.buttonMask = SDL_ConvertMouseButtonMask(event.motion.state);
					newEvent.motion.modifiers = g_modifiers;
					newEvent.motion.deltaX = event.motion.xrel;
					newEvent.motion.deltaY = event.motion.yrel;

					bMouseMoved = (event.motion.xrel != 0) || (event.motion.yrel != 0);

					PostEvent( newEvent, states );
				}
				//else if(shouldgrab) inputgrab(grabinput = true);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if( bProcessMouseButtons && g_bHasFocus )
				{
					const bool isPressed = (event.button.state == SDL_PRESSED);

					SystemEvent newEvent;
					newEvent.type = SysEvent_Key;
					newEvent.button.action = isPressed ? IA_Pressed : IA_Released;
					newEvent.button.modifiers = g_modifiers;

					EKeyCode button = KEY_Unknown;

					switch( event.button.button )
					{
					case SDL_BUTTON_LEFT:
						button = KEY_MOUSE1;
						break;
					case SDL_BUTTON_MIDDLE:
						button = KEY_MOUSE3;
						break;
					case SDL_BUTTON_RIGHT:
						button = KEY_MOUSE2;
						break;
					case SDL_BUTTON_X1:
						button = KEY_MOUSE4;
						break;
					case SDL_BUTTON_X2:
						button = KEY_MOUSE5;
						break;
					}

					newEvent.button.key = button;

					g_keyboard[button] = isPressed;

					PostEvent( newEvent, states );
				}
				break;

			case SDL_MOUSEWHEEL:
				if( bProcessMouseButtons && g_bHasFocus )
				{
					const int value = event.wheel.y;
					const EKeyCode key = (value > 0) ? KEY_MWHEELUP : KEY_MWHEELDOWN;
					const int iterations = abs( value );
					for ( int i = 0; i < iterations; i++ )
					{
						SystemEvent newEvent;						newEvent.type = SysEvent_Key;						newEvent.button.key = key;						newEvent.button.modifiers = g_modifiers;
						newEvent.button.action = IA_Pressed;
						PostEvent( newEvent, states );

						newEvent.button.action = IA_Released;
						PostEvent( newEvent, states );
					}
				}
				break;
			}
		}

		if( g_bHasFocus ) {
			DoSetRelativeMouseMode(bRelativeMouseMode);
		} else {
			DoSetRelativeMouseMode(false);
		}
	}
	static void ProcessEvent( const SystemEvent& _event, UINT32 states )
	{
		bool processed = false;
		const ClientCallbacks::Data& input = ClientCallbacks::GetData();

		//// free any block data
		//if ( _event.evPtr ) {
		//	mxFree( _event.evPtr );
		//}
		
		switch(_event.type)
		{
		case SysEvent_Window:
			{
				switch(_event.window.type)
				{
				case WinEvent_Exiting:
					RequestExit();
					return;
				}
			}
			break;

		case SysEvent_Key:
			{
				const EInputState keyStatus =
					(_event.button.action == IA_Pressed) ? IS_Pressed
					: ((_event.button.action == IA_Released) ? IS_Released : IS_HeldDown);
					//(EInputState)_event.button.action;

				const UINT16 keyModifiers = _event.button.modifiers;

				for( int k = 0; k < input.contexts.Num(); k++ )
				{
					const InputContext& inputContext = input.contexts[k];

					if( !(inputContext.state_mask.m_value & states) ) {
						continue;
					}

					for( int i = 0; i < inputContext.key_binds.Num(); i++ )
					{
						const KeyBind& keyBind = inputContext.key_binds[i];

						if( keyBind.key == _event.button.key
							&& (keyBind.when & BIT(keyStatus) )
							&& (keyBind.with ? ((keyBind.with & keyModifiers) == keyBind.with) : (keyModifiers==0))
							)
						{
							for( int t = 0; t < input.handlers.Num(); t++ )
							{
								const ActionBindingT& binding = input.handlers[t];
								if( binding.key == keyBind.action )
								{
									(binding.value)( keyBind.action, keyStatus, 0.0f );
									processed = true;
								}
							}
						}
					}
				}
			}
			break;

		case SysEvent_MouseMove:
			{
				for( int k = 0; k < input.contexts.Num(); k++ )
				{
					const InputContext& inputContext = input.contexts[k];

					if( !(inputContext.state_mask.m_value & states) ) {
						continue;
					}

					for( int i = 0; i < inputContext.axis_binds.Num(); i++ )
					{
						const AxisBind& axis_bind = inputContext.axis_binds[i];

						if( _event.motion.deltaX != 0 && axis_bind.axis == MouseAxisX )
						{
							for( int t = 0; t < input.handlers.Num(); t++ )
							{
								const ActionBindingT& binding = input.handlers[t];
								if( binding.key == axis_bind.action )
								{
									(binding.value)( axis_bind.action, IS_Changed, _event.motion.deltaX );
									processed = true;
								}
							}
						}
						if( _event.motion.deltaY != 0 && axis_bind.axis == MouseAxisY )
						{
							for( int t = 0; t < input.handlers.Num(); t++ )
							{
								const ActionBindingT& binding = input.handlers[t];
								if( binding.key == axis_bind.action )
								{
									(binding.value)( axis_bind.action, IS_Changed, _event.motion.deltaY );
									processed = true;
								}
							}
						}
					}
				}
			}
			break;
		}

		if( !processed ) {
			//DBG_MSG("Event was not be processed, mask: %d", states);
		}
	}
	int ProcessSystemEvents( UINT32 states )
	{
#if USE_DEFERRED_EVENT_PROCESSING
		for(;;)
		{
			// Update the timing information
			// Use SDL_GetTicks(); ?
			const Uint64 ticksThen = g_lastFrameMicroseconds;
			const Uint64 ticksNow = mxGetTimeInMicroseconds();
			const Uint64 ticksElapsed = ticksNow - ticksThen;
			const double secondsElapsed = (double)ticksElapsed * 1e-6;

			g_lastFrameMicroseconds = ticksNow;
			g_lastFrameDeltaSeconds = secondsElapsed;

			SystemEvent	event = EventSystem::GetEvent();
			// if no more events are available
			if ( event.type == SysEvent_None ) {
				return 0;
			}
			ProcessEvent( event, states );
		}
#endif
		return 0;	// never reached
	}
	void* GetNativeWindowHandle()
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		if(SDL_GetWindowWMInfo(g_window, &wmInfo)!=SDL_TRUE) {
			return NULL;
		}
		return wmInfo.info.win.window;
	}
	ERet SetRelativeMouseMode(bool enable)
	{
		DBG_MSG("SetRelativeMouseMode(%d)",enable);
		bRelativeMouseMode = enable;
		return ALL_OK;
	}
	static ERet DoSetRelativeMouseMode(bool enable)
	{
		// This function will flush any pending mouse motion.
		if( SDL_SetRelativeMouseMode( enable ? SDL_TRUE : SDL_FALSE ) != 0 ) {
			ptERROR("SDL_SetRelativeMouseMode() failed: %s", SDL_GetError());
			return ERR_UNKNOWN_ERROR;
		}
		return ALL_OK;
	}
	bool IsRelativeMouseModeEnabled()
	{
		return bRelativeMouseMode;
	}
	bool HasFocus()
	{
		return g_bHasFocus;
	}
	void RequestExit()
	{
		DBG_MSG("WindowsDriver::RequestExit()\n");
		g_bRequestedExit = true;
	}
	bool IsAboutToExit()
	{
		return g_bRequestedExit;
	}
	void GetWindowSize( int *x, int *y )
	{
		SDL_GetWindowSize(g_window, x, y);
	}
	void GetWindowPosition( int *x, int *y )
	{
		SDL_GetWindowPosition(g_window, x, y);
	}
	const UINT8* GetKeyboardState()
	{
		return g_keyboard;
	}
	const UINT16 GetModifiers()
	{
		return g_modifiers;
	}
	int GetMouseState( int *x, int *y )
	{
		Uint32 state = SDL_GetMouseState( x, y );
		return SDL_ConvertMouseButtonMask( state );
	}
	double GetDeltaSeconds()
	{
		return g_lastFrameDeltaSeconds;
	}

}//namespace WindowsDriver

#endif // #if defined(TB_USE_SDL)

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
