#include "LCD.h"

#include "Memory.h"
#include "Log.h"
#include <iostream>
#include <cmath>

LCD::LCD(std::shared_ptr<Memory> mem) : m_Ready(false), m_Mem(mem)
{
	m_Palette[0] = 0x9bbc0f; // Lighter
	m_Palette[1] = 0x8bac0f;
	m_Palette[2] = 0x306230;
	m_Palette[3] = 0x0f380f; // Darker
}

void LCD::SetWindow(SDL_Window *window)
{
	m_Window = window;
	m_Screen = SDL_GetWindowSurface(window);
	m_Surface = SDL_CreateSurface(160, 144, m_Screen->format);

	m_Ready = true;
}

bool LCD::IsReady()
{
	return m_Ready;
}

void LCD::DrawScanline(int LY)
{
	unsigned char LCDC = m_Mem->ReadU8(0xFF40);
	//if(!bgEnabled) return;

	int x = 0;

	for (unsigned char i = 0; i < 32; i++)
	{
		int verticalTile = std::floor(LY / 8.0f);
		int tilemapAddress = 0x9800;
		if((LCDC & 0b00001000) != 0) tilemapAddress = 0x9C00;

		unsigned char tileId = m_Mem->ReadU8Unfiltered((tilemapAddress + i) + (32 * verticalTile));

		int verticalLine = LY % 8;

		unsigned char lsb;
		unsigned char msb;

		// Take into account tile indexing modes
		if((LCDC & 0b00010000) == 0)
		{
			if(tileId < 128)
			{
				lsb = m_Mem->ReadU8Unfiltered(0x9000 + (verticalLine * 2) + tileId * 16);
				msb = m_Mem->ReadU8Unfiltered(0x9000 + (verticalLine * 2) + 1 + tileId * 16);
			}
			else
			{
				lsb = m_Mem->ReadU8Unfiltered(0x8800 + (verticalLine * 2) + (tileId - 128) * 16);
				msb = m_Mem->ReadU8Unfiltered(0x8800 + (verticalLine * 2) + 1 + (tileId - 128) * 16);
			}
		}
		else
		{
			lsb = m_Mem->ReadU8Unfiltered(0x8000 + (verticalLine * 2) + tileId * 16);
			msb = m_Mem->ReadU8Unfiltered(0x8000 + (verticalLine * 2) + 1 + tileId * 16);
		}

		for (int j = 7; j >= 0; j--)
		{
			unsigned char color = 0;
			color = (((msb >> j) & 0b1) << 1) | ((lsb >> j) & 0b1);

			int colorIndex = m_Mem->ReadU8(0xFF47);
			int paletteColor = m_Palette[(colorIndex >> (color * 2)) & 0b11];

			int r = (paletteColor & 0xFF0000) >> 16;
			int g = (paletteColor & 0x00FF00) >> 8;
			int b = (paletteColor & 0x0000FF);

			SDL_WriteSurfacePixel(m_Surface, x, LY, r, g, b, 0xFF);
			x++;
		}
	}
}

void LCD::DisableLCD()
{
	SDL_FillSurfaceRect(m_Surface, NULL, 0xCADC9F);
}

void LCD::Render()
{
	if(!m_Ready) return;

	//ShowTiles();
	SDL_BlitSurfaceScaled(m_Surface, nullptr, m_Screen, nullptr, SDL_SCALEMODE_NEAREST);
	SDL_UpdateWindowSurface(m_Window);
}

void LCD::ShowTiles()
{
	int x = 0;
	int y = 0;

	for (int i = 0; i < 0x17FF; i++)
	{
		unsigned char lsb = m_Mem->ReadU8Unfiltered(0x8000 + (i * 2));
		unsigned char msb = m_Mem->ReadU8Unfiltered(0x8000 + 1 + (i * 2));

		for (int j = 7; j >= 0; j--)
		{
			unsigned char color = 0;
			color = (((msb >> j) & 0b1) << 1) | ((lsb >> j) & 0b1);

			unsigned char colorIndex = m_Mem->ReadU8(0xFF47);
			int paletteColor = m_Palette[(colorIndex >> (color * 2)) & 0b11];

			int r = (paletteColor & 0xFF0000) >> 16;
			int g = (paletteColor & 0x00FF00) >> 8;
			int b = (paletteColor & 0x0000FF);

			SDL_WriteSurfacePixel(m_Surface, x, y, r, g, b, 0xFF);
			x++;
		}
		
		x -= 8;
		y++;
		
		if(y % 8 == 0)
		{
			if(x == 160)
			{
				// Next row
				x = 0;
			}
			else
			{
				// Next tile horizontally
				x += 8;
				y -= 8;
			}
		}
	}
}
