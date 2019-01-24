#pragma once


void SDLCV_CopyTexture (SDL_Texture * texture, cv::Mat const &mat)
{
    unsigned char * buffer = NULL;
    int pitch = 0;
    SDL_LockTexture (texture, 0, (void **)&buffer, &pitch);
    memcpy (buffer, (void *)mat.ptr (), mat.cols * mat.rows * 3);
    SDL_UnlockTexture (texture);
}


SDL_Texture * SDLCV_CreateTexture (SDL_Renderer * renderer, cv::Mat const &mat)
{
	SDL_Texture * texture = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_BGR24,
		SDL_TEXTUREACCESS_STREAMING,
		mat.cols,
		mat.rows
	);
	ASSERT (texture);
	return texture;
}
