/*
*  C Implementation: SDL_extras
*
* Description: a few handy functions for using SDL graphics.
*
*
* Author: David Bruce,,, <dbruce@tampabay.rr.com>, (C) 2007
*
* Copyright: GPL v3 or later
*
*/
#include <math.h>

#include "SDL_extras.h"
#include "tuxmath.h"
#include "pixels.h"

#ifdef SDL_Pango
#include "SDL_Pango.h"
#endif


/* DrawButton() creates and draws a translucent button with */
/* rounded ends.  All colors and alpha values are supported.*/
void DrawButton(SDL_Rect* target_rect,
                int radius,
                Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  /* NOTE - we use a 32-bit temp surface even if we have a 16-bit */
  /* screen - it gets converted during blitting.                  */
  SDL_Surface* tmp_surf = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
                                          target_rect->w,
                                          target_rect->h,
                                          32,
                                          rmask, gmask, bmask, amask);
  Uint32 color = SDL_MapRGBA(tmp_surf->format, r, g, b, a);
  SDL_FillRect(tmp_surf, NULL, color);
  RoundCorners(tmp_surf, radius);

  SDL_BlitSurface(tmp_surf, NULL, screen, target_rect);
  SDL_FreeSurface(tmp_surf);
}



void RoundCorners(SDL_Surface* s, Uint16 radius)
{
  int y = 0;
  int x_dist, y_dist;
  Uint32* p = NULL;
  Uint32 alpha_mask;
  int bytes_per_pix;

  if (!s)
    return;
  if (SDL_LockSurface(s) == -1)
    return;

  bytes_per_pix = s->format->BytesPerPixel;
  if (bytes_per_pix != 4)
    return;

  /* radius cannot be more than half of width or height: */
  if (radius > (s->w)/2)
    radius = (s->w)/2;
  if (radius > (s->h)/2)
    radius = (s->h)/2;


  alpha_mask = s->format->Amask;

  /* Now round off corners: */
  /* upper left:            */
  for (y = 0; y < radius; y++)
  {
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* upper right:            */
  for (y = 0; y < radius; y++)
  {
    /* start at end of top row: */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);

    x_dist = radius;
    y_dist = radius - y;

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }

  /* bottom left:            */
  for (y = (s->h - 1); y > (s->h - radius); y--)
  {
    /* start at beginning of bottom row */
    p = (Uint32*)(s->pixels + (y * s->pitch));
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p++;
      x_dist--;
    }
  }

  /* bottom right:            */
  for (y = (s->h - 1); y > (s->h - radius); y--)
  {
    /* start at end of bottom row */
    p = (Uint32*)(s->pixels + ((y + 1) * s->pitch) - bytes_per_pix);
    x_dist = radius;
    y_dist = y - (s->h - radius);

    while (((x_dist * x_dist) + (y_dist * y_dist)) > (radius * radius))
    {
      /* (make pixel (x,y) transparent) */
      *p = *p & ~alpha_mask;
      p--;
      x_dist--;
    }
  }
  SDL_UnlockSurface(s);
}


