#pragma once
#include "motp.h"


void draw_kp (cv::Mat &img, std::vector <cv::KeyPoint> const & kp)
{
	char text [10];
	uint32_t i = kp.size ();
	while (i--)
	{
		cv::Point2f p = kp [i].pt;
		float d = kp [i].size;
		//snprintf (text, 10, "%i", kp [i].class_id);
		//cv::circle (img, p, d, cv::Scalar (255, 0, 255), 0.5);
		cv::drawMarker (img, p,  cv::Scalar (255, 0, 255), cv::MARKER_CROSS, 4, 1);
		//cv::putText (img, text, p + cv::Point2f (-10.0f, -10.0f), CV_FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar (255, 0, 255), 1);
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
		float    *     x0 = m->x0     + i * 2; //Tracker position
		float    *     x1 = m->x1     + i * 2; //Tracker velocity
		float    *      d = m->d      + i * 2; //TrackerKeypoint distance
		float    *      r = m->r      + i * 1; //Tracker search-radius
		uint32_t *      t = m->t      + i * 1; //Tracker track-duration.
		uint32_t *      u = m->u      + i * 1; //Tracker track-duration.
		uint32_t *     id = m->id     + i * 1; //Tracker id.
		
		if (u [0] >= MOTP_RELEASE) {continue;};
		//TRACE_F ("%i %f %f", i, x [0], x [1]);
		float r0 = MIN (sqrtf (r [0]), 100000.0f);
		snprintf (text, 20, "%u %u", id [0], t [0]);
		//snprintf (text, 20, "%u %u %u", i, id [0], t [0]);
		//snprintf (text, 10, "%u %f", i, e [0]);
		//snprintf (text, 10, "%u", i);
		cv::Point2f p0 (x0 [0], x0 [1]);
		cv::Point2f p1 (x1 [0], x1 [1]);
		cv::Point2f pd (d [0], d [1]);
		
		//TRACE_F ("%i %f %f", i, r0, sqrtf (PM_MAX_SEARCHR2));
		cv::circle      (img, p0, r0, cv::Scalar (255, 100, 150), 1.5);
		//cv::circle      (img, p0, sqrtf (MOTP_SEARCHR2_MAX), cv::Scalar (255, 150, 100), 1);
		//cv::drawMarker  (img, p0, cv::Scalar (0, 255, 255), 1.5, 10, 1);
		cv::putText     (img, text, p0+cv::Point2f (5,5), CV_FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar (50, 100, 255), 1);
		cv::arrowedLine (img, p0, p0+p1*10.0f, cv::Scalar (0, 0, 255), 1.5, 8, 0, 0.1);
		cv::arrowedLine (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1.5, 8, 0, 0.1);
		//cv::line (img, p0, p0+pd, cv::Scalar (0, 255, 0), 1, 8, 0);
	}
}







void draw_trace 
(
	cv::Mat &img,
	uint32_t n, //Number of elements
	uint32_t id [], //Tracked id, (dim=1)
	uint32_t t [], //Tracked amount, (dim=1)
	uint32_t u [], //Untracked amount (dim=1)
	float xa [], //Old position (dim=2)
	float xb [] //New position (dim=2)
)
{
	while (n--)
	{
		if (u [0] < MOTP_RELEASE && t [0] > 4)
		{
			srand (id [0]);
			cv::Scalar color (rand () & 255, rand () & 255, rand () & 255);
			cv::Point2f a (xa [0], xa [1]);
			cv::Point2f b (xb [0], xb [1]);
			int thickness = 1;
			int line_type = 8;
			int shift = 0;
			double tipLength = 0.5;
			cv::arrowedLine (img, a, b, color, thickness, line_type, shift, tipLength);
		}
		xa += 2;
		xb += 2;
		u += 1;
		t += 1;
		id += 1;
	}
}
