#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utility.hpp>

#include <csc/debug.h> //Debugging functions.
#include <csc/v.h> //Vector operations.
#include <csc/v2.h> //Vector operations.


//Release the tracker after this many untrack steps.
#define MOTP_RELEASE 10

//Can merge with others after this many track steps.
#define MOTP_MERGE 90

//The maximum search radius
#define MOTP_SEARCHR2_MAX (80.0f*80.0f)

//The search radius growrate when no keypoints are found.
#define MOTP_SEARCHR2_GROWRATE_OFFSET (5.0f*5.0f)
#define MOTP_SEARCHR2_GROWRATE_TIMEBOOST (3000.0f)
#define MOTP_SEARCHR2_GROWRATE_VELBOOST (10.0f)

//The starting search radius for tracking a new object.
#define MOTP_SEARCHR2_START (100.0f*100.0f)

//The search radius offset when tracking an object.
//The search radius is already adaptive but this value can offset the adaptive radius.
#define MOTP_SEARCHR2_OFFSET (10.0f*10.0f)

//Tracker mass. Higher value makes it harder to turn.
#define MOTP_MASS 20.0f

//A new tracker will get a boost at the beginning.
//Higher make tracker use more force.
//This becomes insignifcant when a tracker has tracked for a while.
#define MOTP_TIMEBOOST 0.5f

//Position correction and antiorbit constant. Range is 0 .. 1.
//Higher means use more history velocity.
//Lower means use more realtime positioning.
#define MOTP_VELDELTA_MIX 0.8f




//Multiple Object Tracker Particle (MOTP)
struct MOTP
{
	uint32_t cap;  //Capacity
	float    * x0; //Tracker position
	float    * x1; //Tracker velocity
	float    * r;  //Tracker search-radius
	float    * d;  //TrackerDetected vector.
	uint32_t * t;  //Tracker track-duration.
	uint32_t * u;  //Tracker untrack-duration.
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
	m->u  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
}


void motp_update (struct MOTP * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float    * x0 = m->x0 + i * 2; //Tracker position
		float    * x1 = m->x1 + i * 2; //Tracker velocity
		float    *  d = m->d  + i * 2; //TrackerKeypoint distance
		uint32_t *  t = m->t  + i * 1; //Tracker track-duration.
		float const mass = 20.0f; //Tracker mass
		
		//Use the TrackerKeypoint distance (d) to give the tracker a force towards the keypoints.
		//Also give new trackers a force boost 
		//to catchup a fast moving keypoints by using tracking time (t).
		ASSERT (t [0] > 0);
		ASSERT (MOTP_MASS > 0);
		float k = (1.0f / MOTP_MASS) + (MOTP_TIMEBOOST / t [0]);
		float x2 [2];
		v2f32_mus (x2, d, k);
		v2f32_add (x1, x1, x2);
		
		//Reduce tracker oscillation and orbitation around a keypoints.
		//Velocity (x1mix) is mix of 
		//history velocity (x1) and realtime average distance TrackerKeypoint (d)
		float x1mix [2];
		vf32_weight_ab (2, x1mix, x1, d, MOTP_VELDELTA_MIX);
		
		//Finaly update the position.
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
		float    * x0 = m->x0 + i * 2;
		uint32_t *  u = m->u  + i * 1;
		//if (u [0] < MOTP_RELEASE) {continue;}
		float d [2];
		v2f32_sub (d, x0, z);
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
		float    *     x0 = m->x0     + i * 2; //Tracker position
		float    *     x1 = m->x1     + i * 2; //Tracker velocity
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance
		float    *      r = m->r      + i * 1; //Tracker search-radius
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration.
		uint32_t *      u = m->u      + i * 1; //Tracker track-duration.
		
		//The TrackerKeypoint distance will be zero if no keypoints was found or the tracker 
		//starts to track a new object.
		vf32_set1 (2, d, 0.0f);
			
		uint32_t j = kp.size ();
		//If the serach radius has expanded too much then 
		//the tracker is free to jump to a new keypoint which should represent a new object.
		if (u [0] > MOTP_RELEASE)
		{
			vf32_set1 (2, x0, 0.0f);
			vu32_set1 (1, t, 1);
			while (j--)
			{
				//The tracker can not jump to the keypoint (z) if it belong to another tracker.
				if (kp [j].class_id != -1) {continue;}
				float * z0 = (float *) &kp [j].pt;
				//The tracker can not jump to the keypoint (z) if other tracker is in proximity.
				if (motp_shortest (m, z0) < MOTP_SEARCHR2_MAX) {continue;}
				//Now the tracker is free to take this keypoint.
				//The tracker is moved to the keypoint.
				//The tracker resets.
				kp [j].class_id = i;
				vf32_cpy (2, x0, z0);
				vf32_set1 (2, x1, 0.0f);
				vf32_set1 (1, r, MOTP_SEARCHR2_START);
				vu32_set1 (1, u, 1);
			}
		}
		//Search for keypoints inside the search radius.
		else
		{
			//Increase the tracking time (t).
			//The serach radius is below limit so we assume the tracker is tracking something
			//even if there is no keypoint at this time.
			vu32_add1max (1, t, t, 1, UINT32_MAX);
			uint32_t n = 0;
			while (j--)
			{
				float * z0 = (float *) &kp [j].pt;
				float zx0 [2];
				v2f32_sub (zx0, z0, x0);
				float l = v2f32_norm2 (zx0);
				//If the tracker sees keypoints inside the search radius and the maximum search radius
				//then find the average distance (d) to all the keypoints.
				if (l > r [0]) {continue;}
				if (kp [j].class_id >= 0 && t [0] < MOTP_MERGE) {continue;}
				n ++;
				v2f32_add (d, d, zx0);
				kp [j].class_id = i;
			}
			//Increase the search radius (r) if 0 keypoints (n = 0) was found.
			if (n == 0) 
			{
				r [0] += MOTP_SEARCHR2_GROWRATE_OFFSET;
				r [0] += MOTP_SEARCHR2_GROWRATE_VELBOOST * v2f32_norm2 (x1);
				r [0] += MOTP_SEARCHR2_GROWRATE_TIMEBOOST / t [0];
				r [0] = MIN (r [0], MOTP_SEARCHR2_MAX);
				vu32_add1max (1, u, u, 1, UINT32_MAX);
				continue;
			}
			//Get average distance to all the keypoints.
			v2f32_mus (d, d, 1.0f / n);
			//Set the search radius the same distance as from the tracker to the keypoints.
			r [0] = v2f32_norm2 (d) + MOTP_SEARCHR2_OFFSET;
			u [0] = 1;
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
		uint32_t * u = m->u + i * 1;
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
		//cv::circle      (img, p0, sqrtf (MOTP_SEARCHR2_MAX), cv::Scalar (255, 150, 100), 1);
		cv::drawMarker  (img, p0, cv::Scalar (0, 255, 255), 1, 10, 1);
		cv::putText     (img, text, p0+cv::Point2f (5,5), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (50, 100, 255), 1);
		cv::arrowedLine (img, p0, p0+p1*10.0f, cv::Scalar (0, 0, 255), 1, 8, 0, 0.1);
		cv::arrowedLine (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0, 0.1);
		//cv::line (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0);
	}
}