/**********************
 Flip:
   input: a SDL_Surface, x, y
   output: a copy of the SDL_Surface flipped via rules:

     if x is a nonzero value, then flip horizontally
     if y is a nonzero value, then flip vertically

     note: you can have it flip both
**********************/
SDL_Surface* Flip( SDL_Surface *in, int x, int y ) {
	SDL_Surface *out, *tmp;
	SDL_Rect from_rect, to_rect;
	Uint32	flags;
	Uint32  colorkey=0;

	/* --- grab the settings for the incoming pixmap --- */

	SDL_LockSurface(in);
	flags = in->flags;

	/* --- change in's flags so ignore colorkey & alpha --- */

	if (flags & SDL_SRCCOLORKEY) {
		in->flags &= ~SDL_SRCCOLORKEY;
		colorkey = in->format->colorkey;
	}
	if (flags & SDL_SRCALPHA) {
		in->flags &= ~SDL_SRCALPHA;
	}

	SDL_UnlockSurface(in);

	/* --- create our new surface --- */

	out = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		in->w, in->h, 32, rmask, gmask, bmask, amask);

	/* --- flip horizontally if requested --- */

	if (x) {
		from_rect.h = to_rect.h = in->h;
		from_rect.w = to_rect.w = 1;
		from_rect.y = to_rect.y = 0;
		from_rect.x = 0;
		to_rect.x = in->w - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.x++;
			to_rect.x--;
		} while (to_rect.x >= 0);
	}

	/* --- flip vertically if requested --- */

	if (y) {
		from_rect.h = to_rect.h = 1;
		from_rect.w = to_rect.w = in->w;
		from_rect.x = to_rect.x = 0;
		from_rect.y = 0;
		to_rect.y = in->h - 1;

		do {
			SDL_BlitSurface(in, &from_rect, out, &to_rect);
			from_rect.y++;
			to_rect.y--;
		} while (to_rect.y >= 0);
	}

	/* --- restore colorkey & alpha on in and setup out the same --- */

	SDL_LockSurface(in);

	if (flags & SDL_SRCCOLORKEY) {
		in->flags |= SDL_SRCCOLORKEY;
		in->format->colorkey = colorkey;
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
		out->flags |= SDL_SRCCOLORKEY;
		out->format->colorkey = colorkey;
	} else if (flags & SDL_SRCALPHA) {
		in->flags |= SDL_SRCALPHA;
		tmp = SDL_DisplayFormatAlpha(out);
		SDL_FreeSurface(out);
		out = tmp;
	} else {
		tmp = SDL_DisplayFormat(out);
		SDL_FreeSurface(out);
		out = tmp;
	}

	SDL_UnlockSurface(in);

	return out;
}

/* Blend two surfaces together. The third argument is between 0.0 and
   1.0, and represents the weight assigned to the first surface.  If
   the pointer to the second surface is NULL, this performs fading.

   Currently this works only with RGBA images, but this is largely to
   make the (fast) pointer arithmetic work out; it could be easily
   generalized to other image types. */
SDL_Surface* Blend(SDL_Surface *S1,SDL_Surface *S2,float gamma)
{
  SDL_PixelFormat *fmt1,*fmt2;
  Uint8 r1,r2,g1,g2,b1,b2,a1,a2;
  SDL_Surface *tmpS,*ret;
  Uint32 *cpix1,*epix1,*cpix2,*epix2;
  float gamflip;

  gamflip = 1.0-gamma;
  if (gamma < 0 || gamflip < 0) {
    perror("gamma must be between 0 and 1");
    exit(0);
  }
  fmt1 = S1->format;
  if (fmt1->BitsPerPixel != 32) {
    perror("This works only with RGBA images");
    return S1;
  }
  if (S2 != NULL) {
    fmt2 = S2->format;
    if (fmt2->BitsPerPixel != 32) {
    perror("This works only with RGBA images");
    return S1;
    }
    // Check that both images have the same width dimension
    if (S1->w != S2->w) {
      printf("S1->w %d, S2->w %d;  S1->h %d, S2->h %d\n",
	     S1->w,S2->w,S1->h,S2->h);
      printf("Both images must have the same width dimensions\n");
      return S1;
    }
  }

  tmpS = SDL_ConvertSurface(S1,fmt1,SDL_SWSURFACE);
  SDL_LockSurface(tmpS);
  // We're going to go through the pixels in reverse order, to start
  // from the bottom of each image. That way, we can blend things that
  // are not of the same height and have them align at the bottom.
  // So the "ending pixel" (epix) will be before the first pixel, and
  // the current pixel (cpix) will be the last pixel.
  epix1 = (Uint32*) tmpS->pixels-1;
  cpix1 = epix1 + tmpS->w*tmpS->h;
  if (S2 != NULL) {
    SDL_LockSurface(S2);
    epix2 = (Uint32*) S2->pixels-1;
    cpix2 = epix2 + S2->w*S2->h;
  } else {
    epix2 = epix1;
    cpix2 = cpix1;
  }

  for (; cpix1 > epix1; cpix1--,cpix2--) {
    SDL_GetRGBA(*cpix1,fmt1,&r1,&g1,&b1,&a1);
    a1 = gamma*a1;
    if (S2 != NULL && cpix2 > epix2) {
      SDL_GetRGBA(*cpix2,fmt2,&r2,&g2,&b2,&a2);
      r1 = gamma*r1 + gamflip*r2;
      g1 = gamma*g1 + gamflip*g2;
      b1 = gamma*b1 + gamflip*b2;
      a1 += gamflip*a2;
    }
    *cpix1 = SDL_MapRGBA(fmt1,r1,g1,b1,a1);
  }
  SDL_UnlockSurface(tmpS);
  if (S2 != NULL)
    SDL_UnlockSurface(S2);

  ret = SDL_DisplayFormatAlpha(tmpS);
  SDL_FreeSurface(tmpS);

  return ret;
}

