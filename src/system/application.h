#pragma once


#include "keys.h"
#include "../essentials/types.h"

#include "../../dependencies/SDL2-2.0.5/include/SDL.h"


#undef main // because of SDL


namespace NMaxestFramework { namespace NSystem
{
	class CApplication
	{
	public:
		CApplication();

		bool Create(int width, int height, bool fullScreen, int displayIndex = 0);
		void Destroy();

		void Run(bool(*runFunction)());

		void SetWindowTitle(const char* text);

		bool IsKeyPressed(EKey key);
		bool IsKeyDown(EKey key);
		bool IsKeyUp(EKey key);
		void SetMouseWrapping(bool state);
		bool MouseLeftButtonDown();
		bool MouseMiddleButtonDown();
		bool MouseRightButtonDown();
		int MouseWindowX();
		int MouseWindowY();
		int MouseDesktopX();
		int MouseDesktopY();
		int MouseRelX();
		int MouseRelY();
		void ShowCursor(bool show);

		float LastFrameTime();

		void SetKeyDownFunction(void (*keyDownFunction)(EKey key));
		void SetMouseMotionFunction(void (*mouseLeftButtonDownFunction)(int x, int y));
		void SetMouseLeftButtonDownFunction(void (*mouseLeftButtonDownFunction)(int x, int y));
		void SetMouseRightButtonDownFunction(void (*mouseRightButtonDownFunction)(int x, int y));

	private:
		bool fullScreen;

		bool keysPressed[512];
		bool keysDown[512];
		bool keysUp[512];
		bool mouseWrapping;
		bool mouseLeftButtonDown;
		bool mouseMiddleButtonDown;
		bool mouseRightButtonDown;
		int mouseWindowX;
		int mouseWindowY;
		int mouseDesktopX;
		int mouseDesktopY;
		int mouseRelX;
		int mouseRelY;
		bool cursorVisible;

		uint64 runStartTime, runStopTime;
		float lastFrameTime;

		SDL_Window* window;
		SDL_GLContext glContext;

		void (*keyDownFunction)(EKey key);
		void (*mouseMotionFunction)(int x, int y);
		void (*mouseLeftButtonDownFunction)(int x, int y);
		void (*mouseRightButtonDownFunction)(int x, int y);
	};
} }
