//
// Created by kenny on 18/06/18.
//
#include <stdlib.h>
#include "texture.h"

struct Texture *setupTexture(){
  struct Texture *texture = (struct Texture *) malloc(sizeof(struct Texture));
  texture->texture = NULL;
  texture->width = 0;
  texture->height = 0;
}

void freeTexture(struct Texture *texture){

  SDL_DestroyTexture(texture->texture);
  texture->width = 0;
  texture->height = 0;
}

void freeTextureStructure(struct Texture *texture){
  if (texture) {
    freeTexture(texture);
    free(texture);
  }
}

void setBlendMode (struct Texture *texture, SDL_BlendMode blendMode){

  if (texture)
    SDL_SetTextureBlendMode(texture->texture,blendMode);
}

void setAlphaTransparency (struct Texture *texture, uint8_t a){
  SDL_SetTextureAlphaMod(texture->texture,a);
}


bool loadImage (struct Texture *texture, SDL_Renderer* renderer, char* path, bool transparency,
                uint8_t r, uint8_t g, uint8_t b ){

  //Default, r=g=b=0

  freeTexture(texture);

  //Load an image
  SDL_Surface* temp = IMG_Load (path);

  //This isn't required in this program, but it adds a tinge of a certain colour
  if (transparency){
    SDL_SetColorKey (temp,SDL_TRUE,SDL_MapRGB(temp->format,r,g,b));
  }

  if (!temp)
    return false;

  //Store width and height of an image
  texture->width = temp->w;
  texture->height = temp->h;

  //Convert SDL_Surface to SDL_Texture
  texture->texture = SDL_CreateTextureFromSurface (renderer,temp);

  if (!texture->texture)
    return false;

  return true;
}

bool loadText(struct Texture *texture, TTF_Font* font_name, const char* text,
              SDL_Colour colour, SDL_Renderer* renderer){

  freeTexture(texture);
  //Loads a text that can be rendered
  SDL_Surface* temp = TTF_RenderUTF8_Blended(font_name,text,colour);

  if (!temp)
    return false;

  //Store width and height of text
  //This won't be easily available when its converted into a texture
  texture->width = temp -> w;
  texture->height = temp -> h;

  //Convert text to SDL_Texture so text can be rendered on a renderer
  texture->texture = SDL_CreateTextureFromSurface (renderer, temp);

  //Free the temporary SDL_Surface*
  /**Note: I don't use surfaces, as texture rendering is often faster**/
  SDL_FreeSurface(temp);

  if (!texture->texture)
    return false;


  return true;
}

void renderTexture (struct Texture *texture,SDL_Renderer* renderer,
                    SDL_Rect* screenRect, SDL_Rect* textureRect){

  //Render texture. Fourth argument is the amount of the texture to consider for rendering
  //Passing null implies the entire texture.
  //Third argument is the amount of the screen to take up when rendering texture
  //This means a texture can be resized
  SDL_RenderCopy (renderer,texture->texture,textureRect,screenRect);
}

