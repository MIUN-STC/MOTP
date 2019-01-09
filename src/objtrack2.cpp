#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utility.hpp>

#include <opencv2/xfeatures2d.hpp>

//Common
#include "SDLGL.h" //SDL OpenCV bridge functions.
#include "SDLCV.h" //SDL OpenGL bridge functions.
#include "debug.h" //Debugging functions.
#include "v2.h" //Vector operations.

//Application
#include "objtrack2.h"
#include "option.h"






struct pmodel
{
	uint32_t cap;
	float * x0; //Pos
	float * x1; //Vel
	float * x2; //Acc
	uint32_t * u;
	float sr0;
	float sr1;
};


void draw_kp (cv::Mat &img, std::vector <cv::KeyPoint> const & kp)
{
	uint32_t i = kp.size ();
	while (i--)
	{
		cv::Point2f p = kp [i].pt;
		float d = kp [i].size;
		cv::circle (img, p, d, cv::Scalar (255, 0, 255), 0.5);
		cv::drawMarker (img, p,  cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 100, 1);
		//TRACE_F ("%i %f %f", i, p.x, p.y);
	}
}


void draw_pmodel 
(
	cv::Mat &img,
	struct pmodel * m
)
{
	char text [100];
	uint32_t i = m->cap;
	while (i--)
	{
		float * x = m->x0 + i * 2;
		uint32_t * u = m->u + i * 1;
		//snprintf (buf, 10, "%u %u", i, m->p [i]);
		snprintf (text, 10, "%u", i);
		cv::Point2f p (x [0], x [1]);
		float r = m->sr0 + u [0] * m->sr1;
		cv::circle (img, p, MIN (r, 10000.0f), cv::Scalar (60, 80, 234), 1);
		cv::drawMarker (img, p, cv::Scalar (255, 66, 11), 1, 100, 1);
		cv::putText (img, text, p, CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
	}
}


void pmodel_init (struct pmodel * m)
{
	m->x0 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x1 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x2 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->u  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	vu32_set1 (m->cap, m->u, UINT32_MAX);
	//vu32_set1 (m->cap, m->persistence, UINT32_MAX);
	m->sr0 = 50.0f;
	m->sr1 = 10.0f;
}


void pmodel_update (struct pmodel * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i*2;
		float * x1 = m->x1 + i*2;
		float * x2 = m->x2 + i*2;
		
		v2f32_add (x0, x0, x1);
		v2f32_add (x1, x1, x2);
		
		//TRACE_F ("%f %f", x2 [0], x2 [1]);
	}
	
	//Slow down acc and vel.
	vf32_mus (m->cap*2, m->x1, m->x1, 0.98f);
	vf32_mus (m->cap*2, m->x2, m->x2, 0.90f);
}


void pmodel_lockon_simple1 (struct pmodel * m, std::vector <cv::KeyPoint> const & kp)
{
	uint32_t j = kp.size ();
	while (j--)
	{
		float dmin [2];
		float lmin = FLT_MAX;
		uint32_t imin = UINT32_MAX;
		float * z0 = (float *) &kp [j].pt;
		uint32_t i = m->cap;
		while (i--)
		{
			float * x0 = m->x0 + i * 2;
			v2f32_sub (dmin, x0, z0);
			float l = v2f32_norm2 (dmin);
			if (l < 400.0f)
			if (l < lmin)
			{
				imin = i;
			}
		}
		if (imin > m->cap) {continue;}
		float * x0 = m->x0 + imin * 2;
		x0 [0] = z0 [0];
		x0 [1] = z0 [1];
	}
}


void pmodel_lockon (struct pmodel * m, std::vector <cv::KeyPoint> const & kp)
{
	uint32_t j = kp.size ();
	while (j--)
	{
		float dmin [2];
		float lmin = FLT_MAX;
		uint32_t imin = UINT32_MAX;
		float * z0 = (float *) &kp [j].pt;
		uint32_t i = m->cap;
		while (i--)
		{
			float * x0 = m->x0 + i * 2;
			float * x1 = m->x1 + i * 2;
			float * x2 = m->x2 + i * 2;
			uint32_t * u = m->u + i * 1;
			//TRACE_F ("%f %f", z0 [0], z0 [1]);
			v2f32_sub (dmin, x0, z0);
			float l = v2f32_norm2 (dmin);
			float r = m->sr0 + u [0] * m->sr1;
			//float r = m->sr0;
			if (l > (r*r)) {continue;}
			if (l < lmin)
			{
				imin = i;
			}
		}
		TRACE_F ("%i", imin);
		if (imin > m->cap) {continue;}
		float * x0 = m->x0 + imin * 2;
		float * x1 = m->x1 + imin * 2;
		float * x2 = m->x2 + imin * 2;
		uint32_t * u = m->u + imin * 1;
		x0 [0] = z0 [0];
		x0 [1] = z0 [1];
		u [0] = 0;
		
			
		//if (m->u [imin] > 10)
		{
			//m->u [imin] = 0;
		}
		//else
		{
		}
		
		
		//float d = v2f32_dist2 (x0);
	}
}






