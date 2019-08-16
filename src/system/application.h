#pragma once


#include "keys.h"
#include "../essentials/types.h"


#ifdef MAXEST_FRAMEWORK_DESKTOP
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		#include "../../dependencies/SDL2-2.0.5/include/SDL.h"
	#else
		#include "../../dependencies/SDL2-2.0/include/SDL.h"
	#endif
#endif


#undef main // because of SDL


namespace NMaxestFramework { namespace NSystem
{
	class CApplication
	{
	public:
		CApplication();

		bool Create(int width, int height, bool fullScreen);
		void Destroy();

		void Run(bool(*runFunction)());

		void SetWindowTitle(const char* text);

		bool IsKeyPressed(EKey key);
		bool IsKeyDown(EKey key);
		bool IsKeyUp(EKey key);
		void MouseShowCursor(bool show);
		void MouseSetRelativeMode(bool state);
		bool MouseLeftButtonDown();
		bool MouseMiddleButtonDown();
		bool MouseRightButtonDown();
		int MouseX();
		int MouseY();
		int MouseRelX();
		int MouseRelY();

		float LastFrameTime();

		void SetKeyDownFunction(void (*keyDownFunction)(EKey key));
		void SetMouseMotionFunction(void (*mouseLeftButtonDownFunction)(int x, int y));
		void SetMouseLeftButtonDownFunction(void (*mouseLeftButtonDownFunction)(int x, int y));
		void SetMouseRightButtonDownFunction(void (*mouseRightButtonDownFunction)(int x, int y));

	private:
		bool fullScreen;
		int displayWidth, displayHeight;

		bool keysPressed[512];
		bool keysDown[512];
		bool keysUp[512];
		bool mouseCursorVisible;
		bool mouseRelativeMode;
		bool mouseLeftButtonDown;
		bool mouseMiddleButtonDown;
		bool mouseRightButtonDown;
		int mouseX;
		int mouseY;
		int mouseRelX;
		int mouseRelY;

		uint64 runStartTime, runStopTime;
		float lastFrameTime;

	#ifdef MAXEST_FRAMEWORK_DESKTOP
		SDL_Window* window;
		SDL_GLContext glContext;
	#endif

		void (*keyDownFunction)(EKey key);
		void (*mouseMotionFunction)(int x, int y);
		void (*mouseLeftButtonDownFunction)(int x, int y);
		void (*mouseRightButtonDownFunction)(int x, int y);
	};
} }
