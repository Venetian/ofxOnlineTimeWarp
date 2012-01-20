/*
 *  TimeWarp.h
 *  chromaReader13
 *
 *  Created by Andrew on 16/05/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#ifndef _TIME_WARP
#define _TIME_WARP


#include "ofMain.h"
#include "chromaGram.h"
#include "ChordDetect.h"
#include "sndfile.h"
#include "ofxFileDialogOSX.h"

//11/2011
//what is similarity and what chromaSimilarity?

#define FRAMESIZE 512
#define ENERGY_LENGTH 80000
#define CHROMA_LENGTH 12000
#define CHROMA_CONVERSION_FACTOR 16 //16 times as many frames in energy as in chroma
//length in terms of frames (at 512 samples per frame - there are 90 per second) => 900: 10 seconds
#define ALIGNMENT_FRAMESIZE 128

class TimeWarp : public ofBaseApp{
	
public:
	TimeWarp();											// constructor
	~TimeWarp();

	void initialiseVariables();
	void clearVectors();
	//variables
	typedef std::vector<double> DoubleVector;
	typedef std::vector<DoubleVector> DoubleMatrix;
	
	DoubleMatrix chromaMatrix;
	DoubleMatrix secondMatrix;
	DoubleMatrix* matrixPtr;

	DoubleVector firstEnergyVector;
	DoubleVector secondEnergyVector;	
	
	DoubleMatrix firstChromaEnergyMatrix;
	DoubleMatrix secondChromaEnergyMatrix;
	
	Chromagram chromoGramm;
	Chromagram secondChromoGramm;
	
	DoubleMatrix similarityMatrix;
	DoubleMatrix tmpSimilarityMatrix;
	DoubleMatrix alignmentMeasureMatrix;
	DoubleMatrix tmpAlignmentMeasureMatrix;	
	DoubleVector minimumAlignmentPath;
	
	double partAlignmentMeasureMatrix[ALIGNMENT_FRAMESIZE][ALIGNMENT_FRAMESIZE];
	
	typedef std::vector<int> IntVector;
	typedef std::vector<IntVector> IntMatrix;
	IntMatrix backwardsAlignmentPath;
	IntMatrix tmpBackwardsPath;
	
	int backwardsAlignmentIndex;
	
	IntMatrix partBackwardsAlignmentPath;
	IntMatrix forwardsAlignmentPath;
	IntMatrix anchorPoints;
	void addAnchorPoints(const int&  startFrameX, const int& startFrameY);
	
	int partBackwardsAlignmentIndex;

	
	void createCombinedMatrix(DoubleMatrix myChromaMatrix, DoubleVector energyVector, DoubleMatrix* chromaEnergyMatrix);

	double getChromaSimilarity(int x, int y, DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix);
	double getEuclideanDistance(int x, int y, DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix);
	void calculateChromaSimilarityMatrix(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix);
	void calculateCausalChromaSimilarityMatrix(DoubleMatrix& firstChromaMatrix, DoubleMatrix& secondChromaMatrix, DoubleMatrix& simMatrix);

	
	void calculateSimilarityMatrix();

	int findMinimumOfMatrixColumn(DoubleMatrix d, int column);
	
	
	//new addition
	void calculateSimilarityMatrixWithPointers(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix);

	void calculateJointSimilarityMatrix(DoubleVector* energyVectorOne, DoubleVector* energyVectorTwo, DoubleMatrix* chromaSimilarityMatrix, DoubleMatrix* simMatrix);
	//where we waim to do combined chroma and energy
	
	
	void calculatePartJointSimilarityMatrix(DoubleVector* firstEnergyVector, DoubleVector* secondEnergyVector, DoubleMatrix* chromaSimMatrix, DoubleMatrix* simMatrix, const  int& startX, const int& startY, int endX, int endY);

	
	double getJointChromaAndEnergyDistance(DoubleVector* energyVectorOne, DoubleMatrix* firstChromaMatrix, DoubleVector* energyVectorTwo, DoubleMatrix* secondChromaMatrix, int energyIndexX, int energyIndexY, double energyProportion, double chromaProportion);
	
	DoubleMatrix chromaSimilarityMatrix;
//	DoubleMatrix superAlignmentMeasureMatrix;	//for the onset + chromagram alignment
//	DoubleVector superMinimumAlignmentPath;	
	//end new additions
	
	int findStartWidthFrame();	  

	

	int conversionFactor; 
	
	void calculateAlignmentMatrix(DoubleMatrix firstMatrix, DoubleMatrix secondMatrix, DoubleMatrix *alignmentMatrix);
	double getDistance(int i, int j);

	
	double getMinimum(int i, int j, float newValue);
	bool extendAlignmentUp(const int& endIndexY, DoubleMatrix *alignmentMatrix);
	bool extendAlignmentAlong(const int& endIndexX, DoubleMatrix *alignmentMatrix);
	void calculateMinimumAlignmentPathColumn(DoubleMatrix* alignmentMatrix, IntMatrix* backPath, bool pickMinimumFlag);//writes the backwards laignment path to *backPath

	void calculateMinimumAlignmentPathRow(DoubleMatrix* alignmentMatrix, IntMatrix* backPath, bool pickMinimumFlag);
	
	bool findPreviousMinimumInBackwardsPath(DoubleMatrix* alignmentMatrix, IntMatrix* backPath);
	bool testForNewAlignmentMinimum(double *previousMinimum, const int& i, const int& j, DoubleMatrix* alignmentMatrix);	
	
	int findMinimumOfVector(DoubleVector *d);
		
	void extendForwardAlignmentPath(int endX, IntMatrix* backPath, int anchorPointX, int anchorPointY);//specify forwards path to extend?
	void extendForwardAlignmentPathToYanchor(int endY, IntMatrix* backPath, int anchorPointX, int anchorPointY);
	
	void addNewForwardsPath(int indexX, IntMatrix* backPath, int anchorPointX, int anchorPointY);

	int getMinimumIndexOfColumnFromMatrix(int i, DoubleMatrix* matrix);
	int getMinimumIndexOfRowFromMatrix(int j, DoubleMatrix& matrix);
	
	void addNewForwardsPathFromYindex(const int& indexY, IntMatrix* backPath, const int& anchorPointX, const int& anchorPointY);
//PART ALIGNMENT FUNCTIONS
	void calculatePartSimilarityMatrix(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix, int startX, int startY, int endX);
	void calculatePartAlignmentMatrix(int endIndexX, int endIndexY, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix);

	
	double getDistanceFromMatrix(int i, int j, DoubleMatrix* simMatrix);
	double getMinimumFromMatrix(int i, int j, float newValue, DoubleMatrix* alignMatrix);
	
	void calculatePartMinimumAlignmentPath(int startX, int startY, int endX, int endY, DoubleMatrix alignmentMatrix);
//	bool findPreviousMinimumInPartBackwardsPath(DoubleMatrix* alignmentMatrix);
	double getRestrictedMinimum(int i, int j, float newValue, int minX, int minY);
	bool extendRestrictedAlignmentUp(const int& endIndexY, DoubleMatrix *alignmentMatrix, DoubleMatrix* simMatrix);
	bool extendRestrictedAlignmentAlong(const int& endIndexX, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix);
	
	
	void printBackwardsPath(int startIndex, int endIndex, const IntMatrix* backPath);
	void printForwardsPath();
	
	void copyForwardsPathToBackwardsPath();
	
	float diagonalPenalty;
	bool useDotProduct;
};

#endif