#ifdef SDL_Pango
SDLPango_Context *context = NULL;
void init_SDLPango_Context()
{
   context =  SDLPango_CreateContext_GivenFontDesc(DEFAULT_FONT_NAME);
}
void free_SDLPango_Context()
{
  if(context != NULL)
    SDLPango_FreeContext(context);
  context = NULL;
}
#endif


/* BlackOutline() creates a surface containing text of the designated */
/* foreground color, surrounded by a black shadow, on a transparent    */
/* background.  The appearance can be tuned by adjusting the number of */
/* background copies and the offset where the foreground text is       */
/* finally written (see below).                                        */
SDL_Surface* BlackOutline(unsigned char *t, TTF_Font *font, SDL_Color *c)
{
  SDL_Surface* out = NULL;
  SDL_Surface* black_letters = NULL;
  SDL_Surface* white_letters = NULL;
  SDL_Surface* bg = NULL;
  SDL_Rect dstrect;
  Uint32 color_key;

  if (!t || !font || !c)
  {
    fprintf(stderr, "BlackOutline(): invalid ptr parameter, returning.");
    return NULL;
  }

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nEntering BlackOutline(): \n");
  fprintf( stderr, "BlackOutline of \"%s\"\n", t );
#endif

#ifndef SDL_Pango
  black_letters = TTF_RenderUTF8_Blended(font, t, black);
#else
  if( context != NULL)
  {
    SDLPango_SetDefaultColor(context, MATRIX_TRANSPARENT_BACK_BLACK_LETTER);
    SDLPango_SetText(context, t, -1);
    black_letters = SDLPango_CreateSurfaceDraw(context);
  }
  else {
    black_letters = TTF_RenderUTF8_Blended(font, t, black);
  }
#endif

  if (!black_letters)
  {
    fprintf (stderr, "Warning - BlackOutline() could not create image for %s\n", t);
    return NULL;
  }

  bg = SDL_CreateRGBSurface(SDL_SWSURFACE,
                            (black_letters->w) + 5,
                            (black_letters->h) + 5,
                             32,
                             rmask, gmask, bmask, amask);
  /* Use color key for eventual transparency: */
  color_key = SDL_MapRGB(bg->format, 01, 01, 01);
  SDL_FillRect(bg, NULL, color_key);

  /* Now draw black outline/shadow 2 pixels on each side: */
  dstrect.w = black_letters->w;
  dstrect.h = black_letters->h;

  /* NOTE: can make the "shadow" more or less pronounced by */
  /* changing the parameters of these loops.                */
  for (dstrect.x = 1; dstrect.x < 4; dstrect.x++)
    for (dstrect.y = 1; dstrect.y < 3; dstrect.y++)
      SDL_BlitSurface(black_letters , NULL, bg, &dstrect );

  SDL_FreeSurface(black_letters);

  /* --- Put the color version of the text on top! --- */
#ifndef SDL_Pango
  white_letters = TTF_RenderUTF8_Blended(font, t, *c);
#else
  if( context != NULL)
  {
    SDLPango_SetDefaultColor(context, MATRIX_TRANSPARENT_BACK_WHITE_LETTER);
    white_letters = SDLPango_CreateSurfaceDraw(context);
  }
  else {
    white_letters = TTF_RenderUTF8_Blended(font, t, *c);
  }
#endif

  dstrect.x = 1;
  dstrect.y = 1;
  SDL_BlitSurface(white_letters, NULL, bg, &dstrect);
  SDL_FreeSurface(white_letters);

  /* --- Convert to the screen format for quicker blits --- */
  SDL_SetColorKey(bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, color_key);
  out = SDL_DisplayFormatAlpha(bg);
  SDL_FreeSurface(bg);

#ifdef TUXMATH_DEBUG
  fprintf( stderr, "\nLeaving BlackOutline(): \n");
#endif

  return out;
}


