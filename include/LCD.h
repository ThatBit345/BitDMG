#include <memory>
#include <SDL3/SDL.h>

#include "Memory.h"

class LCD
{
public:
	LCD(std::shared_ptr<Memory> mem);

	void SetWindow(SDL_Window* window);
	bool IsReady();

	void DrawScanline(int LY);
	void DisableLCD();
	void Render();

	void ShowTiles();

	bool bgEnabled;

private:
	std::shared_ptr<Memory> m_Mem;

	SDL_Window* m_Window;
	SDL_Surface* m_Surface;
	SDL_Surface* m_Screen;

	unsigned int m_Palette[4];

	bool m_Ready;
};