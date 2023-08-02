#pragma once

#include "GameCore/Rendering/Texture2D.h"

namespace Game {
	static std::map<int, bool> Keymap;

	SchedulerRate* Scheduler = nullptr;

	enum GameEvent {
		NONE,
		Update,
		KeyDown,
		KeyUp,
		KeyPress
	};

	class FontRepos
	{
	public:
		static inline std::vector<FontBitmap*> fonts = {};

		static const char* FontName(FontBitmap* font)
		{
			if (font->FontPath[0] == 'f')
				return "Mojangles";

			return "Unknown";
		}

		static void PutFont(FontBitmap* font)
		{
			bool fontExists = false;
			for (FontBitmap* fontPtr : fonts)
			{
				if (font == fontPtr)
					fontExists = true;
			}

			if (not fontExists)
			{
				fonts.push_back(font);
			}
		}

		static FontBitmap* GetFont(const char* font)
		{
			for (FontBitmap* fontPtr : fonts)
			{
				if (strcmp(FontName(fontPtr), font) == 0)
					return fontPtr;
			}

			return nullptr;
		}

		static FontBitmap* GetClientFont()
		{
			return GetFont("Mojangles");
		}
	};
}