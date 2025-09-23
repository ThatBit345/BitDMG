#pragma once
#include <memory>

#include "Memory.h"
#include "LCD.h"

class PPU
{
public:
	PPU(std::shared_ptr<Memory> memory);

	/* Tick PPU by the CPU cycle count (keeping them synced).
	 * @param cycles CPU T-Cycles taken during last operation.
	 */
	void Tick(int cycles);

	/* Print tiles to console using characters: " , ░, ▓, █".
	 */
	void PrintTiles();

	/* Render to window.
	 */
	void Render();

	/* Set window for rendering.
	 *  @param window Window in which to render.
	 */
	void ConfigureLCD(SDL_Window *window);

	/* Set current color palette on the LCD.
	 * @param id ID of the palette.
	 */
	void SetLCDPalette(int id);

private:
	std::shared_ptr<Memory> m_Mem;

	LCD m_LCD;

	int m_Clock;
	int m_Mode;

	bool m_STAT;
	bool m_LYCSTAT;
	bool m_Mode2STAT;
	bool m_Mode1STAT;
	bool m_Mode0STAT;

	/* Increment LY register and handle STAT flags.
	 */
	void IncrementLY();

	/* Handle STAT register's interrupts.
	 */
	void HandleSTAT();
};
