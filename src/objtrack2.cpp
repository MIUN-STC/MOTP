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
	uint32_t cap; //Capacity
	float * x0; //Position
	float * x1; //Velocity
	
	uint32_t * u; //Number of untracked frames.
	uint32_t * t; //Number of tracked frames.
	float * r;
	
	float sr0; //Search radius start.
	float sr1; //Search radius growth rate.
	
	float * d; //TrackerDetected vector.
	float * e;
};


void draw_kp (cv::Mat &img, std::vector <cv::KeyPoint> const & kp)
{
	char text [10];
	uint32_t i = kp.size ();
	while (i--)
	{
		cv::Point2f p = kp [i].pt;
		float d = kp [i].size;
		snprintf (text, 10, "%u", i);
		//cv::circle (img, p, d, cv::Scalar (255, 0, 255), 0.5);
		cv::drawMarker (img, p,  cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 20, 1);
		//cv::putText (img, text, p + cv::Point2f (-10.0f, -10.0f), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (255, 0, 255), 1);
		//cv::putText (img, text, p + cv::Point2f (-10.0f, -10.0f), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (255, 0, 255), 1);
		//TRACE_F ("%i %f %f", i, p.x, p.y);
	}
}


void draw_pmodel 
(
	cv::Mat &img,
	struct pmodel * m
)
{
	char text [10];
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i * 2;
		float * x1 = m->x1 + i * 2;
		float * d = m->d + i * 2;
		float * e = m->e + i * 1;
		uint32_t * u = m->u + i * 1;
		uint32_t * t = m->t + i * 1;
		//TRACE_F ("%i %f %f", i, x [0], x [1]);
		snprintf (text, 10, "%u %u", i, m->u [i]);
		//snprintf (text, 10, "%u %f", i, e [0]);
		//snprintf (text, 10, "%u", i);
		cv::Point2f p0 (x0 [0], x0 [1]);
		cv::Point2f p1 (x1 [0], x1 [1]);
		cv::Point2f pd (d [0], d [1]);
		float r = m->sr0 + u [0] * m->sr1;
		//float r = m->sr0;
		//cv::circle      (img, p0, MIN (r, 10000.0f), cv::Scalar (255, 150, 150), 1);
		cv::drawMarker  (img, p0, cv::Scalar (0, 255, 255), 1, 10, 1);
		cv::putText     (img, text, p0+cv::Point2f (5,5), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
		cv::arrowedLine (img, p0, p0+p1*10.0f, cv::Scalar (0, 0, 255), 1, 8, 0, 0.1);
		cv::arrowedLine (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0, 0.1);
		//cv::line (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0);
	}
}


void draw_trace 
(
	cv::Mat &img,
	struct pmodel * m
)
{
	char text [10];
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i * 2;
		float * x1 = m->x1 + i * 2;
		float * d = m->d + i * 2;
		float * e = m->e + i * 1;
		uint32_t * u = m->u + i * 1;
		uint32_t * t = m->t + i * 1;
	}
}


void pmodel_init (struct pmodel * m)
{
	m->x0 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x1 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->d  = (float *) calloc (m->cap, sizeof (float) * 2);
	m->e  = (float *) calloc (m->cap, sizeof (float));
	m->r  = (float *) calloc (m->cap, sizeof (float));
	m->u  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	m->t  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	vu32_set1 (m->cap, m->u, UINT32_MAX);
	//vu32_set1 (m->cap, m->persistence, UINT32_MAX);
	m->sr0 = 0.0f;
	m->sr1 = 20.0f;
}


void pmodel_update (struct pmodel * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i*2;
		float * x1 = m->x1 + i*2;
		float * d = m->d + i*2;
		float * e = m->e + i*1;
		uint32_t * t = m->t + i*1;
		uint32_t * u = m->u + i*1;
		if (u [0] != UINT32_MAX) {u [0] ++;}
		float mass = 18.0f;
		ASSERT (u [0] > 0);
		//float k = (1.0f / mass) * (1.0f / u [0]) + 0.0001f * sqrt (e [0]);
		float k = (1.0f / mass) * (1.0f / u [0]) + (1.0f / t [0]);
		float x2 [2];
		v2f32_mus (x2, d, k);
		v2f32_add (x1, x1, x2);
		v2f32_add (x0, x0, x1);
	}
	//Slow down acc and vel.
	vf32_mus (m->cap*2, m->x1, m->x1, 0.98f);
}


float pmodel_shortest (struct pmodel * m, float z [2])
{
	float lmin = FLT_MAX;
	uint32_t i = m->cap;
	while (i--)
	{
		float * a = m->x0 + i * 2;
		float d [2];
		v2f32_sub (d, a, z);
		float l = v2f32_norm2 (d);
		if (l < lmin) {lmin = l;}
	}
	return lmin;
}


