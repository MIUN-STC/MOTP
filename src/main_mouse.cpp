#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utility.hpp>

#include <opencv2/xfeatures2d.hpp>

//Common
#include <csc/SDLGL.h> //SDL OpenCV bridge functions.
#include <csc/SDLCV.h> //SDL OpenGL bridge functions.
#include <csc/debug.h> //Debugging functions.
#include <csc/v.h> //Vector operations.
#include <csc/v2.h> //Vector operations.

//Application
#include "motp.h"
#include "draw.h"


#define QUIT 0x0001
#define UPDATE 0x0004
#define SEMIAUTO 0x0008



int main (int argc, char** argv)
{
	double w = 800;
	double h = 600;
	cv::Mat mat0 (h, w, CV_8UC3, cv::Scalar (0,0,0));
	cv::Mat mat_trace (h, w, CV_8UC3, cv::Scalar (0,0,0));

	ASSERT (SDL_Init (SDL_INIT_EVERYTHING) == 0);
	SDL_Window * window = NULL;
	SDL_Renderer * renderer = NULL;
	ASSERT (SDL_CreateWindowAndRenderer (w, h, SDL_WINDOW_RESIZABLE, &window, &renderer) == 0);
	
	uint32_t flags = 0;
	SDL_Texture * texture = SDLCV_CreateTexture (renderer, mat0);
	SDL_Event event;
	
	
	struct MOTP m;
	m.cap = 2;
	motp_init (&m);
	v2f32_random_wh (m.cap, m.x0, mat0.cols, mat0.rows);
	float * x0 = (float *) malloc (m.cap * sizeof (float) * 2);
	ASSERT (x0);
	
	
	std::vector <cv::KeyPoint> kp;
	
	while (1)
	{
		if (flags & SEMIAUTO) {flags &= ~(UPDATE);}
		
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				flags |= QUIT;
				break;
				
				case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					flags |= QUIT;
					break;
					
					case SDLK_SPACE:
					flags ^= UPDATE;
					flags &= ~SEMIAUTO;
					break;
					
					case SDLK_RIGHT:
					flags ^= UPDATE;
					flags |= SEMIAUTO;
					break;
				}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
				{
					//Convert the mouse position to OpenCV mat/image (f0) coordinate system.
					int w;
					int h;
					SDL_GetWindowSize (window, &w, &h);
					if (event.button.button == SDL_BUTTON_LEFT && kp.size () > 0)
					{
						kp [0].class_id =  -1;
						float * x = (float *) &kp [0].pt;
						x [0] = (event.motion.x * mat0.cols) / w;
						x [1] = (event.motion.y * mat0.rows) / h;
					}
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						cv::Point2f p;
						p.x = (event.motion.x * mat0.cols) / w;
						p.y = (event.motion.y * mat0.rows) / h;
						kp.push_back (cv::KeyPoint (p, 0.0f));
					}	
					flags ^= UPDATE;
					flags |= SEMIAUTO;
				}
				break;
				
				case SDL_MOUSEBUTTONUP:
				break;
					
				case SDL_MOUSEMOTION:
				break;
			}
		}
		if (flags & QUIT) {break;}
		
		if (flags & UPDATE)
		{
			vf32_cpy (m.cap * 2, x0, m.x0);
			motp_search (&m, kp);
			motp_update (&m);
			motp_release (&m, kp);
			motp_expand (&m, kp);
			
			draw_trace (mat_trace, m.cap, m.id, m.t, m.u, x0, m.x0);
			draw_kp (mat0, kp);
			draw_motp (mat0, &m);
			SDLCV_CopyTexture (texture, mat0);
			SDL_RenderClear (renderer);
			SDL_RenderCopy (renderer, texture, NULL, NULL);
			SDL_RenderPresent (renderer);	
			mat0.setTo (0);
		}
		
		
		
		SDL_Delay (100);
	}
	
	return 0;
}

