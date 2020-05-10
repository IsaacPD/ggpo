#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "vectorwar.h"
#include "sdl_renderer.h"
#include "gamestate.h"

#define  PROGRESS_BAR_WIDTH        100
#define  PROGRESS_BAR_TOP_OFFSET    22
#define  PROGRESS_BAR_HEIGHT         8
#define  PROGRESS_TEXT_OFFSET       (PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT + 4)

SDL_Window*
CreateMainWindow()
{
   SDL_Window *window;

   char titlebuf[128];
   snprintf(titlebuf, strlen(titlebuf),
      "(pid: %d) ggpo sdk sample: vector war", GetProcessID());

   window = SDL_CreateWindow(
       titlebuf,
       SDL_WINDOWPOS_UNDEFINED,           // initial x position
       SDL_WINDOWPOS_UNDEFINED,           // initial y position
       640,                               // width, in pixels
       480,                               // height, in pixels
       SDL_WINDOW_SHOWN
   );

   return window;
}


SDLRenderer::SDLRenderer()
{
   int ret = SDL_Init(SDL_INIT_VIDEO);
   if (ret) {
      fprintf( stderr, "Error (SDL): could not initialise SDL: %s\n",
         SDL_GetError());
      exit(1);
   }

   if (TTF_Init() == -1) {
     printf("SDL TTF_Init: %s\n", TTF_GetError());
     exit(2);
   }

	 CreateFont();

   _win = CreateMainWindow();

   // initialise the first available renderer
   _rend = SDL_CreateRenderer(_win, -1,
      SDL_RENDERER_ACCELERATED);

   *_status = '\0';

   SDL_GetWindowSize(_win, &_rc.w, &_rc.h);

   _shipColors[0] = ((SDL_Color) {255, 0, 0});
   _shipColors[1] = ((SDL_Color) {0, 255, 0});
   _shipColors[2] = ((SDL_Color) {0, 0, 255});
   _shipColors[3] = ((SDL_Color) {128, 128, 128});

   _white = {255, 255, 255};
}


SDLRenderer::~SDLRenderer()
{
   SDL_DestroyWindow(_win);
   SDL_DestroyRenderer(_rend);
   TTF_Quit();
   SDL_Quit();
}

void print_SDL_error(const char* loc)
{
	printf("SDL Error (%s): %s\n", loc, SDL_GetError());
}

void
SDLRenderer::Draw(GameState &gs, NonGameState &ngs)
{
   int ret = SDL_SetRenderDrawColor(_rend, 0, 0, 0 , SDL_ALPHA_OPAQUE);
   if (ret) {
		 print_SDL_error("SDL_SetRenderDrawColor");
   }

   // render clear actually uses the draw coor
   ret = SDL_RenderClear(_rend);
   if (ret) {
		 print_SDL_error("SDL_RenderClear");
   }

   // FillRect(hdc, &_rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
   // FrameRect(hdc, &gs._bounds, (HBRUSH)GetStockObject(WHITE_BRUSH));

   // SetBkMode(hdc, TRANSPARENT);
   // SelectObject(hdc, _font);

   for (int i = 0; i < gs._num_ships; i++) {
      SDL_Color color = _shipColors[i];
      int ret = SDL_SetRenderDrawColor(_rend, color.r, color.g, color.b,
           SDL_ALPHA_OPAQUE);
      if (ret) {
        print_SDL_error("SDL_SetRenderDrawColor shipcolor");
      }

      DrawShip(i, gs);
      DrawConnectState(gs._ships[i], ngs.players[i], &_shipColors[i]);
   }

   //SetTextAlign(hdc, TA_BOTTOM | TA_CENTER);
   //TextOutA(hdc, _rc.h / 2, _rc.y + _rc.h - 32, _status, strlen(_status));

   SDL_Rect dst = {
     // more or less centered
     .x = (_rc.w / 2) - 80,
     .y = _rc.h - 32,
   };
   DrawText(_status, &dst, &_white);

   SDL_Color col = {192, 192, 192};
   RenderChecksum(40, ngs.periodic, &col);

   col = {128, 128, 128};
   RenderChecksum(56, ngs.now, &col);

   SDL_RenderPresent(_rend);
}