void pmodel_lockon (struct pmodel * m, std::vector <cv::KeyPoint> const & kp)
{
	vf32_set1 (m->cap * 2, m->d, 0.0f);
	vu32_add1max (m->cap, m->u, m->u, 1, UINT32_MAX);
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
			uint32_t * u = m->u + i * 1;
			if (u [0] == 0) {continue;}
			float d [2];
			v2f32_sub (d, z0, x0);
			float l = v2f32_norm2 (d);
			float r = m->sr0 + u [0] * m->sr1;
			if (l > (r*r) && u [0] < 10) {continue;}
			if (l > lmin) {continue;}
			imin = i;
			lmin = l;
			dmin [0] = d [0];
			dmin [1] = d [1];
		}
		if (imin > m->cap) {continue;}
		float *   x0 = m->x0 + imin * 2;
		float *   x1 = m->x1 + imin * 2;
		float *    d = m->d  + imin * 2;
		float *    e = m->e  + imin * 1;
		uint32_t * u = m->u  + imin * 1;
		uint32_t * t = m->t  + imin * 1;
		if (u [0] < 10)
		{
			vf32_weight_ab (2, x0, x0, z0, 0.8f);
			vf32_cpy (2, d, dmin);
			e [0] += lmin;
			t [0] += 1;
			if (lmin < (m->sr1*m->sr1)){e [0] = 0.0f;}
		}
		else if (pmodel_shortest (m, z0) > 40*40)
		{
			vf32_cpy (2, x0, z0);
			vf32_set1 (2, x1, 0.0f);
			vf32_set1 (1, e, 0.0f);
			vu32_set1 (1, t, 1);
		}
		vu32_set1 (1, u, 0);
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
	
	// Position of frame 
	//cap.set (cv::CAP_PROP_POS_FRAMES, (2*60*60+35*60 +10)*20);
	//cap.set (cv::CAP_PROP_POS_FRAMES, (5*60*60+48*60 + 30)*20);
	//cap.set (cv::CAP_PROP_POS_FRAMES, (12*60*60+25*60)*20);
	//Duration 5 or 10 minutes
	double w = cap.get (cv::CAP_PROP_FRAME_WIDTH);
	double h = cap.get (cv::CAP_PROP_FRAME_HEIGHT);
	
	cv::Mat f0 (h, w, CV_8UC3);
	cv::Mat f1 (h, w, CV_8UC3);
	cv::Mat f2 (h, w, CV_8UC3);
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
	pm.cap = 30;
	pmodel_init (&pm);
	v2f32_random_wh (pm.cap, pm.x0, f0.cols, f0.rows);
	
	
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
					
					case SDLK_u:
					flags ^= UPDATE_TRACKER;
					flags |= SEMIAUTO;
					break;
					
					case SDLK_r:
					kp.resize (0);
					flags ^= UPDATE_TRACKER;
					flags |= SEMIAUTO;
					break;
					
					case SDLK_RIGHT:
					flags ^= UPDATE_WORLD | UPDATE_TRACKER;
					flags |= SEMIAUTO;
					break;
					
					case SDLK_SPACE:
					flags ^= UPDATE_WORLD | UPDATE_TRACKER;
					flags &= ~SEMIAUTO;
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
						float * x = (float *) &kp [0].pt;
						x [0] = (event.motion.x * f0.cols) / w;
						x [1] = (event.motion.y * f0.rows) / h;
					}
					
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
						cv::Point2f p;
						p.x = (event.motion.x * f0.cols) / w;
						p.y = (event.motion.y * f0.rows) / h;
						kp.push_back (cv::KeyPoint (p, 0.0f));
					}	
		
					flags ^= UPDATE_TRACKER;
					flags |= SEMIAUTO;
					//TRACE_F ("%i", kp.size ());
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
	
		
		if (flags & UPDATE_WORLD)
		{
			flags |= cap.read (f0) ? 0 : QUIT;
			if (flags & QUIT) {break;}
			f0.copyTo (f1);
			cv::morphologyEx (f0, f1, cv::MORPH_GRADIENT, element);
			bgfs->apply (f1, mask);
			cv::blur (mask, mask, cv::Size (3, 3));
			blobber->detect (mask, kp);
			f0.copyTo (f1);
		}
		

		if (flags & UPDATE_TRACKER)
		{
			pmodel_update (&pm);
			draw_pmodel (f1, &pm);
			draw_trace (f2, &pm);
			pmodel_lockon (&pm, kp);
		}
		
		
		
		//if (flags & (UPDATE_TRACKER | UPDATE_WORLD))
		{
			draw_kp (f1, kp);
			SDLCV_CopyTexture (texture, f1);
			SDL_RenderClear (renderer);
			SDL_RenderCopy (renderer, texture, NULL, NULL);
			SDL_RenderPresent (renderer);	
			//f1.setTo (0);
		}
		
		if (flags & SEMIAUTO) {flags &= ~(UPDATE_WORLD | UPDATE_TRACKER);}
		
		usleep (50000);
	}
	
	return 0;
}

