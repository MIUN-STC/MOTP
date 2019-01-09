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


//Convert to keypoints to normalized data.
void kp_to_v2f32 
(
	uint32_t * n, //Input buffer size. Output used buffer size.
	float p [], //2D vectors. Blob position.
	float D [], //1D vectors. Blob diameter.
	std::vector <cv::KeyPoint> const & kp
)
{
	uint32_t i;
	(*n) = MIN ((*n), kp.size ());
	for (i = 0; i < (*n); ++i)
	{
		p [0] = kp [i].pt.y;
		p [1] = kp [i].pt.x;
		p += 2; //Position has dim=2.
		D [0] = kp [i].size;
		D += 1; //Diameter has dim=1.
	}
}






struct particle_model
{
	uint32_t cap;
	uint32_t n;
	
	float * pos; //Position vector
	float * vel; //Velocity vector
	float * D; //Blob diameter
	float * Davg; //Blob average diameter
	uint32_t * persistence;
	float * A; //Angle of velocity
	uint32_t * duration;
};


void pm_init (struct particle_model * m)
{
	m->pos         = (float *) calloc (m->cap, sizeof (float) * 2);
	m->vel         = (float *) calloc (m->cap, sizeof (float) * 2);
	m->D           = (float *) calloc (m->cap, sizeof (float));
	m->Davg        = (float *) calloc (m->cap, sizeof (float));
	m->A           = (float *) calloc (m->cap, sizeof (float));
	m->persistence = (uint32_t *) calloc (m->cap, sizeof (uint32_t));
	m->duration    = (uint32_t *) calloc (m->cap, sizeof (uint32_t));
	
	vu32_set1 (m->cap, m->persistence, UINT32_MAX);
}


void draw_circle 
(
	cv::Mat &img,
	uint32_t n, 
	float pos [], 
	float D [],  
	uint8_t r, 
	uint8_t g, 
	uint8_t b
)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		cv::Point2f p (pos [i*2 + 1], pos [i*2 + 0]);
		cv::circle (img, p, D [i], cv::Scalar (r, g, b), 1);
	}
}


void draw_blob 
(
	cv::Mat &img,
	uint32_t n, 
	float pos [], 
	float D [],  
	uint8_t r, 
	uint8_t g, 
	uint8_t b
)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		cv::Point2f p (pos [i*2 + 1], pos [i*2 + 0]);
		cv::circle (img, p, 3, cv::Scalar (r, g, b), -1);
		//cv::circle (img, p, D [i], cv::Scalar (r, g, b), 1);
	}
}


void draw_id 
(
	cv::Mat &img,
	uint32_t n, 
	float pos [], 
	float vel [], 
	float D [], 
	uint32_t pers [],
	uint32_t duration [],
	uint32_t persistance,
	float proximity,
	float grow_cone
)
{
	char buf [10];
	for (uint32_t i = 0; i < n; ++i)
	{
		//snprintf (buf, 10, "%i", pers [i]);
		cv::Point2f p (pos [i*2 + 1], pos [i*2 + 0]);
		cv::Point2f v (vel [i*2 + 1], vel [i*2 + 0]);
		cv::Point2f p1 = p + v*10;
		
		//if (pers [i] < persistance)
		{
			snprintf (buf, 10, "%u %u", i, pers [i]);
			float proxy =  proximity + (pers [i] * grow_cone);
			ASSERT (proxy >= 0);
			//cv::circle (img, p, D [i], cv::Scalar (255, 0, 255), 1);
			cv::circle (img, p, MIN (proxy, 1000), cv::Scalar (255, 0, 255), 0.5);
			cv::circle (img, p, 3, cv::Scalar (255, 0, 255), -1);
			cv::putText (img, buf, p, CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
			cv::arrowedLine (img, p, p1, cv::Scalar (0, 255, 255), 1, 8, 0, 0.1);
		}
		
	}
}



void pm_simulate 
(
	uint32_t n, 
	uint32_t duration [],
	uint32_t persistence [],
	float pos [],
	float vel [],
	
	uint32_t persistence_max
)
{
	while (n--)
	{
		persistence [0] += 1;
		if (persistence [0] > persistence_max) 
		{
			duration [0] = 0;
			vel [0] = 0;
			vel [1] = 0;
		}
		v2f32_add (pos, pos, vel);
		v2f32_mus (vel, vel, 0.96f);
		
		duration += 1;
		persistence += 1;
		pos += 2;
		vel += 2;
	}
}


void pm_shortest 
(
	uint32_t n,
	float const pos [],
	uint32_t persistence [],

	uint32_t * imin,
	float * dmin,
	
	float const pos_ref [2],
	float proximity,
	float grow_cone,
	float persistence_max
)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		float d = v2f32_dist2 (pos_ref, pos);
		float prox = proximity + ((float)persistence [0] * grow_cone);
		if (persistence [0] > persistence_max)
		{
			(*dmin) = d;
			(*imin) = i;
			//persistence [0] = 0;
		}
		if (d < (*dmin) && d < (prox * prox))
		{
			(*dmin) = d;
			(*imin) = i;
		}
		
		persistence += 1;
		pos += 2;
	}
}


