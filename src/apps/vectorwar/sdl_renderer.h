#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

#include "renderer.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

/*
 * sdl_renderer.h --
 *
 * A simple C++ renderer that uses SDL to render the game state.
 *
 */

class SDLRenderer : public Renderer {
public:
   SDLRenderer();
   ~SDLRenderer();

   int WindowWidth();
   int WindowHeight();

   virtual void Draw(GameState &gs, NonGameState &ngs);
   virtual void DrawText(char* text, SDL_Rect* dstrect, SDL_Color* color);
   virtual void SetStatusText(const char *text);

protected:
   void RenderChecksum(int y, NonGameState::ChecksumInfo &info, SDL_Color* color);
   void DrawShip(int which, GameState &gamestate);
   void DrawConnectState(Ship &ship, PlayerConnectionInfo &info, SDL_Color* color);
   void CreateFont();

   TTF_Font       *_font;
   SDL_Renderer   *_rend;
   SDL_Window     *_win;
   SDL_Rect       _rc;
   char           _status[1024];
   SDL_Color      _shipColors[4];
   SDL_Color      _white;
};

#endif
