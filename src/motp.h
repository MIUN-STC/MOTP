#pragma once
#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/utility.hpp>

#include <csc/debug.h> //Debugging functions.
#include <csc/v.h> //Vector operations.
#include <csc/v2.h> //Vector operations.

#include <string.h>

//Release the tracker after this many untrack steps.
#define MOTP_RELEASE 20

//How close a new tracker can start to all other trackers.
#define MOTP_RELEASE_PROXIMITY (50.0f*50.0f)

//Can merge with others after this many track steps.
#define MOTP_MERGE 200

//The maximum search radius
#define MOTP_SEARCHR2_MAX (100.0f*100.0f)

//The search radius growrate when no keypoints are found.
#define MOTP_SEARCHR2_GROWRATE_OFFSET (10.0f*10.0f)
#define MOTP_SEARCHR2_GROWRATE_TIMEBOOST (1000.0f)
#define MOTP_SEARCHR2_GROWRATE_VELBOOST (20.0f)

//The starting search radius for tracking a new object.
#define MOTP_SEARCHR2_START (100.0f*100.0f)

//The search radius offset when tracking an object.
//The search radius is already adaptive but this value can offset the adaptive radius.
#define MOTP_SEARCHR2_OFFSET (5.0f*5.0f)
#define MOTP_SEARCHR2_TIMEBOOST (4000.0f)
#define MOTP_SEARCHR2_VELBOOST (2.0f)

//Tracker mass. Higher value makes it harder to turn.
#define MOTP_MASS 20.0f

//A new tracker will get a boost at the beginning.
//Higher make tracker use more force.
//This becomes insignifcant when a tracker has tracked for a while.
#define MOTP_TIMEBOOST 0.5f

//Position correction and antiorbit constant. Range is 0 .. 1.
//Higher means use more history velocity.
//Lower means use more realtime positioning.
//Setting this constant 1 can make the tracker oscillating or orbit around keypoints.
#define MOTP_VELDELTA_MIX 0.8f


//The rate tracker velocity slows down
#define MOTP_VELREDUCE 0.95f




//Experimental
void motp_dist_exp (float y [2], float x [2], std::vector <cv::KeyPoint> & kp, float r, uint32_t * n, float t, float ix)
{
	vf32_set1 (2, y, 0.0f);
	n [0] = 0;
	float dmin [2];
	float lmin= FLT_MAX;
	float ysum [2] = {0.0f, 0.0f};
	uint32_t i = kp.size ();
	while (i--)
	{
		float * z = (float *) &kp [i].pt;
		//TRACE_F ("%f %f", d [0], d [1]);
		float zx [2];
		vf32_sub (2, zx, z, x);
		float l = vf32_norm2 (2, zx);
		if (l > r) {continue;}
		if (kp [i].class_id >= 0 && t < MOTP_MERGE) {continue;}
		if (l < lmin)
		{
			vf32_cpy (2, dmin, zx);
			lmin = l;
		}
		vf32_mus (2, zx, zx, 1.0f / (1.0f + l));
		vf32_add (2, ysum, ysum, zx);
		n [0] ++;
		kp [i].class_id = ix;
	}
	//TRACE_F ("%i %u %f %f", 0, n [0], ysum [0], ysum [1]);
	if (n [0] > 0 && (ysum [0]+ysum[1]) != 0)
	{
		vf32_mus (2, y, ysum, vf32_norm (2, dmin) / vf32_norm (2, ysum));
	}
	//TRACE_F ("%i %f %f", n [0], y [0], y [1]);
}






