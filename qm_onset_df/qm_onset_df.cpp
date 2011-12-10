/*
 *  qm_onset_df.cpp
 *  Mex interface for OnsetDetectionFunction class
 *
 *  Created by Adam Stark on 22/03/2011.
 *  Copyright 2011 Queen Mary University of London. All rights reserved.
 *
 */

#include <math.h>
#include <iostream>
#include "mex.h"
#include "OnsetDetectionFunction.h"

using namespace std;

/* Input Arguments */

#define	Y_IN	prhs[0]
#define	HSIZE	prhs[1]
#define	FSIZE	prhs[2]
#define	DF_TYPE	prhs[3]


/* Output Arguments */

#define	YP_OUT	plhs[0]

#if !defined(MAX)
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
#endif

#if !defined(MIN)
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#endif


void mexFunction( int nlhs, mxArray *plhs[], 
				 int nrhs, const mxArray*prhs[] )

{ 
	int in_fsize; // input value of frame size
	int in_hsize; // input value of hop size
	int in_df_type; // input detection function type
	int fsize;	// frame size
	int hsize;	// hop size
	int df_type; // detection function type
	
    // if one input then we just have the signal so use default parameters
    if (nrhs == 1) { 
		
		fsize = 1024;	// frame size
		hsize = fsize/2;	// hop size
		
		df_type = 6;		// default is complex spectral difference (half-wave rectified)
		
	}
	else if (nrhs > 1)	// else parameters should be specified
	{
		in_hsize = *mxGetPr(HSIZE);
		in_fsize = *mxGetPr(FSIZE);
		in_df_type = *mxGetPr(DF_TYPE);
		
		// check for valid detection function type
		if ((in_df_type < 0) || (in_df_type > 9))
		{
			mexErrMsgTxt("Invalid Detection Function Type");
		}
		
		
		// check hop size does not exceed frame size
		if (in_hsize > in_fsize)
		{
			mexErrMsgTxt("Hop size exceeds frame size");
		}
		
		hsize = in_hsize;
		fsize = in_fsize;
		df_type = in_df_type;
		
	}
    
	if (nlhs > 1) {
		mexErrMsgTxt("Too many output arguments."); 
    } 
    
	double *df; 
    double *t,*y; 
    mwSize m,n; 
	
	int signal_length; // length of the input signal
	
	OnsetDetectionFunction onset(hsize,fsize,df_type,1);
	
	int numframes;
	
	double buffer[hsize];	// buffer to hold one hopsize worth of audio samples
	
	// fill buffer with zeros to initialise it
	for (int i = 0;i < hsize;i++)
	{
		buffer[i] = 0.0;
	}
	
    /* Check the dimensions of Y.  Y can be 4 X 1 or 1 X 4. */ 
    m = mxGetM(Y_IN); 
    n = mxGetN(Y_IN);
	
	// check that df input is a vector
	if ((m > 1) && (n > 1))
	{
		mexErrMsgTxt("Input must be a vector."); 
	}
	
	// get signal length
	signal_length = MAX(m,n);
		        
    y = mxGetPr(Y_IN);
	
	
	// get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hsize));
	
	/* Create a matrix for the return argument */ 
    YP_OUT = mxCreateDoubleMatrix(1, numframes, mxREAL); 
	
	/* Assign pointers to the various parameters */ 
    df = mxGetPr(YP_OUT);
		
	///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int k=0;k < numframes;k++)
	{		
		// add new samples to frame
		for (int n = 0;n < hsize;n++)
		{
			buffer[n] = y[(k*hsize)+n];
		}
		
		df[k] = onset.getDFsample(buffer);
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////

    return;
    
}


