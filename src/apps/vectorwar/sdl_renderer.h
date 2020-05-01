#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

#include "renderer.h"
#include "SDL2/SDL.h"

/*
 * sdl_renderer.h --
 *
 * A simple C++ renderer that uses SDL to render the game state.
 *
 */

typedef struct RGB {
   uint8_t r;
   uint8_t g;
   uint8_t b;
} RGB;

class SDLRenderer : public Renderer {
public:
   SDLRenderer();
   ~SDLRenderer();

   int WindowWidth();
   int WindowHeight();

   virtual void Draw(GameState &gs, NonGameState &ngs);
   virtual void SetStatusText(const char *text);

protected:
   void RenderChecksum(int y, NonGameState::ChecksumInfo &info);
   void DrawShip(int which, GameState &gamestate);
   void DrawConnectState(Ship &ship, PlayerConnectionInfo &info, RGB color);
   void CreateFont();

   // HFONT          _font;
   SDL_Renderer   *_rend;
   SDL_Window     *_win;
   SDL_Rect       _rc;
   char           _status[1024];
   RGB            _shipColors[4];
};

#endif
