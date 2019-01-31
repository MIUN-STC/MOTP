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
#include "draw.h"

#define QUIT     0x0001
#define UPDATE   0x0002
#define SEMIAUTO 0x0004



int main (int argc, char** argv)
{
	struct Option opt;
	opt_init (&opt, argc, argv);
	opt_print (&opt);
	if (opt.mode & OPT_HELP) {opt_help (&opt);return 0;}
	if (opt.src == NULL) {return 0;}

	cv::VideoCapture cap (opt.src);
	
	// Position of frame 
	//cap.set (cv::CAP_PROP_POS_FRAMES, (2*60*60+35*60 +10)*20);
	//cap.set (cv::CAP_PROP_POS_FRAMES, (5*60*60+48*60 + 30)*20);
	//cap.set (cv::CAP_PROP_POS_FRAMES, (12*60*60+25*60)*20);
	//Duration 5 or 10 minutes
	double w = cap.get (cv::CAP_PROP_FRAME_WIDTH);
	double h = cap.get (cv::CAP_PROP_FRAME_HEIGHT);
	
	cv::Mat mat_src      (h, w, CV_8UC3, cv::Scalar (0,0,0));
	cv::Mat mat_filtered (h, w, CV_8UC3, cv::Scalar (0,0,0));
	cv::Mat mat_trace    (h, w, CV_8UC3, cv::Scalar (0,0,0));
	cv::Mat mat_mask     (h, w, CV_8UC1);
	
	cv::Ptr<cv::BackgroundSubtractorKNN> bgfs;
	bgfs = cv::createBackgroundSubtractorKNN ();
	bgfs->setHistory (100);
	bgfs->setDist2Threshold (2000.0);
	bgfs->setDetectShadows (false);
	
	int morph_elem = cv::MorphShapes::MORPH_ELLIPSE;
	int morph_size = 4;
	cv::Mat element = cv::getStructuringElement 
	(
		morph_elem, 
		cv::Size (2*morph_size + 1, 2*morph_size+1), 
		cv::Point (morph_size, morph_size)
	);
	
	cv::Ptr<cv::SimpleBlobDetector> blobber;
	std::vector <cv::KeyPoint> kp;
	
	{
		cv::SimpleBlobDetector::Params blobparams;
		blobparams.minDistBetweenBlobs = 30;
		blobparams.minRepeatability = 2;
		blobparams.minThreshold = 200;
		blobparams.maxThreshold = 255;
		blobparams.filterByArea = false;
		blobparams.minArea = 40;
		blobparams.maxArea = 10000;
		blobparams.filterByColor = false;
		blobparams.blobColor = 255;
		blobparams.filterByCircularity = false;
		blobparams.minCircularity = 0.7;
		blobparams.maxCircularity = 1.0;
		blobparams.filterByConvexity = false;
		blobparams.minConvexity = 0.4;
		blobparams.filterByInertia = false;
		blobparams.minInertiaRatio = 0.3;
		blobber = cv::SimpleBlobDetector::create (blobparams);
	}
	
	

	ASSERT (SDL_Init (SDL_INIT_EVERYTHING) == 0);
	SDL_Window * window = NULL;
	SDL_Renderer * renderer = NULL;
	ASSERT (SDL_CreateWindowAndRenderer (w, h, SDL_WINDOW_RESIZABLE, &window, &renderer) == 0);
	
	uint32_t flags = 0;
	flags |= cap.read (mat_src) ? 0 : QUIT;
	uint32_t iframe = cap.get (cv::CAP_PROP_POS_FRAMES);
	SDL_Texture * texture = SDLCV_CreateTexture (renderer, mat_src);
	SDL_Event event;
	
	
	struct MOTP m;
	m.cap = 30;
	motp_init (&m);
	v2f32_random_wh (m.cap, m.x0, mat_src.cols, mat_src.rows);
	float * x0 = (float *) malloc (m.cap * sizeof (float) * 2);
	ASSERT (x0);
	
	struct MOTP_Files files;
	motp_openfiles (&files, opt.src);
	
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
					
					case SDLK_RIGHT:
					flags ^= UPDATE;
					flags |= SEMIAUTO;
					break;
					
					case SDLK_SPACE:
					flags ^= UPDATE;
					flags &= ~SEMIAUTO;
					break;
				}
				break;
				
	
			}
		}
		if (flags & QUIT) {break;}

		
		if (flags & UPDATE)
		{
			flags |= cap.read (mat_src) ? 0 : QUIT;
			if (flags & QUIT) {break;}
			
			cv::morphologyEx (mat_src, mat_filtered, cv::MORPH_GRADIENT, element);
			bgfs->apply (mat_filtered, mat_mask);
			cv::blur (mat_mask, mat_mask, cv::Size (3, 3));
			blobber->detect (mat_mask, kp);
	
			
			vf32_cpy (m.cap * 2, x0, m.x0);
			motp_search (&m, kp);
			motp_update (&m);
			motp_release (&m, kp);
			motp_expand (&m, kp);
			
			draw_trace (mat_trace, m.cap, m.id, m.t, m.u, x0, m.x0);
			draw_kp (mat_src, kp);
			draw_motp (mat_src, &m);
			mat_src += mat_trace;
			
			motp_writefiles (&files, &m);
			
			SDLCV_CopyTexture (texture, mat_src);
			SDL_RenderClear (renderer);
			SDL_RenderCopy (renderer, texture, NULL, NULL);
			SDL_RenderPresent (renderer);
		}
		
		
		
		SDL_Delay (1);
	}
	
	return 0;
}