//Experimental
void motp_dist (float d [2], float x [2], std::vector <cv::KeyPoint> & kp, float r, uint32_t * n, float t, float ix)
{
	vf32_set1 (2, d, 0.0f);
	n [0] = 0;
	float dmin [2];
	float lmin= FLT_MAX;
	uint32_t imin = UINT32_MAX;
	float ysum [2] = {0.0f, 0.0f};
	uint32_t i = kp.size ();
	while (i--)
	{
		float * z = (float *) &kp [i].pt;
		//TRACE_F ("%f %f", d [0], d [1]);
		float zx [2];
		vf32_sub (2, zx, z, x);
		float l = vf32_norm2 (2, zx);
		if (l > r) {continue;}
		if (kp [i].class_id >= 0 && t < MOTP_MERGE) {continue;}
		if (l < lmin)
		{
			vf32_cpy (2, d, zx);
			lmin = l;
			imin = i;
		}
		n [0] ++;
	}
	if (imin < UINT32_MAX)
	{
		kp [imin].class_id = ix;
	}
}







//Multiple Object Tracker Particle (MOTP)
struct MOTP
{
	uint32_t id_counter;
	uint32_t cap;  //Capacity
	float    * x0; //Tracker position (dim=2)
	float    * x1; //Tracker velocity (dim=2)
	float    * x2; //Tracker acceleration (dim=2)
	float    * d;  //TrackerDetected vector (dim=2)
	float    * r;  //Tracker search-radius (dim=1)
	uint32_t * t;  //Tracker track-duration (dim=1)
	uint32_t * u;  //Tracker untrack-duration (dim=1)
	uint32_t * id; //Tracker id (dim=1)
};


void motp_init (struct MOTP * m)
{
	m->id_counter = 1;
	//Allocate memory where memory size is according to 
	//capacity times number of dimensions.
	//All bits is set to zero.
	m->x0 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x1 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->x2 = (float *) calloc (m->cap, sizeof (float) * 2);
	m->d  = (float *) calloc (m->cap, sizeof (float) * 2);
	m->r  = (float *) calloc (m->cap, sizeof (float) * 1);
	m->t  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	m->u  = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	m->id = (uint32_t *) calloc (m->cap, sizeof (uint32_t) * 1);
	ASSERT (m->x0);
	ASSERT (m->x1);
	ASSERT (m->x2);
	ASSERT (m->d);
	ASSERT (m->r);
	ASSERT (m->t);
	ASSERT (m->u);
	ASSERT (m->id);
	vu32_set1 (m->cap, m->u, UINT32_MAX);
	for (uint32_t i = 0; i < m->cap; ++i)
	{
		//m->id [i] = m->id_counter;
		//m->id_counter ++;
	}
}


void motp_update (struct MOTP * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float    *     x0 = m->x0     + i * 2; //Tracker position (dim=2)
		float    *     x1 = m->x1     + i * 2; //Tracker velocity (dim=2)
		float    *     x2 = m->x2     + i * 2; //Tracker acceleration (dim=2)
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance (dim=2)
		float    *      r = m->r      + i * 1; //Tracker search-radius (dim=1)
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration (dim=1)
		uint32_t *      u = m->u      + i * 1; //Tracker untrack-duration (dim=1)
		uint32_t *     id = m->id     + i * 1; //Tracker id (dim=1)
		
		//Do not update released trackers.
		if (u [0] >= MOTP_RELEASE) {continue;}
		
		//Use the TrackerKeypoint distance (d) to give the tracker a force towards the keypoints.
		//Also give new trackers a force boost 
		//to catchup a fast moving keypoints by using tracking time (t).
		ASSERT (t [0] > 0);
		ASSERT (MOTP_MASS > 0);
		float k = (1.0f / MOTP_MASS) + (MOTP_TIMEBOOST / (t [0] + u [0]));
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
	//Here we can choose the amount of slowdown.
	vf32_mus (m->cap*2, m->x1, m->x1, MOTP_VELREDUCE);
}


