#pragma once
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "debug.h"
#include "xxav.h"


//Define everything that is needed to decode videos.
//TODO: 
//This seems to be alot of variables for decoding videos.
//Check if it is possible to use less variables or make it simplier.
struct VideoPlayer
{
	uint32_t n;
	AVFormatContext    ** fctx;
	AVCodecContext     ** cctx;
	struct SwsContext  ** wctx;
	//frame0 is decoded output. frame1 is converted to OpenGL pixel format.
	AVFrame            ** frame0;
	AVFrame            ** frame1;
	int                 * ivid;
	char const         ** url;
	int64_t             * pts;
	
	uint32_t width;
	uint32_t height;
};


void vp_init (struct VideoPlayer * vp)
{
	ASSERT (vp->n == 2);
	vp->fctx = calloc (vp->n, sizeof (AVFormatContext *));
	vp->cctx = calloc (vp->n, sizeof (AVCodecContext *));
	vp->wctx = calloc (vp->n, sizeof (struct SwsContext *));
	vp->frame0 = calloc (vp->n, sizeof (AVFrame *));
	vp->frame1 = calloc (vp->n, sizeof (AVFrame *));
	vp->ivid = calloc (vp->n, sizeof (int));
	vp->url = calloc (vp->n, sizeof (char const *));
	vp->pts = calloc (vp->n, sizeof (int64_t));
	ASSERT (vp->fctx);
	ASSERT (vp->cctx);
	ASSERT (vp->wctx);
	ASSERT (vp->frame0);
	ASSERT (vp->frame1);
	ASSERT (vp->ivid);
	ASSERT (vp->url);
	ASSERT (vp->pts);
}


AVStream * vp_getstream (struct VideoPlayer * vp, uint32_t i)
{
	return vp->fctx [i]->streams [vp->ivid [i]];
}


void vp_seek (struct VideoPlayer * vp, uint32_t i, int64_t ts)
{
	avformat_seek_file (vp->fctx [i], -1, INT64_MIN, ts, ts, 0);
	xxav_seek 
	(
		vp->fctx [i], //Av Format context array
		vp->cctx [i], //Av Codec context array
		vp->ivid [i], //AV Index video array
		vp->wctx [i], //SWS context array
		vp->frame0 [i], //Av frame array (video format)
		vp->frame1 [i], //Av frame array (OpenGL format)
		vp->pts + i, //
		ts
	);
}



/*
void player_free (struct player * play)
{
    avcodec_close (play->cctx);
    av_free (play->frame0);
    av_free (play->frame1);
    avformat_close_input (&play->fctx);
}
*/
