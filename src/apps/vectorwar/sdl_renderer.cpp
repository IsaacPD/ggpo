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

SDLRenderer::SDLRenderer(SDL_Renderer* renderer) :
   _rend(renderer)
{
   // HDC hdc = GetDC(_hwnd);
   // GetClientRect(hwnd, &_rc);
   // CreateGDIFont(hdc);
   // ReleaseDC(_hwnd, hdc);

   *_status = '\0';

   if (SDL_GetDisplayBounds(0, &_rc) != 0) {
      SDL_Log("SDL_GetDisplayBounds n SDLRenderer failed: %s", SDL_GetError());
      exit(1);
   }
   _shipColors[0] = ((RGB) {255, 0, 0});
   _shipColors[1] = ((RGB) {0, 255, 0});
   _shipColors[2] = ((RGB) {0, 0, 255});
   _shipColors[3] = ((RGB) {128, 128, 128});
}


SDLRenderer::~SDLRenderer()
{
   //DeleteObject(_font);
}


void
SDLRenderer::Draw(GameState &gs, NonGameState &ngs)
{
   SDL_RenderClear(_rend);

   SDL_SetRenderDrawColor(_rend, 0, 0, 0 , SDL_ALPHA_OPAQUE);
   SDL_RenderFillRect(_rend, &_rc);
   SDL_RenderDrawRect(_rend, &_rc);

   // FillRect(hdc, &_rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
   // FrameRect(hdc, &gs._bounds, (HBRUSH)GetStockObject(WHITE_BRUSH));

   // SetBkMode(hdc, TRANSPARENT);
   // SelectObject(hdc, _font);

   for (int i = 0; i < gs._num_ships; i++) {
      // SetTextColor(hdc, _shipColors[i]);
      // SelectObject(hdc, _shipPens[i]);
      DrawShip(i, gs);
      DrawConnectState(gs._ships[i], ngs.players[i], _shipColors[i]);
   }

   //SetTextAlign(hdc, TA_BOTTOM | TA_CENTER);
   //TextOutA(hdc, _rc.h / 2, _rc.y + _rc.h - 32, _status, strlen(_status));

   //SetTextColor(hdc,((RGB) {192, 192, 192}));
   //RenderChecksum(hdc, 40, ngs.periodic);
   //SetTextColor(hdc, ((RGB) {128, 128, 128}));
   //RenderChecksum(hdc, 56, ngs.now);

   //SwapBuffers(hdc);

   SDL_RenderPresent(_rend);

   // ReleaseDC(_hwnd, hdc);
}

void
SDLRenderer::RenderChecksum(int y, NonGameState::ChecksumInfo &info)
{
   char checksum[128];
   sprintf(checksum, "Frame: %04d  Checksum: %08x", info.framenumber, info.checksum);
   // TextOutA(hdc, _rc.w / 2, _rc.y + y, checksum, strlen(checksum));
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
   SDL_Rect bullet = { 0 };
   SDL_Point shape[] = {
      { SHIP_RADIUS,           0 },
      { -SHIP_RADIUS,          SHIP_WIDTH },
      { SHIP_TUCK-SHIP_RADIUS, 0 },
      { -SHIP_RADIUS,          -SHIP_WIDTH },
      { SHIP_RADIUS,           0 },
   };
   // int alignments[] = {
   //    TA_TOP | TA_LEFT,
   //    TA_TOP | TA_RIGHT,
   //    TA_BOTTOM | TA_LEFT,
   //    TA_BOTTOM | TA_RIGHT,
   // };
   SDL_Point text_offsets[] = {
      { gs._bounds.x  + 2, gs._bounds.y + 2 },
      { gs._bounds.x + gs._bounds.w - 2, gs._bounds.y + 2 },
      { gs._bounds.x  + 2, gs._bounds.y + gs._bounds.h - 2 },
      { gs._bounds.x + gs._bounds.w - 2, gs._bounds.y + gs._bounds.h - 2 },
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
         bullet.w = 1;
         bullet.y = ship->bullets[i].position.y - 1;
         bullet.h = 1;
         SDL_RenderFillRect(_rend, &bullet);
      }
   }
   // SetTextAlign(hdc, alignments[which]);
   // sprintf(buf, "Hits: %d", ship->score);
   // TextOutA(hdc, text_offsets[which].x, text_offsets[which].y, buf, strlen(buf));
}

void
SDLRenderer::DrawConnectState(Ship& ship, PlayerConnectionInfo &info, RGB color)
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

   //if (*status) {
   //   SetTextAlign(hdc, TA_TOP | TA_CENTER);
   //   TextOutA(hdc, ship.position.x, ship.position.y + PROGRESS_TEXT_OFFSET, status, strlen(status));
   //}
   if (progress >= 0) {
			SDL_SetRenderDrawColor(_rend, 0, 0, 0, SDL_ALPHA_OPAQUE);
      SDL_Rect rc = { ship.position.x - (PROGRESS_BAR_WIDTH / 2),
                      ship.position.y + PROGRESS_BAR_TOP_OFFSET,
                      PROGRESS_BAR_WIDTH / 2,
                      PROGRESS_BAR_TOP_OFFSET + PROGRESS_BAR_HEIGHT };

      SDL_RenderDrawRect(_rend, &rc);
      rc.w = rc.x + std::min(100, progress) * PROGRESS_BAR_WIDTH / 100;
      //InflateRect(&rc, -1, -1);
      SDL_RenderFillRect(_rend, &rc);
   }
}


void
SDLRenderer::CreateFont()
{
//   _font = CreateFont(-12,
//                      0,                         // Width Of Font
//                      0,                         // AnGDIe Of Escapement
//                      0,                         // Orientation AnGDIe
//                      0,                         // Font Weight
//                      FALSE,                     // Italic
//                      FALSE,                     // Underline
//                      FALSE,                     // Strikeout
//                      ANSI_CHARSET,              // Character Set Identifier
//                      OUT_TT_PRECIS,             // Output Precision
//                      CLIP_DEFAULT_PRECIS,       // Clipping Precision
//                      ANTIALIASED_QUALITY,       // Output Quality
//                      FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
//                      L"Tahoma");                // Font Name

}
