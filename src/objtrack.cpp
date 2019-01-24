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
#include "objtrack.h"
#include "option.h"



#define PM_TAKEN 0x0001
#define PM_SEARCHR2_MAX (80.0f*80.0f)
#define PM_SEARCHR2_RATE (35.0f*35.0f)
#define PM_SEARCHR2_OFFSET (10.0f*10.0f)
#define PM_SEARCHR2_MARGIN (10.0f*10.0f)

struct pmodel
{
	uint32_t cap; //Capacity
	float * x0; //Position
	float * x1; //Velocity
	float * r;
	float * d; //TrackerDetected vector.
	uint32_t * t; //Number of tracked frames.
};


void draw_kp (cv::Mat &img, std::vector <cv::KeyPoint> const & kp)
{
	char text [10];
	uint32_t i = kp.size ();
	while (i--)
	{
		cv::Point2f p = kp [i].pt;
		float d = kp [i].size;
		snprintf (text, 10, "%i", kp [i].class_id);
		//cv::circle (img, p, d, cv::Scalar (255, 0, 255), 0.5);
		cv::drawMarker (img, p,  cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 4, 1);
		cv::putText (img, text, p + cv::Point2f (-10.0f, -10.0f), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (255, 0, 255), 1);
	}
}


void draw_pmodel 
(
	cv::Mat &img,
	struct pmodel * m
)
{
	char text [20];
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i * 2;
		float * x1 = m->x1 + i * 2;
		float * d = m->d + i * 2;
		float * r = m->r + i * 1;
		uint32_t * t = m->t + i * 1;
		//TRACE_F ("%i %f %f", i, x [0], x [1]);
		float r0 = MIN (sqrtf (r [0]), 100000.0f);
		snprintf (text, 20, "%u %u", i, t [0]);
		//snprintf (text, 10, "%u %f", i, e [0]);
		//snprintf (text, 10, "%u", i);
		cv::Point2f p0 (x0 [0], x0 [1]);
		cv::Point2f p1 (x1 [0], x1 [1]);
		cv::Point2f pd (d [0], d [1]);
		
		//TRACE_F ("%i %f %f", i, r0, sqrtf (PM_MAX_SEARCHR2));
		cv::circle      (img, p0, r0, cv::Scalar (255, 100, 150), 1);
		cv::circle      (img, p0, sqrtf (PM_SEARCHR2_MAX), cv::Scalar (255, 150, 100), 1);
		cv::drawMarker  (img, p0, cv::Scalar (0, 255, 255), 1, 10, 1);
		cv::putText     (img, text, p0+cv::Point2f (5,5), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
		cv::arrowedLine (img, p0, p0+p1*10.0f, cv::Scalar (0, 0, 255), 1, 8, 0, 0.1);
		cv::arrowedLine (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0, 0.1);
		//cv::line (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0);
	}
}


void pmodel_init (struct pmodel * m)
{
	m->x0 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x1 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->d  = (float *) calloc (m->cap, sizeof (float) * 2);
	m->r  = (float *) calloc (m->cap, sizeof (float));
	m->t  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
}


void pmodel_update (struct pmodel * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float * x0 = m->x0 + i*2;
		float * x1 = m->x1 + i*2;
		float * d = m->d + i*2;
		uint32_t * t = m->t + i*1;
		float mass = 20.0f;
		ASSERT (t [0] > 0);
		float k = (1.0f / mass) + (0.5f / t [0]);
		float x2 [2];
		v2f32_mus (x2, d, k);
		v2f32_add (x1, x1, x2);
		
		float x1mix [2];
		vf32_weight_ab (2, x1mix, x1, d, 0.8f);
		v2f32_add (x0, x0, x1mix);
	}
	//Slow down acc and vel.
	vf32_mus (m->cap*2, m->x1, m->x1, 0.98f);
}


float pmodel_shortest (struct pmodel * m, float const z [2])
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


void pmodel_lockon (struct pmodel * m, std::vector <cv::KeyPoint> & kp)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float    *     x0 = m->x0     + i * 2;
		float    *     x1 = m->x1     + i * 2;
		float    *      d = m->d      + i * 2;
		float    *      r = m->r      + i * 1;
		uint32_t *      t = m->t      + i * 1;
		
		uint32_t j = kp.size ();
		if (r [0] > PM_SEARCHR2_MAX)
		{
			while (j--)
			{
				if (kp [j].class_id != -1) {continue;}
				float * z0 = (float *) &kp [j].pt;
				if (pmodel_shortest (m, z0) < PM_SEARCHR2_MAX) {continue;}
			
				kp [j].class_id = i;
				vf32_cpy (2, x0, z0);
				vf32_set1 (2, x1, 0.0f);
				vf32_set1 (1, r, PM_SEARCHR2_OFFSET);
				vu32_set1 (1, t, 1);
			}
		}
		else
		{
			vu32_add1max (1, t, t, 1, UINT32_MAX);
			uint32_t n = 0;
			vf32_set1 (2, d, 0.0f);
			while (j--)
			{	
				float * z0 = (float *) &kp [j].pt;
				float zx0 [2];
				v2f32_sub (zx0, z0, x0);
				float l = v2f32_norm2 (zx0);
				if (l < r [0] && l < PM_SEARCHR2_MAX)
				{
					n ++;
					v2f32_add (d, d, zx0);
					kp [j].class_id = i;
				}
			}
			r [0] += PM_SEARCHR2_RATE;
			if (n == 0) {continue;}
			v2f32_mus (d, d, 1.0f / n);
			r [0] = v2f32_norm2 (d) + PM_SEARCHR2_MARGIN;
		}
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
						kp [0].class_id =  -1;
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
			pmodel_lockon (&pm, kp);
			pmodel_update (&pm);
			draw_kp (f1, kp);
			draw_pmodel (f1, &pm);
			//draw_trace (f2, &pm);
		}
		
		
		
		if (flags & (UPDATE_TRACKER | UPDATE_WORLD))
		{
			SDLCV_CopyTexture (texture, f1);
			SDL_RenderClear (renderer);
			SDL_RenderCopy (renderer, texture, NULL, NULL);
			SDL_RenderPresent (renderer);	
			f1.setTo (0);
		}
		
		if (flags & SEMIAUTO) {flags &= ~(UPDATE_WORLD | UPDATE_TRACKER);}
		
		usleep (50000);
	}
	
	return 0;
}