int main (int argc, char** argv)
{
	struct Option opt;
	opt_init (&opt, argc, argv);
	opt_print (&opt);
	if (opt.mode & OPT_HELP) {opt_help (&opt);return 0;}
	if (opt.src == NULL) {return 0;}

	cv::VideoCapture cap (opt.src);
	double w = cap.get (cv::CAP_PROP_FRAME_WIDTH);
	double h = cap.get (cv::CAP_PROP_FRAME_HEIGHT);
	
	cv::Mat f0 (h, w, CV_8UC3);
	cv::Mat f1 (h, w, CV_8UC3);
	cv::Mat mask (h, w, CV_8UC1);
	
	cv::Ptr<cv::BackgroundSubtractorKNN> bgfs;
	bgfs = cv::createBackgroundSubtractorKNN ();
	bgfs->setHistory (100);
	bgfs->setDist2Threshold (2000.0);
	bgfs->setDetectShadows (false);
	
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
	
	
	int morph_elem = cv::MorphShapes::MORPH_ELLIPSE;
	int morph_size = 4;
	cv::Mat element = cv::getStructuringElement (morph_elem, cv::Size (2*morph_size + 1, 2*morph_size+1), cv::Point (morph_size, morph_size));
	
	ASSERT (SDL_Init (SDL_INIT_EVERYTHING) == 0);
	SDL_Window * window = NULL;
	SDL_Renderer * renderer = NULL;
	ASSERT (SDL_CreateWindowAndRenderer (800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer) == 0);
	
	uint32_t flags = 0;
	flags |= cap.read (f0) ? 0 : QUIT;
	uint32_t iframe = cap.get (cv::CAP_PROP_POS_FRAMES);
	SDL_Texture * texture = SDLCV_CreateTexture (renderer, f0);
	SDL_Event event;
	
	
	struct pmodel pm;
	pm.cap = 10;
	pmodel_init (&pm);
	v2f32_random_wh (pm.cap, pm.x0, f0.rows, f0.cols);
	v2f32_random (pm.cap, pm.x2);
	
	
	while (1)
	{
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

					case SDLK_p:
					flags ^= PAUSE;
					break;
					
					case SDLK_k:
					break;
					
					case SDLK_RIGHT:
					flags ^= UPDATE_WORLD | UPDATE_TRACKER;
					flags |= SEMIAUTO;
					break;
					
					case SDLK_SPACE:
					break;
				}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
				{
					//Convert the mouse position to OpenCV mat/image (f0) coordinate system.
					int w;
					int h;
					SDL_GetWindowSize (window, &w, &h);
					//cv::Point2f p;
					//p.x = (event.motion.x * f0.cols) / w;
					//p.y = (event.motion.y * f0.rows) / h;
					//kp.push_back (cv::KeyPoint (p, 0.0f));
					kp.resize (1);
					float * x = (float *) &kp [0].pt;
					x [0] = (event.motion.x * f0.cols) / w;
					x [1] = (event.motion.y * f0.rows) / h;
					flags ^= UPDATE_TRACKER;
					flags |= SEMIAUTO;
					TRACE_F ("%i", kp.size ());
				}

				break;
				
				case SDL_MOUSEBUTTONUP:
				break;
					
				case SDL_MOUSEMOTION:
				break;
			}
		}
		if (flags & QUIT) {break;}
		if (flags & PAUSE) {continue;}
	
		
		draw_kp (f1, kp);
		draw_pmodel (f1, &pm);
	
		
		if (flags & UPDATE_WORLD)
		{
			flags |= cap.read (f0) ? 0 : QUIT;
			if (flags & QUIT) {break;}
			f0.copyTo (f1);
			cv::morphologyEx (f0, f1, cv::MORPH_GRADIENT, element);
			bgfs->apply (f1, mask);
			cv::blur (mask, mask, cv::Size (3, 3));
			blobber->detect (mask, kp);
			//f0.copyTo (f1);
		}
		
		if (flags & UPDATE_TRACKER)
		{
			pmodel_lockon (&pm, kp);
			//pmodel_update (&pm);
		}
		
		
		if (flags & SEMIAUTO) {flags &= ~(UPDATE_WORLD | UPDATE_TRACKER);}
		
		

		
		
		SDLCV_CopyTexture (texture, f1);
		f1.setTo (0);
		SDL_RenderClear (renderer);
		SDL_RenderCopy (renderer, texture, NULL, NULL);
		SDL_RenderPresent (renderer);
	}
	
	return 0;
}

