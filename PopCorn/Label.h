#pragma once

#include "Config.h"

//------------------------------------------------------------------------------------------------------------
class ALabel
{
public:
	ALabel(int x_pos, int y_pos, int width, int height, const AFont &font, const AColor &color);

	void Draw(HDC hdc);

	RECT Content_Rect;
	AString Content;

private:
	int X_Pos, Y_Pos;
	int Width, Height;
	const AFont &Font;
	const AColor &Color;
};
//------------------------------------------------------------------------------------------------------------
