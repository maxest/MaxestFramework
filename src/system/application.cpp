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
	mouseCursorVisible = true;
	mouseRelativeMode = false;
	mouseLeftButtonDown = false;
	mouseMiddleButtonDown = false;
	mouseRightButtonDown = false;
	mouseX = 0;
	mouseY = 0;
	mouseRelX = 0;
	mouseRelY = 0;

	runStartTime = runStopTime = 0;
	lastFrameTime = 0.0f;

	keyDownFunction = nullptr;
	mouseMotionFunction = nullptr;
	mouseLeftButtonDownFunction = nullptr;
	mouseRightButtonDownFunction = nullptr;
}


bool NSystem::CApplication::Create(int width, int height, bool fullScreen)
{
	this->fullScreen = fullScreen;

#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	displayWidth = displayMode.w;
	displayHeight = displayMode.h;

	if (width == 0 && height == 0)
	{
		width = displayWidth;
		height = displayHeight;
	}

	Uint32 windowCreationFlags = 0;
	if (fullScreen)
		windowCreationFlags |= SDL_WINDOW_FULLSCREEN;
	#ifdef MAXEST_FRAMEWORK_OPENGL
		windowCreationFlags |= SDL_WINDOW_OPENGL;
	#endif

	int x = displayWidth/2 - width/2;
	int y = displayHeight/2 - height/2;

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
		mouseRelX = 0;
		mouseRelY = 0;

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
				int prevMouseX = mouseX;
				int prevMouseY = mouseY;

				mouseX = event.motion.x;
				mouseY = event.motion.y;

				if (!mouseRelativeMode)
				{
					mouseRelX = mouseX - prevMouseX;
					mouseRelY = mouseY - prevMouseY;
				}

				if (mouseMotionFunction)
					mouseMotionFunction(event.motion.x, event.motion.y);
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
			if (mouseRelativeMode)
			{
				const int refX = displayWidth / 2;
				const int refY = displayHeight / 2;

				SDL_GetGlobalMouseState(&mouseX, &mouseY);

				mouseRelX = mouseX - refX;
				mouseRelY = mouseY - refY;

				SDL_WarpMouseGlobal(refX, refY);
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


void NSystem::CApplication::MouseShowCursor(bool show)
{
	mouseCursorVisible = show;
#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_ShowCursor(show);
#endif
}


void NSystem::CApplication::MouseSetRelativeMode(bool state)
{
	mouseRelativeMode = state;
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


int NSystem::CApplication::MouseX()
{
	return mouseX;
}


int NSystem::CApplication::MouseY()
{
	return mouseY;
}


int NSystem::CApplication::MouseRelX()
{
	return mouseRelX;
}


int NSystem::CApplication::MouseRelY()
{
	return mouseRelY;
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