void pm_update 
(
	uint32_t n0,
	float pos0 [],
	float vel0 [],
	float D0 [],
	float Davg0 [],
	uint32_t pers0 [],
	uint32_t duration0 [],

	uint32_t n1,
	float const pos1 [],
	float const D1 [],
	
	uint32_t persistence,
	float proximity,
	float grow_cone
)
{
	for (uint32_t i1 = 0; i1 < n1; ++i1)
	{
		float dmin = FLT_MAX;
		uint32_t imin = UINT32_MAX;
		pm_shortest 
		(
			n0, 
			pos0, 
			pers0, 
			&imin, 
			&dmin, 
			pos1 + (i1*2), 
			proximity, 
			grow_cone, 
			persistence
		);
		//TRACE_F ("%f", dmin);
		if (imin == UINT32_MAX) {continue;}
		
		//TRACE_F ("%u %u %f %f", imin, pers0 [imin], pos0 [imin*2+0], pos0 [imin*2+1]);
		
		if (pers0 [imin] <= 1)
		{
			float delta [2];
			v2f32_sub (delta, pos1 + (i1*2), pos0 + (imin*2));
			vel0 [imin*2 + 0] = (0.9f * vel0 [imin*2 + 0]) + (0.3f * delta [0]);
			vel0 [imin*2 + 1] = (0.9f * vel0 [imin*2 + 1]) + (0.3f * delta [1]);
		}
		
		vf32_cpy (2, pos0 + (imin*2), pos1 + (i1*2));

		Davg0 [imin] = (1.0f * D1 [i1]) + (0.0f * Davg0 [imin]);
		//pers0 [imin] = 0;
		//pers0 [imin] = MIN (pers0 [imin], persistence);
		pers0 [imin] = pers0 [imin] >> 2;
		duration0 [imin] ++;
		//A0 [imin] = atan2f (deltamin [1], deltamin [0]);	
	}
	
	pm_simulate (n0, duration0, pers0, pos0, vel0, persistence);
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
	
	cv::Ptr<cv::SimpleBlobDetector> blobber = cv::SimpleBlobDetector::create (blobparams);
	std::vector <cv::KeyPoint> kp;
	
	int morph_elem = cv::MorphShapes::MORPH_ELLIPSE;
	int morph_size = 4;
	cv::Mat element = cv::getStructuringElement (morph_elem, cv::Size (2*morph_size + 1, 2*morph_size+1), cv::Point (morph_size, morph_size) );
	
	
	struct particle_model sm;
	struct particle_model pm;
	sm.cap = 100;
	pm.cap = 10;
	sm.n = 100;
	pm.n = 10;
	pm_init (&sm);
	pm_init (&pm);
	v2f32_random_wh (pm.n, pm.pos, w, h);
	for (uint32_t i = 0; i < pm.n; ++i) {pm.D [i] = 50.0f;}
	
	
	ASSERT (SDL_Init (SDL_INIT_EVERYTHING) == 0);
	SDL_Window * window = NULL;
	SDL_Renderer * renderer = NULL;
	ASSERT (SDL_CreateWindowAndRenderer (800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer) == 0);
	
	uint32_t flags = 0;
	flags |= cap.read (f0) ? 0 : QUIT;
	uint32_t iframe = cap.get (cv::CAP_PROP_POS_FRAMES);
	SDL_Texture * texture = SDLCV_CreateTexture (renderer, f0);
	SDL_Event event;
	
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
					flags ^= UPDATE_ALL;
					//flags ^= (UPDATE_TRACKER | UPDATE_GRAPHICS);
					flags |= SEMIAUTO;
					break;
					
					case SDLK_SPACE:
					flags ^= UPDATE_ALL;
					//flags ^= (UPDATE_TRACKER | UPDATE_GRAPHICS);
					flags &= ~SEMIAUTO;
					break;
				}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
				flags |= SIMULATE_MOVE;
				break;
				
				case SDL_MOUSEBUTTONUP:
				flags &= ~SIMULATE_MOVE;
				break;
					
				case SDL_MOUSEMOTION:
				if (flags & SIMULATE_MOVE)
				{
					//Convert the mouse position to OpenCV mat/image (f0) coordinate system.
					int w;
					int h;
					SDL_GetWindowSize (window, &w, &h);
					uint32_t i = 0;
					float * x = sm.pos + (i * 2);
					x [1] = (event.motion.x * f0.cols) / w;
					x [0] = (event.motion.y * f0.rows) / h;
					//TRACE_F ("%f %f %i", x [0], x [1], sm.n);
					//TRACE_F ("%i %i %f", w, f0.cols, ((float) / (float)w));
				}
				break;
			}
		}
		if (flags & QUIT) {break;}
		if (flags & PAUSE) {continue;}
	
	
		if (flags & UPDATE_FRAME)
		{
			flags |= cap.read (f0) ? 0 : QUIT;
			if (flags & QUIT) {break;}
		}
		
		if (flags & UPDATE_DECTECTION)
		{
			f0.copyTo (f1);
			cv::morphologyEx (f0, f1, cv::MORPH_GRADIENT, element);
			bgfs->apply (f1, mask);
			cv::blur (mask, mask, cv::Size (3, 3));
			blobber->detect (mask, kp);
			f0.copyTo (f1);
		}
		
		if (flags & UPDATE_TRACKER)
		{
			sm.n = sm.cap;
			kp_to_v2f32 (&sm.n, sm.pos, sm.D, kp);
			//draw_circle (f1, pm.n, pm.pos, pm.Davg, 0, 255, 0);
			pm_update 
			(
				pm.n, 
				pm.pos, 
				pm.vel, 
				pm.D, 
				pm.Davg, 
				pm.persistence, 
				pm.duration,
				sm.n, 
				sm.pos, 
				sm.D, 
				7, 
				10.0f,
				7.0f
			);
		}
		
		if (flags & UPDATE_GRAPHICS)
		{
			//f1.setTo (0);
			draw_blob (f1, sm.n, sm.pos, sm.D, 0, 0, 255);
			//TRACE_F ("%f", pm.pos [0]);
			draw_id 
			(
				f1, 
				pm.n, 
				pm.pos, 
				pm.vel, 
				pm.Davg, 
				pm.persistence,
				pm.duration,
				7,
				10.0f,
				7.0f
			);
			
			if (0)
			{
				uint32_t k0 = 0;
				uint32_t k1 = 0;
				uint32_t k2 = 0;
				for (uint32_t i = 0; i < pm.n; ++i) 
				{
					if (pm.persistence [i] == 0)      {k0++;}
					else if (pm.persistence [i] == 4) {k1++;}
					else                              {k2++;}
				}
				char buf [100] = {0};
				snprintf (buf, 100, "b: %03i   k: (%03i %03i %03i)", sm.n, k0, k1, k2);
				cv::Point2f p (10, 20);
				cv::putText (f1, buf, p, CV_FONT_HERSHEY_DUPLEX, 0.76, cv::Scalar (0, 0, 255), 1);
			}
		}
		
		if (flags & SEMIAUTO) 
		{
			flags &= ~(UPDATE_ALL | BACKWARD);
		}

		
		SDLCV_CopyTexture (texture, f1);
		SDL_RenderClear (renderer);
		SDL_RenderCopy (renderer, texture, NULL, NULL);
		SDL_RenderPresent (renderer);
	}
	
	return 0;
}

