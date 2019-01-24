#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utility.hpp>

#include <csc/debug.h> //Debugging functions.
#include <csc/v.h> //Vector operations.
#include <csc/v2.h> //Vector operations.

#define MOTP_TAKEN 0x0001
#define MOTP_SEARCHR2_MAX (80.0f*80.0f)
#define MOTP_SEARCHR2_RATE (35.0f*35.0f)
#define MOTP_SEARCHR2_OFFSET (10.0f*10.0f)
#define MOTP_SEARCHR2_MARGIN (10.0f*10.0f)

//Multiple Object Tracker Particle (MOTP)
struct MOTP
{
	uint32_t cap; //Capacity
	float * x0; //Position
	float * x1; //Velocity
	float * r; //Search radius
	float * d; //TrackerDetected vector.
	uint32_t * t; //Number of tracked frames.
};


void motp_init (struct MOTP * m)
{
	//Allocate memory where memory size is according to 
	//capacity times number of dimensions.
	//All bits is set to zero.
	m->x0 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x1 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->d  = (float *) calloc (m->cap, sizeof (float) * 2);
	m->r  = (float *) calloc (m->cap, sizeof (float) * 1);
	m->t  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
}


void motp_update (struct MOTP * m)
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


float motp_shortest (struct MOTP * m, float const z [2])
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


void motp_lockon (struct MOTP * m, std::vector <cv::KeyPoint> & kp)
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
		if (r [0] > MOTP_SEARCHR2_MAX)
		{
			while (j--)
			{
				if (kp [j].class_id != -1) {continue;}
				float * z0 = (float *) &kp [j].pt;
				if (motp_shortest (m, z0) < MOTP_SEARCHR2_MAX) {continue;}
			
				kp [j].class_id = i;
				vf32_cpy (2, x0, z0);
				vf32_set1 (2, x1, 0.0f);
				vf32_set1 (1, r, MOTP_SEARCHR2_OFFSET);
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
				if (l < r [0] && l < MOTP_SEARCHR2_MAX)
				{
					n ++;
					v2f32_add (d, d, zx0);
					kp [j].class_id = i;
				}
			}
			r [0] += MOTP_SEARCHR2_RATE;
			if (n == 0) {continue;}
			v2f32_mus (d, d, 1.0f / n);
			r [0] = v2f32_norm2 (d) + MOTP_SEARCHR2_MARGIN;
		}
	}
}



















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


void draw_motp 
(
	cv::Mat &img,
	struct MOTP * m
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
		cv::circle      (img, p0, sqrtf (MOTP_SEARCHR2_MAX), cv::Scalar (255, 150, 100), 1);
		cv::drawMarker  (img, p0, cv::Scalar (0, 255, 255), 1, 10, 1);
		cv::putText     (img, text, p0+cv::Point2f (5,5), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
		cv::arrowedLine (img, p0, p0+p1*10.0f, cv::Scalar (0, 0, 255), 1, 8, 0, 0.1);
		cv::arrowedLine (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0, 0.1);
		//cv::line (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0);
	}
}