int inRect( SDL_Rect r, int x, int y) {
	if ((x < r.x) || (y < r.y) || (x > r.x + r.w) || (y > r.y + r.h))
		return 0;
	return 1;
}

/* Darkens the screen by a factor of 2^bits */
void DarkenScreen(Uint8 bits)
{
#if PIXEL_BITS == 32
  Uint32* p;
#elif PIXEL_BITS == 16
  Uint16* p;
#else
  return;
#endif
  Uint32 rm = screen->format->Rmask;
  Uint32 gm = screen->format->Gmask;
  Uint32 bm = screen->format->Bmask;


  int x, y;

  /* (realistically, 1 and 2 are the only useful values) */
  if (bits > 8)
    return;

  p = screen->pixels;

  for (y = 0; y < RES_Y; y++)
  {
    for (x = 0; x < RES_X; x++)
    {
      *p = (((*p&rm)>>bits)&rm)
         | (((*p&gm)>>bits)&gm)
         | (((*p&bm)>>bits)&bm);
      p++;
    }
  }
}


void SwitchScreenMode(void)
{
  int window = (screen->flags & SDL_FULLSCREEN);
  SDL_Surface* oldscreen = screen;

  if (!window)
  {
    screen = SDL_SetVideoMode(fs_res_x,
                              fs_res_y,
                              PIXEL_BITS,
                              SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
  }
  else
  {
    screen = SDL_SetVideoMode(RES_X,
                              RES_Y,
                              PIXEL_BITS,
                              SDL_SWSURFACE|SDL_HWPALETTE);

  }

  if (screen == NULL)
  {
    fprintf(stderr,
            "\nError: I could not switch to %s mode.\n"
            "The Simple DirectMedia error that occured was:\n"
            "%s\n\n",
            window ? "windowed" : "fullscreen",
            SDL_GetError());
    screen = oldscreen;
  }
  else
  {
    SDL_FreeSurface(oldscreen);
    oldscreen = NULL;
    SDL_UpdateRect(screen, 0, 0, 0, 0);
  }

}

//#if 0

/* Swiped shamelessly from TuxPaint
   Based on code from: http://www.codeproject.com/cs/media/imageprocessing4.asp
   copyright 2002 Christian Graus */

SDL_Surface* zoom(SDL_Surface* src, int new_w, int new_h)
{
  SDL_Surface* s;

  /* These function pointers will point to the appropriate */
  /* putpixel() and getpixel() variants to be used in the  */
  /* current colorspace:                                   */
  void (*putpixel) (SDL_Surface*, int, int, Uint32);
  Uint32(*getpixel) (SDL_Surface*, int, int);
 
  float xscale, yscale;
  int x, y;
  int floor_x, ceil_x,
        floor_y, ceil_y;
  float fraction_x, fraction_y,
        one_minus_x, one_minus_y;
  float n1, n2;
  Uint8 r1, g1, b1, a1;
  Uint8 r2, g2, b2, a2;
  Uint8 r3, g3, b3, a3;
  Uint8 r4, g4, b4, a4;
  Uint8 r, g, b, a;

  tmdprintf("\nEntering zoom():\n");

  /* Create surface for zoom: */

  s = SDL_CreateRGBSurface(src->flags,	/* SDL_SWSURFACE, */
			   new_w, new_h, src->format->BitsPerPixel,
                           src->format->Rmask,
                           src->format->Gmask,
                           src->format->Bmask,
                           src->format->Amask);

  if (s == NULL)
  {
    fprintf(stderr, "\nError: Can't build zoom surface\n"
	    "The Simple DirectMedia Layer error that occurred was:\n"
	    "%s\n\n", SDL_GetError());
    return NULL;
//    cleanup();
//    exit(1);
  }

  tmdprintf("orig surface %dx%d, %d bytes per pixel\n",
            src->w, src->h, src->format->BytesPerPixel);
  tmdprintf("new surface %dx%d, %d bytes per pixel\n",
            s->w, s->h, s->format->BytesPerPixel);

  /* Now assign function pointers to correct functions based */
  /* on data format of original and zoomed surfaces:         */
  getpixel = getpixels[src->format->BytesPerPixel];
  putpixel = putpixels[s->format->BytesPerPixel];

  SDL_LockSurface(src);
  SDL_LockSurface(s);

  xscale = (float) src->w / (float) new_w;
  yscale = (float) src->h / (float) new_h;

  for (x = 0; x < new_w; x++)
  {
    for (y = 0; y < new_h; y++)
    {
      /* Here we calculate the new RGBA values for each pixel */
      /* using a "weighted average" of the four pixels in the */
      /* corresponding location in the orginal surface:       */

      /* figure out which original pixels to use in the calc: */
      floor_x = floor((float) x * xscale);
      ceil_x = floor_x + 1;
      if (ceil_x >= src->w)
        ceil_x = floor_x;

      floor_y = floor((float) y * yscale);
      ceil_y = floor_y + 1;
      if (ceil_y >= src->h)
        ceil_y = floor_y;

      fraction_x = x * xscale - floor_x;
      fraction_y = y * yscale - floor_y;

      one_minus_x = 1.0 - fraction_x;
      one_minus_y = 1.0 - fraction_y;

      /* Grab their values:  */
      SDL_GetRGBA(getpixel(src, floor_x, floor_y), src->format,
                  &r1, &g1, &b1, &a1);
      SDL_GetRGBA(getpixel(src, ceil_x,  floor_y), src->format,
                  &r2, &g2, &b2, &a2);
      SDL_GetRGBA(getpixel(src, floor_x, ceil_y),  src->format,
                  &r3, &g3, &b3, &a3);
      SDL_GetRGBA(getpixel(src, ceil_x,  ceil_y),  src->format,
                  &r4, &g4, &b4, &a4);

      /* Create the weighted averages: */
      n1 = (one_minus_x * r1 + fraction_x * r2);
      n2 = (one_minus_x * r3 + fraction_x * r4);
      r = (one_minus_y * n1 + fraction_y * n2);

      n1 = (one_minus_x * g1 + fraction_x * g2);
      n2 = (one_minus_x * g3 + fraction_x * g4);
      g = (one_minus_y * n1 + fraction_y * n2);

      n1 = (one_minus_x * b1 + fraction_x * b2);
      n2 = (one_minus_x * b3 + fraction_x * b4);
      b = (one_minus_y * n1 + fraction_y * n2);

      n1 = (one_minus_x * a1 + fraction_x * a2);
      n2 = (one_minus_x * a3 + fraction_x * a4);
      a = (one_minus_y * n1 + fraction_y * n2);

      /* and put them into our new surface: */
      putpixel(s, x, y, SDL_MapRGBA(s->format, r, g, b, a));

    }
  }

  SDL_UnlockSurface(s);
  SDL_UnlockSurface(src);

  tmdprintf("\nLeaving zoom():\n");

  return s;
}

//FIXME: everything below is slightly modified code from pixels.c and would do
//       better to be included as such.

#if 0 //selectively omit from here to the end of file until pixels.c is in

















/*
  pixels.c

  For Tux Paint
  Pixel read/write functions

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)

  June 14, 2002 - February 17, 2006
  $Id: pixels.c,v 1.3 2006/08/27 21:00:55 wkendrick Exp $
*/

#include "pixels.h"
#include "compiler.h"
//#include "debug.h"

/* Draw a single pixel into the surface: */
void putpixel8(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

//  printf("putpixel8() called\n");

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (
      (((unsigned) x < (unsigned) surface->w)
       && ((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   x);		/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *p = pixel;
  }
}

/* Draw a single pixel into the surface: */
void putpixel16(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

//  printf("putpixel16() called\n");

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (
      (((unsigned) x < (unsigned) surface->w)
       && ((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 2));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *(Uint16 *) p = pixel;
  }
}

/* Draw a single pixel into the surface: */
void putpixel24(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

//  printf("putpixel24() called\n");

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (
      (((unsigned) x < (unsigned) surface->w)
       && ((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 3));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    }
    else
    {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }

  }
}

/* Draw a single pixel into the surface: */
void putpixel32(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
  Uint8 *p;

//  printf("putpixel32() called\n");

  /* Assuming the X/Y values are within the bounds of this surface... */
  if (
      (((unsigned) x < (unsigned) surface->w)
       && ((unsigned) y < (unsigned) surface->h)))
  {
    // Set a pointer to the exact location in memory of the pixel
    p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start: beginning of RAM */
		   (y * surface->pitch) +	/* Go down Y lines */
		   (x * 4));	/* Go in X pixels */


    /* Set the (correctly-sized) piece of data in the surface's RAM
     *          to the pixel value sent in: */

    *(Uint32 *) p = pixel;	// 32-bit display
  }
}

/* Get a pixel: */
Uint32 getpixel8(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

//  printf("getpixel8() called\n");

  /* get the X/Y values within the bounds of this surface */
  if ((unsigned) x < (unsigned) surface->w)
    x = (x < 0) ? 0 : surface->w - 1;
  if ((unsigned) y < (unsigned) surface->h)
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 x);		/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return (*p);
}

/* Get a pixel: */
Uint32 getpixel16(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

//  printf("getpixel16() called\n");

  /* get the X/Y values within the bounds of this surface */
  if ((unsigned) x < (unsigned) surface->w)
    x = (x < 0) ? 0 : surface->w - 1;
  if ((unsigned) y < (unsigned) surface->h)
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 2));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return (*(Uint16 *) p);
}

