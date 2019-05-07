#include "application.h"
#include "common.h"
#include "../essentials/common.h"
#include "../essentials/macros.h"

#ifdef MAXEST_FRAMEWORK_OPENGL
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		#include "../../dependencies/glew-2.0.0/include/GL/glew.h"
	#else
		#include "../../dependencies/glew-2.1.0/include/GL/glew.h"
	#endif
#endif


using namespace NMaxestFramework;
using namespace NEssentials;


NSystem::CApplication::CApplication()
{
	for (size_t i = 0; i < ARRAY_SIZE(keysPressed); i++)
	{
		keysPressed[i] = false;
		keysDown[i] = false;
		keysUp[i] = false;
	}
	mouseWrapping = false;
	mouseLeftButtonDown = false;
	mouseMiddleButtonDown = false;
	mouseRightButtonDown = false;
	mouseWindowX = 0;
	mouseWindowY = 0;
	mouseDesktopX = 0;
	mouseDesktopY = 0;
	mouseRelX = 0;
	mouseRelY = 0;
	cursorVisible = true;

	runStartTime = runStopTime = 0;
	lastFrameTime = 0.0f;

	keyDownFunction = nullptr;
	mouseMotionFunction = nullptr;
	mouseLeftButtonDownFunction = nullptr;
	mouseRightButtonDownFunction = nullptr;
}


bool NSystem::CApplication::Create(int width, int height, bool fullScreen, int displayIndex)
{
	this->fullScreen = fullScreen;

#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	int screenWidth = displayMode.w;
	int screenHeight = displayMode.h;

	if (width == 0 && height == 0)
	{
		width = screenWidth;
		height = screenHeight;
	}

	Uint32 windowCreationFlags = 0;
	if (fullScreen)
		windowCreationFlags |= SDL_WINDOW_FULLSCREEN;
	#ifdef MAXEST_FRAMEWORK_OPENGL
		windowCreationFlags |= SDL_WINDOW_OPENGL;
	#endif

	int x = screenWidth/2 - width/2;
	int y = screenHeight/2 - height/2;
	x += displayIndex * screenWidth;

	window = SDL_CreateWindow(
		"MaxestFrameworkWindow",
		x,
		y,
		width,
		height,
		windowCreationFlags
	);

	#ifdef MAXEST_FRAMEWORK_OPENGL
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

		glContext = SDL_GL_CreateContext(window);

		SDL_GL_SetSwapInterval(0);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
			return false;
	#endif
#endif

	return true;
}


void NSystem::CApplication::Destroy()
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	#ifdef MAXEST_FRAMEWORK_OPENGL
		SDL_GL_DeleteContext(glContext);
	#endif

	SDL_DestroyWindow(window);
#endif
}


