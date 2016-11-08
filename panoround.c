#include <stdio.h>
#include "ffmdecode.h"
#include <stdlib.h>
#include <string.h>

int gwidth;
int gheight;
int firstframe = 1;
int maxframe;
int * framenos;

//MAXFRAMES = frames*FRAMEPXCT
#define MAXFRAMES 76000


int framepxct = 2;
int pxspacing = 0;
int norot90 = 0;

#define FRAMEPXCT framepxct
#define PXSPACING pxspacing

unsigned char * bigframedata;

void initframes( const unsigned char * frame, int linesize )
{
	int x, y;
	firstframe = 0;

	printf( "First frame got.\n" );

	bigframedata = malloc( gwidth * MAXFRAMES * 3);
}

void got_video_frame( const unsigned char * rgbbuffer, int linesize, int width, int height, int frame )
{
	int line;
	int y, lx;
	float x;

	if( firstframe )
	{
		if( norot90 )
		{
			gwidth = height;
			gheight = width;
		} else {
			gwidth = width;
			gheight = height;
		}
		initframes( rgbbuffer, linesize );
	}

	//X/Y flipped
	lx = 0;
	x = (height + FRAMEPXCT * PXSPACING)/2;

	for( ; lx < FRAMEPXCT; lx++ )
	{
		if( norot90 )
		{
			for( y = 0; y < gwidth; y++ )
			{
				bigframedata[(frame*FRAMEPXCT+lx + (y)*MAXFRAMES)*3+0] = rgbbuffer[(((int)x)*3+y*linesize)+0];
				bigframedata[(frame*FRAMEPXCT+lx + (y)*MAXFRAMES)*3+1] = rgbbuffer[(((int)x)*3+y*linesize)+1];
				bigframedata[(frame*FRAMEPXCT+lx + (y)*MAXFRAMES)*3+2] = rgbbuffer[(((int)x)*3+y*linesize)+2];
			}
		} else {
			for( y = 0; y < gwidth; y++ )
			{
				bigframedata[(frame*FRAMEPXCT+lx + y*MAXFRAMES)*3+0] = rgbbuffer[(y*3+((int)x)*linesize)+0];
				bigframedata[(frame*FRAMEPXCT+lx + y*MAXFRAMES)*3+1] = rgbbuffer[(y*3+((int)x)*linesize)+1];
				bigframedata[(frame*FRAMEPXCT+lx + y*MAXFRAMES)*3+2] = rgbbuffer[(y*3+((int)x)*linesize)+2];
			}
		}

		
		x-=PXSPACING;
	}

	printf( "%d %d %d\n", frame, linesize, width );

	maxframe = frame;
}

int main( int argc, char ** argv )
{
	FILE * f;
	int line;
	setup_video_decode();
	if( argc < 2 )
	{
		fprintf( stderr, "Need Parameters:   [video name] [norot90 ([[1]]/0)] [framepxct [2]] [pxspacing [0]]\n" );
		fprintf( stderr, "  norot90 = 1 for a portrait video, 0 for a landscape.\n" );
		fprintf( stderr, "  framepxct = # of pixels to pull from centerline\n" );
		fprintf( stderr, "  pxspacing = Offset to pull pixels from.\n" );
		exit( -1 );
	}
	char video_ppm[1024];
	
	if( argc > 2 )
	{
		norot90 = atoi( argv[2] );
	}
	if( argc > 3 )
	{
		framepxct = atoi( argv[3] );
		if( framepxct < 1 ) 
		{
			fprintf( stderr, "Error: cannot have 0 pixels pxct\n" );
			exit(-2);
		}
	}
	if( argc > 4 )
	{
		pxspacing = atoi( argv[4] );
	}

	video_decode( argv[1] );
	printf( "Loading...\n" );
	sprintf( video_ppm, "%s.ppm", argv[1] );
	printf( "Video out file: %s\n", video_ppm );
	f = fopen( video_ppm, "wb" );
	fprintf( f, "P6\n%d %d\n255\n", maxframe*FRAMEPXCT, gwidth );
	if( norot90 )
	{
		for( line = 0; line < gwidth; line++ )
		{
			int x;
			for( x = 0; x < maxframe*FRAMEPXCT; x++ )
			{
				unsigned char px[3];
				px[0] = bigframedata[(x+line*MAXFRAMES)*3+0];
				px[1] = bigframedata[(x+line*MAXFRAMES)*3+1];
				px[2] = bigframedata[(x+line*MAXFRAMES)*3+2];
				fwrite( px, 1, 3, f );
			}
		}
	}
	else
	{
		for( line = gwidth-1; line >= 0; line-- )
		{
			int x;
			for( x = 0; x < maxframe*FRAMEPXCT; x++ )
			{
				unsigned char px[3];
				px[0] = bigframedata[(x+line*MAXFRAMES)*3+0];
				px[1] = bigframedata[(x+line*MAXFRAMES)*3+1];
				px[2] = bigframedata[(x+line*MAXFRAMES)*3+2];
				fwrite( px, 1, 3, f );
			}
		}


	}

	fclose( f );

}