float motp_shortest (struct MOTP * m, float const z [2])
{
	float lmin = FLT_MAX;
	uint32_t i = m->cap;
	while (i--)
	{
		float    *     x0 = m->x0     + i * 2; //Tracker position (dim=2)
		float    *     x1 = m->x1     + i * 2; //Tracker velocity (dim=2)
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance (dim=2)
		float    *      r = m->r      + i * 1; //Tracker search-radius (dim=1)
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration (dim=1)
		uint32_t *      u = m->u      + i * 1; //Tracker untrack-duration (dim=1)
		uint32_t *     id = m->id     + i * 1; //Tracker id (dim=1)
		//Ignore released trackers.
		if (u [0] >= MOTP_RELEASE) {continue;}
		float zx0 [2];
		v2f32_sub (zx0, z, x0);
		float l = v2f32_norm2 (zx0);
		if (l < lmin) {lmin = l;}
	}
	return lmin;
}


void motp_search (struct MOTP * m, std::vector <cv::KeyPoint> & kp)
{
	vu32_add1max (m->cap    , m->u, m->u, 1, UINT32_MAX);
	vf32_set1    (m->cap * 2, m->d, 0.0f);
	uint32_t j = kp.size ();
	while (j--)
	{
		float * z0 = (float *)&kp [j].pt;
		uint32_t i = m->cap;
		uint32_t imin = UINT32_MAX;
		float dmin [2];
		float lmin = FLT_MAX;
		//Find closest keypoint:
		while (i--)
		{
			float    *     x0 = m->x0     + i * 2; //Tracker position (dim=2)
			float    *     x1 = m->x1     + i * 2; //Tracker velocity (dim=2)
			float    *      d = m->d      + i * 2; //TrackerKeypoint distance (dim=2)
			float    *      r = m->r      + i * 1; //Tracker search-radius (dim=1)
			uint32_t *      t = m->t      + i * 1; //Tracker track-duration (dim=1)
			uint32_t *      u = m->u      + i * 1; //Tracker untrack-duration (dim=1)
			uint32_t *     id = m->id     + i * 1; //Tracker id (dim=1)
			if (u [0] > MOTP_RELEASE) {continue;}
			float zx0 [2];
			vf32_sub (2, zx0, z0, x0);
			float l = vf32_norm2 (2, zx0);
			if (l > r [0]) {continue;}
			if (l < lmin)
			{
				lmin = l;
				imin = i;
				vf32_cpy (2, dmin, zx0);
			}
		}
		if (imin != UINT32_MAX)
		{
			//TRACE_F ("%f", lmin);
			float    *     x0 = m->x0     + imin * 2; //Tracker position (dim=2)
			float    *     x1 = m->x1     + imin * 2; //Tracker velocity (dim=2)
			float    *      d = m->d      + imin * 2; //TrackerKeypoint distance (dim=2)
			float    *      r = m->r      + imin * 1; //Tracker search-radius (dim=1)
			uint32_t *      t = m->t      + imin * 1; //Tracker track-duration (dim=1)
			uint32_t *      u = m->u      + imin * 1; //Tracker untrack-duration (dim=1)
			uint32_t *     id = m->id     + imin * 1; //Tracker id (dim=1)
			vf32_cpy (2, d, dmin);
			vu32_set1 (1, u, 0);
			vu32_add1max (1, t, t, 1, UINT32_MAX);
		}
	}
}


void motp_release (struct MOTP * m, std::vector <cv::KeyPoint> & kp)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float    *     x0 = m->x0     + i * 2; //Tracker position (dim=2)
		float    *     x1 = m->x1     + i * 2; //Tracker velocity (dim=2)
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance (dim=2)
		float    *      r = m->r      + i * 1; //Tracker search-radius (dim=1)
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration (dim=1)
		uint32_t *      u = m->u      + i * 1; //Tracker untrack-duration (dim=1)
		uint32_t *     id = m->id     + i * 1; //Tracker id (dim=1)
		
		//(u) is the amount of steps the tracker is not been tracking.
		//Check if the the tracker has not been tracking for a while:
		if (u [0] >= MOTP_RELEASE)
		{
			uint32_t j = kp.size ();
			while (j--)
			{
				//The tracker can not jump to the keypoint (z) if it belong to another tracker.
				if (kp [j].class_id != -1) {continue;}
				float * z0 = (float *) &kp [j].pt;
				//The tracker can not jump to the keypoint (z) if other trackers are in proximity.
				if (motp_shortest (m, z0) < MOTP_RELEASE_PROXIMITY) {continue;}
				//Now the tracker is free to take this keypoint.
				//The tracker is moved to the keypoint.
				//The tracker is reset.
				vf32_cpy (2, x0, z0);
				vu32_set1 (1, u, 0);
				vf32_set1 (2, x1, 0.0f);
				vf32_set1 (1, r, MOTP_SEARCHR2_START);
				vu32_set1 (1, t, 1);
				//Generate a new id for the tracker
				kp [j].class_id = m->id_counter;
				id [0]          = m->id_counter;
				m->id_counter ++;
				//TRACE_F ("%u %u", i, id [0]);
			}
		}
		
		
		
		
	}
}


