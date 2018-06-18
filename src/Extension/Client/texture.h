//
// Created by kenny on 18/06/18.
//

#ifndef ARM11_35_TEXTURE_H
#define ARM11_35_TEXTURE_H
#include <stdbool.h>
#include "SDL_Libraries.h"

struct Texture{
  SDL_Texture *texture;
  int width;
  int height;
};

struct Texture *setupTexture();
void freeTexture (struct Texture *texture);
void freeTextureStructure (struct Texture *texture);


bool loadImage (struct Texture *texture, SDL_Renderer* renderer, char* path,
                bool transparency, uint8_t r, uint8_t g, uint8_t b);

bool loadText(struct Texture *texture, TTF_Font* font_name, const char* text, SDL_Colour colour, SDL_Renderer* renderer);
void setBlendMode (struct Texture *texture, SDL_BlendMode blendMode);
void setAlphaTransparency (struct Texture *texture, uint8_t a);
void renderTexture (struct Texture *texture, SDL_Renderer* renderer, SDL_Rect* screenRect, SDL_Rect* textureRect);

#endif //ARM11_35_TEXTURE_H
