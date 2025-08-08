#include <memory>
#include <array>
#include <SDL3/SDL.h>

#include "Memory.h"

class LCD
{
public:
	LCD(std::shared_ptr<Memory> mem);
	~LCD();

	/* Set window and create SDL_Surface used for rendering.
	 *  @param window Window in which to render the Gameboy's LCD.
	 */
	void SetWindow(SDL_Window *window);

	/* Check if the LCD is ready for rendering
	 * (LCD might not be ready because it is missing the window).
	 * @return True if the LCD can run correctly.
	 */
	bool IsReady();

	/* Set sprites during OAM scan for rendering next scanline.
	 *  @param sprites Array with the 10 selected sprites.
	 */
	void SetSprites(std::array<int, 10> &sprites);

	/* Draw scanline to internal surface.
	 *  @param LY Line to render (present at memory address $FF44).
	 */
	void DrawScanline(int LY);

	/* Set the internal surface's color to the lightest color.
	 */
	void DisableLCD();

	/* Blit the internal surface to the window's surface and update it.
	 */
	void Render();

	/* Show tile banks 0-2 on screen (write them to the internal surface).
	 */
	void ShowTiles();

	bool bgEnabled;

private:
	std::shared_ptr<Memory> m_Mem;

	SDL_Window *m_Window;
	SDL_Surface *m_Surface;
	SDL_Surface *m_Screen;
	SDL_Rect m_BlitRect;

	unsigned int m_Palette[4];
	std::array<int, 10> m_Sprites;
	std::array<bool, 160> m_SpritePriorityMask;

	bool m_Ready;
};