void motp_expand (struct MOTP * m, std::vector <cv::KeyPoint> & kp)
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
		if (u [0] > 0)
		{
			r [0] += MOTP_SEARCHR2_GROWRATE_OFFSET;
			r [0] += MOTP_SEARCHR2_GROWRATE_VELBOOST * v2f32_norm2 (x1);
			r [0] += MOTP_SEARCHR2_GROWRATE_TIMEBOOST / t [0];
			r [0] = MIN (r [0], MOTP_SEARCHR2_MAX);
		}
		else if (u [0] == 0)
		{
			r [0] = MOTP_SEARCHR2_OFFSET;
			r [0] += MOTP_SEARCHR2_VELBOOST * v2f32_norm2 (x1);
			r [0] += MOTP_SEARCHR2_TIMEBOOST / (t [0]*t [0]);
			r [0] += v2f32_norm2 (d);
		}
	}
}


struct MOTP_Files
{
	FILE * x0;
	FILE * x1;
	FILE * x2;
	FILE * d;
	FILE * r;
	FILE * t;
	FILE * u;
	FILE * id;
};


void motp_openfiles (struct MOTP_Files * m, const char * name)
{
	char filename [100];
	snprintf (filename, 100, "%s%s", name, "_x0.txt");
	m->x0 = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_x1.txt");
	m->x1 = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_x2.txt");
	m->x2 = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_d.txt");
	m->d = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_r.txt");
	m->r = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_t.txt");
	m->t = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_u.txt");
	m->u = fopen (filename, "w+");
	snprintf (filename, 100, "%s%s", name, "_id.txt");
	m->id = fopen (filename, "w+");
	ASSERT (m->x0);
	ASSERT (m->x1);
	ASSERT (m->x2);
	ASSERT (m->d);
	ASSERT (m->r);
	ASSERT (m->t);
	ASSERT (m->u);
	ASSERT (m->id);
}


void motp_writefiles (struct MOTP_Files * f, struct MOTP * m)
{
	uint32_t i = m->cap;
	while (i--)
	{
		float    *     x0 = m->x0     + i * 2; //Tracker position
		float    *     x1 = m->x1     + i * 2; //Tracker velocity
		float    *     x2 = m->x2     + i * 2; //Tracker acceleration
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance
		float    *      r = m->r      + i * 1; //Tracker search-radius
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration.
		uint32_t *      u = m->u      + i * 1; //Tracker track-duration.
		uint32_t *     id = m->id     + i * 1; //Tracker track-duration.
		if (u [0] >= MOTP_RELEASE) {continue;}
		fprintf (f->x0, "%e %e\n", x0 [0], x0 [1]);
		fprintf (f->x1, "%e %e\n", x1 [0], x1 [1]);
		fprintf (f->x2, "%e %e\n", x2 [0], x2 [1]);
		fprintf (f->d, "%e %e\n", d [0], d [1]);
		fprintf (f->r, "%e\n", r [0]);
		fprintf (f->t, "%u\n", t [0]);
		fprintf (f->u, "%u\n", u [0]);
		fprintf (f->id, "%u\n", id [0]);
	}
}

