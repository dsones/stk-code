//  $Id: screen_manager.cpp 855 2006-11-17 01:50:37Z coz $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <cstdlib>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif
#include <SDL/SDL.h>
#include <assert.h>
#include "sdldrv.hpp"
#include "game_manager.hpp"
#include "gui/menu_manager.hpp"
#include "sound_manager.hpp"
#include "material_manager.hpp"
#include "race_manager.hpp"
#include "world.hpp"
#include "user_config.hpp"
#include "scene.hpp"
#include "history.hpp"

GameManager* game_manager = 0;

GameManager::GameManager() :
m_abort(false),
m_frame_count(0),
m_curr_time(m_prev_time),
m_prev_time(SDL_GetTicks())
{
}  // GameManager

//-----------------------------------------------------------------------------
GameManager::~GameManager()
{
}   // ~GameManager

//-----------------------------------------------------------------------------
void GameManager::run()
{
    const GLuint TITLE_SCREEN_TEXTURE = 
        material_manager->getMaterial("st_title_screen.rgb")->getIndex();
       
    bool music_on = false;
    m_curr_time = SDL_GetTicks();
    while(!m_abort)
    {
        sdl_input();

        m_prev_time = m_curr_time;
        m_curr_time = SDL_GetTicks();
		
		if (!music_on && !race_manager->raceIsActive())
		{
        	sound_manager->playMusic(stk_config->m_title_music);
		    music_on = true;
		}

        if (race_manager->raceIsActive())
        {
            music_on = false; 
            float dt = (m_curr_time - m_prev_time ) * 0.001f;
            scene->draw(dt);
            if ( world->getPhase() != World::LIMBO_PHASE)
            {
                world->update(dt);

                if(user_config->m_profile)
                {
                    m_frame_count++;
                    if (world->getTime()>user_config->m_profile)
                    {
                        //FIXME: SDL_GetTicks() includes the loading time,
                        //so the FPS will be skewed for now.
                        printf("Number of frames: %d time %f, Average FPS: %f\n",
                               m_frame_count, SDL_GetTicks() * 0.001,
                               (float)m_frame_count/(SDL_GetTicks() * 0.001));
                        if(!user_config->m_replay_history) history->Save();
                        std::exit(-2);
                    }
                }   // if m_profile
            }
        }
        else
        {
            glMatrixMode   ( GL_PROJECTION ) ;
            glLoadIdentity () ;
            glMatrixMode   ( GL_MODELVIEW ) ;
            glLoadIdentity () ;
            glDisable      ( GL_DEPTH_TEST ) ;
            glDisable      ( GL_LIGHTING   ) ;
            glDisable      ( GL_FOG        ) ;
            glDisable      ( GL_CULL_FACE  ) ;
            glDisable      ( GL_ALPHA_TEST ) ;
            glEnable       ( GL_TEXTURE_2D ) ;

            // On at least one platform the X server apparently gets overloaded
            // by the large texture, resulting in buffering of key events. This
            // results in the menu being very unresponsive/slow - it can sometimes
            // take (say) half a second before the menu reacts to a pressed key.
            // This is caused by X buffering the key events, delivering them
            // later (and sometimes even several at the same frame). This issue
            // could either be solved by a lazy drawing of the background picture
            // (i.e. draw the background only if something has changed) - which is
            // a lot of implementation work ... or by sleeping for a little while,
            // which apparently reduces the load for the X server, so that no
            // buffering is done --> all key events are handled in time.
        #if !defined(WIN32) && !defined(__CYGWIN__)
//                usleep(2000);
        #endif
            //Draw the splash screen
            glBindTexture(GL_TEXTURE_2D,TITLE_SCREEN_TEXTURE);

            glBegin ( GL_QUADS ) ;
            glColor3f   (1, 1, 1 ) ;
            glTexCoord2f(0, 0); glVertex2i(-1, -1);
            glTexCoord2f(1, 0); glVertex2i( 1, -1);
            glTexCoord2f(1, 1); glVertex2i( 1,  1);
            glTexCoord2f(0, 1); glVertex2i(-1,  1);
            glEnd () ;
        }

        menu_manager->update();
        sound_manager->update();

        glFlush();
        SDL_GL_SwapBuffers();
    }  // while !m_exit
}   // run

//-----------------------------------------------------------------------------
void GameManager::abort()
{
    m_abort = true;
}

/* EOF */
