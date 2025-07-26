#include "LCD.h"

#include <iostream>
#include <cmath>

#include "Memory.h"
#include "Log.h"
#include "Utils.h"

LCD::LCD(std::shared_ptr<Memory> mem) : m_Ready(false), m_Mem(mem), m_Surface(nullptr)
{
	m_Palette[0] = 0xBADA55; // Lighter color
	m_Palette[1] = 0x82993B;
	m_Palette[2] = 0x4A5722;
	m_Palette[3] = 0x121608; // Darker color

	m_SpritePriorityMask.fill(false);
}

LCD::~LCD()
{
	SDL_DestroySurface(m_Surface);
	SDL_DestroySurface(m_Screen);
}

/* Set window and create SDL_Surface used for rendering.
 *  [window] -> Window in which to render the Gameboy's LCD
 */
void LCD::SetWindow(SDL_Window *window)
{
	m_Window = window;
	m_Screen = SDL_GetWindowSurface(window);

	if (m_Surface == nullptr)
	{
		m_Surface = SDL_CreateSurface(160, 144, m_Screen->format);
	}

	int widthRatio = std::floor(m_Screen->w / 160.0f);
	int heightRatio = std::floor(m_Screen->h / 144.0f);
	int scaleRatio = std::min(widthRatio, heightRatio);

	int x = (m_Screen->w / 2.0f) - ((160 * scaleRatio) / 2.0f);

	m_BlitRect = {x, 0, 160 * scaleRatio, 144 * scaleRatio};

	m_Ready = true;
}

/* Returns if the LCD is ready for rendering.
 *  (LCD might not be ready because it is missing the window)
 */
bool LCD::IsReady()
{
	return m_Ready;
}

/* Set sprites during OAM scan for rendering next scanline.
 *  [sprites] -> Array with the 10 selected sprites
 */
void LCD::SetSprites(std::array<int, 10> &sprites)
{
	m_Sprites = sprites;
}

/* Draw scanline to internal surface.
 *  TO-DO: 8x16 Sprites
 *  [LY] -> Line to render (present at memory address $FF44)
 */