/* Get a pixel: */
Uint32 getpixel24(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;
  Uint32 pixel;

//  printf("getpixel24() called\n");

  /* get the X/Y values within the bounds of this surface */
  if ((unsigned) x < (unsigned) surface->w)
    x = (x < 0) ? 0 : surface->w - 1;
  if ((unsigned) y < (unsigned) surface->h)
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 3));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  /* Depending on the byte-order, it could be stored RGB or BGR! */

  if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    pixel = p[0] << 16 | p[1] << 8 | p[2];
  else
    pixel = p[0] | p[1] << 8 | p[2] << 16;

  return pixel;
}

/* Get a pixel: */
Uint32 getpixel32(SDL_Surface * surface, int x, int y)
{
  Uint8 *p;

//  printf("getpixel32() called\n");

  /* get the X/Y values within the bounds of this surface */
  if ((unsigned) x < (unsigned) surface->w)
    x = (x < 0) ? 0 : surface->w - 1;
  if ((unsigned) y < (unsigned) surface->h)
    y = (y < 0) ? 0 : surface->h - 1;

  /* Set a pointer to the exact location in memory of the pixel
     in question: */

  p = (Uint8 *) (((Uint8 *) surface->pixels) +	/* Start at top of RAM */
		 (y * surface->pitch) +	/* Go down Y lines */
		 (x * 4));	/* Go in X pixels */


  /* Return the correctly-sized piece of data containing the
   * pixel's value (an 8-bit palette value, or a 16-, 24- or 32-bit
   * RGB value) */

  return *(Uint32 *) p;		// 32-bit display
}

/* Function pointer arrays to allow correct function */
/* to be used according to colorspace:               */
void (*putpixels[]) (SDL_Surface *, int, int, Uint32) =
{
  putpixel8,
  putpixel8,
  putpixel16,
  putpixel24,
  putpixel32
};

Uint32(*getpixels[])(SDL_Surface *, int, int) =
{
  getpixel8,
  getpixel8,
  getpixel16,
  getpixel24,
  getpixel32
};

#endif