void NSystem::CApplication::Run(bool(*runFunction)())
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	int prevMouseX;
	int prevMouseY;

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
#endif

	while (true)
	{
		runStartTime = TickCount();

		for (size_t i = 0; i < ARRAY_SIZE(keysPressed); i++)
		{
			keysDown[i] = false;
			keysUp[i] = false;
		}

	#ifdef MAXEST_FRAMEWORK_DESKTOP
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (keyDownFunction != NULL)
					keyDownFunction((EKey)event.key.keysym.scancode);

				keysPressed[event.key.keysym.scancode] = true;
				keysDown[event.key.keysym.scancode] = true;
			}
			else if (event.type == SDL_KEYUP)
			{
				keysPressed[event.key.keysym.scancode] = false;
				keysUp[event.key.keysym.scancode] = true;
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouseWindowX = event.motion.x;
				mouseWindowY = event.motion.y;

				if (mouseMotionFunction)
					mouseMotionFunction(event.button.x, event.button.y);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseLeftButtonDown = true;
					if (mouseLeftButtonDownFunction)
						mouseLeftButtonDownFunction(event.button.x, event.button.y);
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouseMiddleButtonDown = true;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					mouseRightButtonDown = true;
					if (mouseRightButtonDownFunction)
						mouseRightButtonDownFunction(event.button.x, event.button.y);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseLeftButtonDown = false;
				}
				else if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouseMiddleButtonDown = false;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					mouseRightButtonDown = false;
				}
			}
		}

		if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
		{
			prevMouseX = mouseDesktopX;
			prevMouseY = mouseDesktopY;

			SDL_GetGlobalMouseState(&mouseDesktopX, &mouseDesktopY);

			mouseRelX = mouseDesktopX - prevMouseX;
			mouseRelY = mouseDesktopY - prevMouseY;

			if (mouseWrapping)
			{
				int minX = 0, maxX = displayMode.w, minY = 0, maxY = displayMode.h;
				for (int i = 0; i < SDL_GetNumVideoDisplays(); i++)
				{
					SDL_Rect rect;
					SDL_GetDisplayBounds(i, &rect);

					if (i == 0)
					{
						minX = rect.x;
						maxX = rect.x + rect.w;
						minY = rect.y;
						maxY = rect.y + rect.h;
					}
					else
					{
						minX = Min(minX, rect.x);
						maxX = Max(maxX, rect.x + rect.w);
						minY = Min(minY, rect.y);
						maxY = Max(maxY, rect.y + rect.h);
					}
				}
				maxX--;
				maxY--;

				if (mouseDesktopX <= minX)
					mouseDesktopX = maxX - 1;
				if (mouseDesktopX >= maxX)
					mouseDesktopX = minX + 1;

				if (mouseDesktopY <= minY)
					mouseDesktopY = maxY - 1;
				if (mouseDesktopY >= maxY)
					mouseDesktopY = minY + 1;

				SDL_WarpMouseGlobal(mouseDesktopX, mouseDesktopY);
			}
		}
	#endif

		if (runFunction)
		{
			if (!runFunction())
				return;
		}

	#ifdef MAXEST_FRAMEWORK_DESKTOP
		#ifdef MAXEST_FRAMEWORK_OPENGL
			SDL_GL_SwapWindow(window);
		#endif
	#endif

		runStopTime = TickCount();
		lastFrameTime = (float)(runStopTime - runStartTime) / 1000.0f;
	}
}


void NSystem::CApplication::SetWindowTitle(const char* text)
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_SetWindowTitle(window, text);
#endif
}


bool NSystem::CApplication::IsKeyPressed(EKey key)
{
	return keysPressed[(int)key];
}


bool NSystem::CApplication::IsKeyDown(EKey key)
{
	return keysDown[(int)key];
}


bool NSystem::CApplication::IsKeyUp(EKey key)
{
	return keysUp[(int)key];
}


void NSystem::CApplication::SetMouseWrapping(bool state)
{
	mouseWrapping = state;
}


bool NSystem::CApplication::MouseLeftButtonDown()
{
	return mouseLeftButtonDown;
}


bool NSystem::CApplication::MouseMiddleButtonDown()
{
	return mouseMiddleButtonDown;
}


bool NSystem::CApplication::MouseRightButtonDown()
{
	return mouseRightButtonDown;
}


int NSystem::CApplication::MouseWindowX()
{
	return mouseWindowX;
}


int NSystem::CApplication::MouseWindowY()
{
	return mouseWindowY;
}


int NSystem::CApplication::MouseDesktopX()
{
	return mouseDesktopX;
}


int NSystem::CApplication::MouseDesktopY()
{
	return mouseDesktopY;
}


int NSystem::CApplication::MouseRelX()
{
	return mouseRelX;
}


int NSystem::CApplication::MouseRelY()
{
	return mouseRelY;
}


void NSystem::CApplication::ShowCursor(bool show)
{
	cursorVisible = show;
#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_ShowCursor(show);
#endif
}


float NSystem::CApplication::LastFrameTime()
{
	return lastFrameTime;
}


void NSystem::CApplication::SetKeyDownFunction(void (*keyDownFunction)(EKey key))
{
	this->keyDownFunction = keyDownFunction;
}


void NSystem::CApplication::SetMouseMotionFunction(void (*mouseMotionFunction)(int x, int y))
{
	this->mouseMotionFunction = mouseMotionFunction;
}


void NSystem::CApplication::SetMouseLeftButtonDownFunction(void (*mouseLeftButtonDownFunction)(int x, int y))
{
	this->mouseLeftButtonDownFunction = mouseLeftButtonDownFunction;
}


void NSystem::CApplication::SetMouseRightButtonDownFunction(void (*mouseRightButtonDownFunction)(int x, int y))
{
	this->mouseRightButtonDownFunction = mouseRightButtonDownFunction;
}
