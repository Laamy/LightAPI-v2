#pragma once

#include "../Math/TextHolder.h"

#include "../ClientInstance.h"
#include "../Math/CaretMeasureData.h"
#include "ScreenContext.h"
#include "UIScene.h"
#include "TextMeasureData.h"

class ComponentRenderBatch;
class NinesliceInfo;
class HashedString;

class RenderContext
{
public:
    uintptr_t** VTable;
    ClientInstance* ClientInstance;
    ScreenContext* ScreenContext;
    void* MeasureStrag;
    UIScene* UIScene;
    float TextAlpha;

public: // functions
    auto GetVTable(int index)
    {
        return VTable[index];
    }

    void drawRectangle(Vector4<float> const& rect, UIColor const& colour, float alpha, int radius)
    {
        return CallFunc<void, uintptr_t, Vector4<float> const&, UIColor const&, float, int>(
            GetVTable(12),
            (uintptr_t)this,
            rect, // * GuiInfo::GuiScale
            colour,
            alpha,
            radius
        );
    }

    void fillRectangle(Vector4<float> const& rect, UIColor const& colour, float alpha)
    {
        return CallFunc<void, uintptr_t, Vector4<float> const&, UIColor const&, float>(GetVTable(13), (uintptr_t)this, rect, colour, alpha);
    }

    void fillRectangle(Vector4<float> const& rect, UIColor const* colour, float alpha)
    {
        return CallFunc<
            void,
            uintptr_t,
            Vector4<float> const&,
            UIColor const*,
            float
        >(GetVTable(13), (uintptr_t)this, rect, colour, alpha);
    }

    float getMeasureData(void* font, TextHolder* str, float textSize, bool calcWidth)
    {
        if (!font) font = Game::FontRepos::GetClientFont();

        if (!calcWidth)
            textSize * 10;

        return CallFunc<
            float,
            uintptr_t,
            void*,
            TextHolder*,
            float,
            bool
        >(GetVTable(1), (uintptr_t)this, font, str, textSize, calcWidth);
    }

    void drawText(FontBitmap* font, const float* pos, TextHolder* text, const float* color, float alpha, unsigned int textAlignment, const TextMeasureData* textMeasureData, const CaretMeasureData* caretMeasureData) {
        if (!font) font = Game::FontRepos::GetClientFont();
        return CallFunc<
            void,
            FontBitmap*,
            const float*,
            TextHolder*,
            const float*,
            float,
            unsigned int,
            const TextMeasureData*,
            const CaretMeasureData*
        >(GetVTable(5), font, pos, text, color, alpha, textAlignment, textMeasureData, caretMeasureData);
    };

    void flushString(float delta) {
        return CallFunc<
            void,
            float
        >(GetVTable(6), delta);
    };

    void setClippingRectangle(Vector4<float> const& pos)
    {
        return CallFunc<
            void,
            Vector4<float> const&
        >(GetVTable(20), pos);
    };

    void saveCurrentClippingRectangle()
    {
        return CallFunc<
            void
        >(GetVTable(22));
    };

    void restoreSavedClippingRectangle()
    {
        return CallFunc<
            void
        >(GetVTable(23));
    };

    void drawImage(const TextureData* a2, Vector2<float> const& ImagePos, Vector2<float> const& ImageDimension, Vector2<float> const& uvPos, Vector2<float> const& uvSize)
    {
        return CallFunc<
            void,
            const TextureData*,
            Vector2<float> const&,
            Vector2<float> const&,
            Vector2<float> const&,
            Vector2<float> const&
        >(GetVTable(7), a2, ImagePos, ImageDimension, uvPos, uvSize);
    };

    void flushImages(UIColor& color, float opacity, class StringHasher& hashedString)
    {
        return CallFunc<
            void,
            UIColor&,
            float,
            class StringHasher&
        >(GetVTable(9), color, opacity, hashedString);
    };

    /*TextureData* getTexture(TextureData* texture, ResourceLocation* resourceLocation)
    {
        return CallFunc<
            TextureData*,
            TextureData*,
            ResourceLocation
        >(GetVTable(29), texture, resourceLocation);
    };*/
};