void LCD::DrawScanline(int LY)
{
	unsigned char LCDC = m_Mem->ReadU8(0xFF40);
	unsigned char SCY = m_Mem->ReadU8(0xFF42);
	unsigned char SCX = m_Mem->ReadU8(0xFF43);

	int x = -(SCX % 8);

	m_SpritePriorityMask.fill(false);

	// Draw BG
	for (int i = 0; i < 21; i++)
	{
		int scrolledY = (SCY + LY) % 256;
		// int scrolledY = LY;
		int tileY = std::floor(scrolledY / 8.0f);
		int verticalLine = (scrolledY % 8);

		int scrolledX = (SCX + (i * 8)) % 256;

		// int scrolledX = i * 8;
		int tileX = std::floor(scrolledX / 8.0f);

		int tilemapAddress = GetBit(LCDC, 3) ? 0x9C00 : 0x9800;
		unsigned char tileId = m_Mem->ReadU8Unfiltered((tilemapAddress + tileX) + (32 * tileY));

		unsigned char lsb;
		unsigned char msb;

		// Take into account tile indexing modes
		if (GetBit(LCDC, 4) == false)
		{
			if (tileId < 128)
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

			if (x >= 0 && x < 160 && colorIndex == 0)
				m_SpritePriorityMask[x] = true;

			int r = (paletteColor & 0xFF0000) >> 16;
			int g = (paletteColor & 0x00FF00) >> 8;
			int b = (paletteColor & 0x0000FF);

			SDL_WriteSurfacePixel(m_Surface, x, LY, r, g, b, 0xFF);
			x++;
		}
	}

	// Draw window
	unsigned char WY = m_Mem->ReadU8(0xFF4A);
	if (LY >= WY && GetBit(LCDC, 5))
	{
		unsigned char WX = m_Mem->ReadU8(0xFF4B);
		// When WX is 166, the window spans the entire scanline
		x = (WX == 166) ? 0 : WX - 7;

		for (int i = 0; i < 22; i++)
		{
			int screenY = LY - WY;
			int tileY = std::floor(screenY / 8.0f);
			int verticalLine = (screenY % 8);

			int tileX = std::floor(x / 8.0f);
			if (WX < 7) tileX++;

			int tilemapAddress = GetBit(LCDC, 6) ? 0x9C00 : 0x9800;
			unsigned char tileId = m_Mem->ReadU8Unfiltered((tilemapAddress + tileX) + (32 * tileY));

			unsigned char lsb;
			unsigned char msb;

			// Take into account tile indexing modes
			if (GetBit(LCDC, 4) == false)
			{
				if (tileId < 128)
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

				if (x >= 0 && x < 160 && colorIndex == 0)
					m_SpritePriorityMask[x] = true;

				int r = (paletteColor & 0xFF0000) >> 16;
				int g = (paletteColor & 0x00FF00) >> 8;
				int b = (paletteColor & 0x0000FF);

				SDL_WriteSurfacePixel(m_Surface, x, LY, r, g, b, 0xFF);

				x++;
			}
		}
	}

	// Draw sprites, no sprites will be present in the array if they have been disabled (LCDC byte 1)
	for (int i = 9; i >= 0; i--)
	{
		if (m_Sprites[i] == -1)
			continue;

		int baseAddress = m_Sprites[i];

		// Remove offsets applied to positions in memory
		int y = m_Mem->ReadU8Unfiltered(baseAddress) - 16;
		int x = m_Mem->ReadU8Unfiltered(baseAddress + 1) - 8;

		unsigned char tileIndex = m_Mem->ReadU8Unfiltered(baseAddress + 2);
		unsigned char flags = m_Mem->ReadU8Unfiltered(baseAddress + 3);

		// Sprites are 8x16 instead of 8x8
		if (GetBit(LCDC, 2) && (LY - y) >= 8)
		{
			tileIndex++;
		}

		bool yFlip = GetBit(flags, 6);
		bool xFlip = GetBit(flags, 5);
		bool priority = GetBit(flags, 7);

		int verticalLine = yFlip ? std::abs(((LY - y) % 8) - 7) : (LY - y) % 8;
		unsigned char lsb = m_Mem->ReadU8Unfiltered(0x8000 + (verticalLine * 2) + tileIndex * 16);
		unsigned char msb = m_Mem->ReadU8Unfiltered(0x8000 + (verticalLine * 2) + 1 + tileIndex * 16);

		int paletteBank = GetBit(flags, 4) ? 0xFF49 : 0xFF48;

		int pixelIteration = xFlip ? 0 : 7;
		do
		{
			if (x >= 160 || x < 0)
				break;

			unsigned char color = 0;
			color = (GetBit(msb, pixelIteration) << 1) | GetBit(lsb, pixelIteration);

			bool prioritySkip = priority && m_SpritePriorityMask[x];
			// Color 0 is used for transparency, ignore it
			if (color > 0 && !prioritySkip)
			{
				int colorIndex = m_Mem->ReadU8(paletteBank);
				int paletteColor = m_Palette[(colorIndex >> (color * 2)) & 0b11];

				int r = (paletteColor & 0xFF0000) >> 16;
				int g = (paletteColor & 0x00FF00) >> 8;
				int b = (paletteColor & 0x0000FF);

				SDL_WriteSurfacePixel(m_Surface, x, LY, r, g, b, 0xFF);
			}

			x++;

			if (xFlip)
				pixelIteration++;
			else
				pixelIteration--;
		} while (pixelIteration <= 7 && pixelIteration >= 0);
	}
}

/* Set the internal surface's color to the lightest color.
 */
void LCD::DisableLCD()
{
	SDL_FillSurfaceRect(m_Surface, NULL, m_Palette[0]);
}

/* Blit the internal surface to the window's surface and update it.
 */
void LCD::Render()
{
	if (!m_Ready)
		return;

	// ShowTiles();
	SDL_BlitSurfaceScaled(m_Surface, nullptr, m_Screen, &m_BlitRect, SDL_SCALEMODE_NEAREST);
	SDL_UpdateWindowSurface(m_Window);
}

/* Show tile banks 0-2 on screen (write them to the internal surface).
 */
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
			color = (GetBit(msb, j) << 1) | GetBit(lsb, j);

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

		if (y % 8 == 0)
		{
			if (x == 160)
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