void
SDLRenderer::DrawText(char* text, SDL_Rect* dst, SDL_Color* color)
{
  if (!text) {
    return;
  }
  if (strlen(text) == 0) {
    return;
  }

  int ret = TTF_SizeUTF8(_font, text, &dst->w, &dst->h);
  if (ret) {
    printf("TTF_SizeUTF8: %s\n", TTF_GetError());
    exit(1);
  }

  SDL_Surface* text_surface = TTF_RenderUTF8_Solid(_font, text, *color);
  if (!text_surface) {
    printf("TTF_RenderUTF8_Solid: %s\n", TTF_GetError());
    exit(1);
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(_rend, text_surface);
  ret = SDL_RenderCopy(_rend, texture, NULL, dst);
  if (ret) {
    printf("SDL_RenderCopy: %s\n", TTF_GetError());
    exit(1);
  }

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(text_surface);
}

void
SDLRenderer::RenderChecksum(int y, NonGameState::ChecksumInfo &info, SDL_Color* color)
{
   char checksum[128];
   sprintf(checksum, "Frame: %04d  Checksum: %08x", info.framenumber, info.checksum);
   SDL_Rect dst = {
     // about centered
     .x = (_rc.w / 2) - 120,
     .y = y,
   };
   DrawText(checksum, &dst, color);
}


void
SDLRenderer::SetStatusText(const char *text)
{
   strcpy(_status, text);
}

void
SDLRenderer::DrawShip(int which, GameState &gs)
{
   Ship *ship = gs._ships + which;
   SDL_Rect bullet = {
     .w = 1,
     .h = 1,
   };
   SDL_Point shape[] = {
      { SHIP_RADIUS,           0 },
      { -SHIP_RADIUS,          SHIP_WIDTH },
      { SHIP_TUCK-SHIP_RADIUS, 0 },
      { -SHIP_RADIUS,          -SHIP_WIDTH },
      { SHIP_RADIUS,           0 },
   };
   const int alignment_adjustment[] = {
      -5,
      65,
      -5,
      65,
   };
   const SDL_Point text_offsets[] = {
      { .x = gs._bounds.x  + 2,
        .y = gs._bounds.y + 2
      },
      { .x = gs._bounds.x + gs._bounds.w - 2,
        .y = gs._bounds.y + 2
      },
      { .x = gs._bounds.x  + 2,
        .y = gs._bounds.y + gs._bounds.h - 20
      },
      { .x = gs._bounds.x + gs._bounds.w - 2,
        .y = gs._bounds.y + gs._bounds.h - 20
      }
   };

   char buf[32];
   int i;

   for (i = 0; i < ARRAY_SIZE(shape); i++) {
      int newx, newy;
      double cost, sint, theta;

      theta = (double)ship->heading * PI / 180;
      cost = ::cos(theta);
      sint = ::sin(theta);

      newx = shape[i].x * cost - shape[i].y * sint;
      newy = shape[i].x * sint + shape[i].y * cost;

      shape[i].x = newx + ship->position.x;
      shape[i].y = newy + ship->position.y;
   }
   SDL_RenderDrawLines(_rend, shape, ARRAY_SIZE(shape));

   for (int i = 0; i < MAX_BULLETS; i++) {
      if (ship->bullets[i].active) {
         bullet.x = ship->bullets[i].position.x - 1;
         bullet.y = ship->bullets[i].position.y - 1;
         SDL_RenderFillRect(_rend, &bullet);
      }
   }

   sprintf(buf, "Hits: %d", ship->score);

   SDL_Rect dst = {
     .x = text_offsets[which].x - alignment_adjustment[which],
     .y = text_offsets[which].y,
   };
   DrawText(buf, &dst, &_shipColors[which]);
}

void
SDLRenderer::DrawConnectState(Ship& ship, PlayerConnectionInfo &info, SDL_Color* color)
{
   char status[64];
   int progress = -1;

   *status = '\0';
   switch (info.state) {
      case Connecting:
         sprintf(status, (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Connecting...");
         break;

      case Synchronizing:
         progress = info.connect_progress;
         sprintf(status, (info.type == GGPO_PLAYERTYPE_LOCAL) ? "Local Player" : "Synchronizing...");
         break;

      case Disconnected:
         sprintf(status, "Disconnected");
         break;

      case Disconnecting:
         sprintf(status, "Waiting for player...");
         progress = (GetCurrentTimeMS() - info.disconnect_start) * 100 / info.disconnect_timeout;
         break;
   }

   if (*status) {
      SDL_Rect dst = {
        .x = ship.position.x - 40,
        .y = ship.position.y + PROGRESS_TEXT_OFFSET,
      };
      DrawText(status, &dst, color);
   }

   if (progress >= 0) {
      SDL_Rect rc = { ship.position.x - (PROGRESS_BAR_WIDTH / 2),
                      ship.position.y + PROGRESS_BAR_TOP_OFFSET,
                      PROGRESS_BAR_WIDTH / 2,
                      PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT };
      SDL_RenderDrawRect(_rend, &rc);
      rc.w = rc.x + std::min(100, progress) * PROGRESS_BAR_WIDTH / 100;
      SDL_RenderFillRect(_rend, &rc);
   }
}

int
SDLRenderer::WindowWidth()
{
   return _rc.w;
}

int
SDLRenderer::WindowHeight()
{
   return _rc.h;
}


void
SDLRenderer::CreateFont()
{
   _font = TTF_OpenFont("Inconsolata-Regular.ttf", 16);
   if(!_font) {
     printf("TTF_OpenFont: %s\n", TTF_GetError());
     exit(1);
   }
}
