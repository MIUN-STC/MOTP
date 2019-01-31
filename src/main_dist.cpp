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
#include "option.h"
#include "motp.h"

#define QUIT 0x0001



void dist (float y [2], float x [2], uint32_t n, float z [])
{
	float dmin [2];
	float lmin= FLT_MAX;
	float ysum [2] = {0.0f, 0.0f};
	uint32_t i = n;
	while (i--)
	{
		//TRACE_F ("%f %f", d [0], d [1]);
		float zx [2];
		vf32_sub (2, zx, z, x);
		float l = vf32_norm2 (2, zx);
		if (l < lmin)
		{
			vf32_cpy (2, dmin, zx);
			lmin = l;
		}
		vf32_mus (2, zx, zx, 1.0f / (1.0f + l));
		vf32_add (2, ysum, ysum, zx);
		z += 2;
	}
	vf32_mus (2, y, ysum, vf32_norm (2, dmin) / vf32_norm (2, ysum));
}





int main (int argc, char** argv)
{
	double w = 1920;
	double h = 1080;
	ASSERT (SDL_Init (SDL_INIT_EVERYTHING) == 0);
	SDL_Window * window = NULL;
	SDL_Renderer * renderer = NULL;
	ASSERT (SDL_CreateWindowAndRenderer (w, h, SDL_WINDOW_RESIZABLE, &window, &renderer) == 0);
	SDL_Event event;
	uint32_t flags = 0;
	cv::Mat mat0 (h, w, CV_8UC3);
	SDL_Texture * texture = SDLCV_CreateTexture (renderer, mat0);
	float z [2*4] = {300.0f, 300.0f, 350.0f, 300.0f, 600.0f, 200.0f, 600.0f, 400.0f};
	float x [2] = {10.0f, 10.0f};
	float y [2];
	while (1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				TRACE ("SDL_QUIT");
				flags |= QUIT;
				break;
				
				case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					TRACE ("SDLK_ESCAPE");
					flags |= QUIT;
					break;
				}
				case SDL_MOUSEBUTTONDOWN:
				{
					int w;
					int h;
					SDL_GetWindowSize (window, &w, &h);
					if (event.button.button == SDL_BUTTON_LEFT)
					{
						x [0] = (event.motion.x * mat0.cols) / w;
						x [1] = (event.motion.y * mat0.rows) / h;
						dist (y, x, 4, z);
						//f0.setTo (0);
						cv::drawMarker (mat0, cv::Point2f (z[0], z [1]), cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 8, 2);
						cv::drawMarker (mat0, cv::Point2f (z[2], z [3]), cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 8, 2);
						cv::drawMarker (mat0, cv::Point2f (z[4], z [5]), cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 8, 2);
						cv::drawMarker (mat0, cv::Point2f (z[6], z [7]), cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 8, 2);
						cv::drawMarker (mat0, cv::Point2f (x[0], x [1]), cv::Scalar (255, 255, 0), cv::MARKER_CROSS, 8, 2);
						cv::drawMarker (mat0, cv::Point2f (x[0], x [1]) + cv::Point2f (y[0], y [1]), cv::Scalar (0, 255, 255), cv::MARKER_CROSS, 4, 1);
						cv::arrowedLine (mat0, cv::Point2f (x[0], x [1]), cv::Point2f (x[0], x [1]) + cv::Point2f (y[0], y [1]), cv::Scalar (0, 0, 255), 1, 8, 0, 0.1);
					}
				}
				break;
				
				case SDL_MOUSEBUTTONUP:
				break;
					
				case SDL_MOUSEMOTION:
				break;
			}	
		}
		if (flags & QUIT) {break;}
		SDLCV_CopyTexture (texture, mat0);
		SDL_RenderClear (renderer);
		SDL_RenderCopy (renderer, texture, NULL, NULL);
		SDL_RenderPresent (renderer);
	}
	
	return 0;
}

