/*
 *  TimeWarp.cpp
 *  chromaReader13
 *
 *  Created by Andrew on 16/05/2011.
 *  Copyright 2011 QMUL. All rights reserved.
 *
 */

#include "TimeWarp.h"

#include "stdio.h"
#include "aubio.h"
#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib> 

//FIX CHORDS IN THE NEW POINTER VERSION OF THE PROCESS AUDIO FN

//BUG IN WHETHER SECOND SONG LOADED OR NOT


//CHECK THAT DTW will not call beyond the limits of the two chroma if different sizes

//CALC DTW on different sizes

//CHECK CORRECT BEST ALIGNMENT METHOD FOR DTW

//RE-DO DRAW SO THAT IT DOES NOT CALCULATE EVERY POINT BUT DOES IN SQUARE CHUNKS	

//UPDATE START FRAME SO ALIGNMENT IS ALWAYS ON SCREEN
//--------------------------------------------------------------
// destructor
TimeWarp :: TimeWarp(){
	//diagonalPenalty = 1;//favours diagonal over other paths 
	//diagonalPenalty = 2;//penalises diagonal so all path gradients equal weighting
	
	useDotProduct = false;////true - dot, falseo: Euclidean dist
}	

// destructor
TimeWarp :: ~TimeWarp(){
	
	chromaMatrix.clear();
	secondMatrix.clear();
	firstEnergyVector.clear();
	secondEnergyVector.clear();	
	similarityMatrix.clear();
	alignmentMeasureMatrix.clear();
	conversionFactor = 16;
	//matrixPtr.clear();
	//chromoGramm.~ChromoGram();
	//secondChromoGramm;
	anchorPoints.clear();
	
	
}	


void TimeWarp::clearVectors(){
	firstEnergyVector.clear();
	secondEnergyVector.clear();
	chromaMatrix.clear();
	secondMatrix.clear();
	similarityMatrix.clear();
	chromaSimilarityMatrix.clear();
	tmpSimilarityMatrix.clear();
	alignmentMeasureMatrix.clear();
	tmpAlignmentMeasureMatrix.clear();	
	minimumAlignmentPath.clear();
	partBackwardsAlignmentPath.clear();
	forwardsAlignmentPath.clear();
	anchorPoints.clear();
}
	
void TimeWarp::initialiseVariables(){
	similarityMatrix.clear();
	chromaSimilarityMatrix.clear();
	tmpSimilarityMatrix.clear();
	alignmentMeasureMatrix.clear();
	tmpAlignmentMeasureMatrix.clear();	
	minimumAlignmentPath.clear();
	partBackwardsAlignmentPath.clear();
	forwardsAlignmentPath.clear();
	anchorPoints.clear();
	
	//diagonalPenalty = 1;
	//chromoGramm.initialise(FRAMESIZE,2048);//framesize 512 and hopsize 2048

}


void TimeWarp::createCombinedMatrix(DoubleMatrix myChromaMatrix, DoubleVector energyVector, DoubleMatrix* chromaEnergyMatrix){
	chromaEnergyMatrix->clear();
	int sizeRatio = energyVector.size() / myChromaMatrix.size();//
	printf("COMBINE: size of my chroma is %i\n", (int) myChromaMatrix.size());// energyVector.size() / myChromaMatrix.size();
	printf("COMBINED: size ratio of energy to chroma is %i \n", sizeRatio);
	int chromaSize = myChromaMatrix.size();	
//	printf("index is %i\n", index);
	
	for (int i = 0;i < energyVector.size();i++){
		DoubleVector d;
		int index = min(chromaSize-1, (int) floor(i/sizeRatio));

		for (int y = 0;y < 12;y++){
			d.push_back(myChromaMatrix[index][y]);//
		}

		
		d.push_back(energyVector[i]);
					(*chromaEnergyMatrix).push_back(d);
	}
	printf("COMBINED: size of chroma energy is %i\n", (int)(*chromaEnergyMatrix).size());

//	int x = (int)(*chromaEnergyMatrix).size()/3;
//	printf("energy[%i] %f \n", x, energyVector[x]);
/*
 for (int y = 0;y < 13;y++){
		printf("chroma[%i][%i] %f \n", x, y, myChromaMatrix[x/sizeRatio][y]);
		printf("c[%i][%i] %f \n", x, y, (*chromaEnergyMatrix)[x][y]);
	}
		printf("\n");
*/	

}

void TimeWarp::calculateSimilarityMatrix(){
	calculateSimilarityMatrixWithPointers(&chromaMatrix, &secondMatrix, &similarityMatrix);
									  
		/*								  
	similarityMatrix.clear();
	printf("calculating similarity matrix...")
	//	userInfoString = "calculating similarity matrix...";
	
	double distance, firstSum, secondSum;
	
	for (int x = 0;x < chromaMatrix.size();x++){
		DoubleVector d;
		
		
		 for (int y = 0;y < secondMatrix.size();y++){
			
			distance = 0;
			firstSum = 0;
			secondSum = 0;
			
			for (int z = 0;z < chromaMatrix[x].size();z++){//z is the twelve chromagram values
				
				distance += chromaMatrix[x][z] * secondMatrix[y][z];
				
				firstSum += chromaMatrix[x][z] * chromaMatrix[x][z];
				secondSum += secondMatrix[y][z] * secondMatrix[y][z];
			}
			
			if (firstSum > 0 && secondSum > 0)
				distance /= sqrt(firstSum * secondSum);
			
			
			d.push_back( distance);	
			
		}	//end for y
		
		similarityMatrix.push_back(d);
		
	}//end for x
	*/
//	printf("..sim size: %i, height: %i \n", (int) similarityMatrix.size(), (int) (chromaMatrix[0]).size());
	
}//end self sim




void TimeWarp::calculateSimilarityMatrixWithPointers(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix){
	printf("Calculate similarity : pointers : size %i x %i  ", (int) (*firstChromaMatrix).size(), (int) (*secondChromaMatrix).size());
	
	simMatrix->clear();

	double distance, firstSum, secondSum;
	
	for (int x = 0;x < (*firstChromaMatrix).size();x++){
		DoubleVector d;
		
		for (int y = 0;y < (*secondChromaMatrix).size();y++){
			
			if (useDotProduct)
				distance = getChromaSimilarity(x, y, firstChromaMatrix, secondChromaMatrix);
			else
				distance = getEuclideanDistance(x, y, firstChromaMatrix, secondChromaMatrix);
			
			d.push_back( distance);	
		}	//end for y
		
		(*simMatrix).push_back(d);
		
	}//end for x

		 printf("..sim size: %i, height: %i \n", (int) (*simMatrix).size(), (int) (*simMatrix)[0].size());
	
}//end self sim


void TimeWarp::calculateJointSimilarityMatrix(DoubleVector* energyVectorOne, DoubleVector* energyVectorTwo,  DoubleMatrix*  chromaSimilarityMatrix, DoubleMatrix* simMatrix){

	//requires a chromagram similarity first this already done
	//calculateSimilarityMatrixWithPointers(chromaMatrixOne, chromaMatrixTwo, &chromaSimilarityMatrix);
	
	conversionFactor = (int) round((*energyVectorOne).size() / (*chromaSimilarityMatrix).size() );
	
	printf("tw.ROUNDED CONVERSION FACTOR IS %i\n", conversionFactor);
	printf("tw.CHROMA SIM SIZE %i\n", (int) (*chromaSimilarityMatrix).size());
	
	simMatrix->clear();

	double energyProportion = 0.3;
	double chromaProportion = 1 - energyProportion;
	
	double distance, firstSum, secondSum;
	
	//lets try not actually doing calculation
	
	for (int x = 0;x < (*energyVectorOne).size();x++){
		DoubleVector d;
		
		for (int y = 0;y < (*energyVectorTwo).size();y++){
			
			//create chroma similarity first
			
			//			distance = (*energyVectorOne)[x] * (*energyVectorTwo)[y];//energy similarity
			
			//now need the chroma part
			int chromaIndexX = min(x / conversionFactor, (int)(*chromaSimilarityMatrix).size()-1);
			int chromaIndexY = min(y / conversionFactor, (int)(*chromaSimilarityMatrix)[chromaIndexX].size()-1);
			
			double chromaComponent = (*chromaSimilarityMatrix)[chromaIndexX][chromaIndexY];
//			getChromaSimilarity(chromaIndexX, chromaIndexY, firstChromaMatrix, secondChromaMatrix);
				

			distance *= energyProportion;
			distance += chromaProportion * chromaComponent;
						
			//distance = getJointChromaAndEnergyDistance(energyVectorOne, firstChromaMatrix, energyVectorTwo, secondChromaMatrix, x, y, conversionFactor, energyProportion, chromaProportion);
			

			
			d.push_back( distance);	
		}	//end for y
		
		(*simMatrix).push_back(d);
		
	}//end for x
	 
	 printf("..sim size: %i, height: %i \n", (int) (*simMatrix).size(), (int) (*simMatrix)[0].size());
	 

	
}

double TimeWarp::getJointChromaAndEnergyDistance(DoubleVector* energyVectorOne, DoubleMatrix* firstChromaMatrix, DoubleVector* energyVectorTwo, DoubleMatrix* secondChromaMatrix, int energyIndexX, int energyIndexY, double energyProportion, double chromaProportion){
	//create chroma similarity first
	double distance = 0;
	
	if (energyIndexX >= 0 && energyIndexY >= 0 && energyIndexX < (*energyVectorOne).size() && energyIndexY < (*energyVectorTwo).size()){
	
	distance = (*energyVectorOne)[energyIndexX] * (*energyVectorTwo)[energyIndexY];//energy similarity
	
	//now need the chroma part
	int chromaIndexX = min(energyIndexX / conversionFactor, (int)(*firstChromaMatrix).size()-1);
	int chromaIndexY = min(energyIndexY / conversionFactor, (int)(*secondChromaMatrix).size()-1);
	double chromaComponent = getChromaSimilarity(chromaIndexX, chromaIndexY, firstChromaMatrix, secondChromaMatrix);
			
	distance *= energyProportion;
	distance += chromaProportion * chromaComponent;
	}
	return distance;
}



void TimeWarp::calculateChromaSimilarityMatrix(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix){
//calculates the chroma only similarity matrix - used to reduce computation later when doing the joint energy and chroma matrix
	for (int x = 0;x < (*firstChromaMatrix).size();x++){
		DoubleVector d;
		for (int y = 0;y < (*secondChromaMatrix).size();y++){
			double distance;
			if (useDotProduct)
			distance = getChromaSimilarity(x, y, firstChromaMatrix, secondChromaMatrix);
			else
			distance = getEuclideanDistance(x, y, firstChromaMatrix, secondChromaMatrix);
			
			d.push_back( distance);	
		}
		(*simMatrix).push_back(d);
	}
	printf("CHROMA ONLY SIM SIZE %i x %i\n", (int)(*simMatrix).size(), (int)(*simMatrix)[0].size());
	
}

double TimeWarp::getChromaSimilarity(int x, int y, DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix){
	
	double distance = 0;
	double firstSum = 0;
	double secondSum = 0;
	
	if (x >= 0 && x < (*firstChromaMatrix).size() && y >= 0 && y < (*secondChromaMatrix).size()){
		for (int z = 0;z < (*firstChromaMatrix)[x].size();z++){//z is the twelve chromagram values
			
		distance += (*firstChromaMatrix)[x][z] * (*secondChromaMatrix)[y][z];
		firstSum += (*firstChromaMatrix)[x][z] * (*firstChromaMatrix)[x][z];
		secondSum += (*secondChromaMatrix)[y][z] * (*secondChromaMatrix)[y][z];
		}
		
		if (firstSum > 0 && secondSum > 0)
			distance /= sqrt(firstSum)*sqrt(secondSum);
	}
	
	return distance;
	
}


double TimeWarp::getEuclideanDistance(int x, int y, DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix){

	double distance = 0;
	double newDistance = 0;
	
	if (x >= 0 && x < (*firstChromaMatrix).size() && y >= 0 && y < (*secondChromaMatrix).size()){
		for (int z = 0;z < (*firstChromaMatrix)[x].size();z++){//z is the twelve chromagram values
			newDistance = (*firstChromaMatrix)[x][z] - (*secondChromaMatrix)[y][z];
			distance += newDistance*newDistance;
		}
	}
	
	return 1-sqrt(distance);
	
}


void TimeWarp::addAnchorPoints(const int&  startFrameX, const int& startFrameY){
	IntVector v;
	v.push_back(startFrameX);
	v.push_back(startFrameY);
	anchorPoints.push_back(v);
}

void TimeWarp::calculateAlignmentMatrix(DoubleMatrix firstMatrix, DoubleMatrix secondMatrix, DoubleMatrix* alignmentMatrix){//, DoubleMatrix simMatrix
	printf("starting Alignment calculation\n");
	//initialise alignment
	alignmentMatrix->clear();

	DoubleVector d;
	d.push_back(getDistance(0,0));
	(*alignmentMatrix).push_back(d);
	
	bool chromaCalculated = false;
	bool secondCalculated = false;
	
	while (!chromaCalculated || !secondCalculated) {
		
		if (!chromaCalculated)
			chromaCalculated = extendAlignmentAlong((int) firstMatrix.size(), alignmentMatrix);
		
		if (!secondCalculated)
			secondCalculated = extendAlignmentUp((int) secondMatrix.size(), alignmentMatrix);
		
	}
	printf("Alignment matrix calculated, size %i\n", (int) (*alignmentMatrix).size());
}

bool TimeWarp::extendAlignmentUp(int endIndexY, DoubleMatrix *alignmentMatrix){
	DoubleVector d;
	d = (*alignmentMatrix)[0];//alignmentMatrix[0];//
	int heightSize = d.size();
	if (heightSize < endIndexY){
		//then we haven't finished yet
		for (int i = 0;i < (*alignmentMatrix).size();i++){
			double value = getDistance(i, heightSize);
			value += getRestrictedMinimum(i, heightSize, value, 0, 0);//min values 0	
			(*alignmentMatrix)[i].push_back(value);//
		}
	}
	if ((*alignmentMatrix)[0].size() == endIndexY)
		return true;
	else
		return false;
	
}


bool TimeWarp::extendAlignmentAlong(int endIndexX, DoubleMatrix* alignmentMatrix){
	DoubleVector d;
	//firstMatrix.size()
	int widthSize = (*alignmentMatrix).size();
	if (widthSize < endIndexX){//firstMatrix.size()
		//then we can extend along
		double value = getDistance(widthSize, 0);
		value += getRestrictedMinimum(widthSize, 0, value, 0, 0);
		
		d.push_back(value);
		(*alignmentMatrix).push_back(d);
		
		for (int j = 1;j < (*alignmentMatrix)[widthSize - 1].size();j++){
			value = getDistance(widthSize, j);
			value += getMinimum(widthSize, j, value);
			(*alignmentMatrix)[widthSize].push_back(value);
		}
		
	}
	
	if ((*alignmentMatrix).size() == endIndexX)
		return true;
	else
		return false;
	
}



void TimeWarp::calculatePartSimilarityMatrix(DoubleMatrix* firstChromaMatrix, DoubleMatrix* secondChromaMatrix, DoubleMatrix* simMatrix, int startX, int startY, int endX){
//	printf("Calculate similarity : pointers : size %i x %i  ", (int) firstChromaMatrix.size(), (int) secondChromaMatrix.size());
	
	simMatrix->clear();
	
	double distance, firstSum, secondSum;
	endX = min (endX, (int)(*firstChromaMatrix).size()-1);//in case out of size
	
	for (int x = startX;x <= endX;x++){
		DoubleVector d;
		
		for (int y = startY;y < (*secondChromaMatrix).size();y++){

			if (useDotProduct)
				distance  = getChromaSimilarity(x, y, firstChromaMatrix, secondChromaMatrix);
			else
				distance = getEuclideanDistance(x, y, firstChromaMatrix, secondChromaMatrix);
			
			d.push_back( distance);	
		}	//end for y
		
		(*simMatrix).push_back(d);
		
	}//end for x
	
	printf("..part sim size: %i, height: %i \n", (int) (*simMatrix).size(), (int) (*simMatrix)[0].size());
	
}//end self sim




void TimeWarp::calculatePartJointSimilarityMatrix(DoubleVector* firstEnergyVector, DoubleVector* secondEnergyVector, DoubleMatrix* chromaSimMatrix, DoubleMatrix* simMatrix, int startX, int startY, int endX, int endY){
	printf("PART SIM CALC Calculate similarity : pointers : size %i x %i  ", startX, startY);//(int) (*firstEnergyVector).size(), (int) (*secondEnergyVector).size());
	
	conversionFactor = (int) round((*firstEnergyVector).size() / (*chromaSimMatrix).size() );
	simMatrix->clear();
	
	double energyProportion = 0.2;
	double chromaProportion = 1 - energyProportion;
	double distance, firstSum, secondSum;
	
	endX = min (endX, (int)(*firstEnergyVector).size()-1);//in case out of size
	endY = min( endY+1, (int)(*secondEnergyVector).size());
	int lastChromaYvalue = 0;
	int chromaIndexY = 0;
	double chromaComponent = 0;
	double 	chromaContribution = 0;
		DoubleVector d;	
	for (int x = startX;x <= endX;x++){
		d.clear();
		
		//now need the chroma part
		int chromaIndexX = min(x / conversionFactor, (int)(*chromaSimMatrix).size()-1);
		
		for (int y = startY;y < endY;y++){
			chromaIndexY = min(y / conversionFactor, (int)(*chromaSimMatrix)[chromaIndexX].size()-1);
			
			//was thinking to restrict the y part too, but not working yet
			if (chromaIndexY != lastChromaYvalue){			
			
			chromaComponent = (*chromaSimMatrix)[chromaIndexX][chromaIndexY];
				lastChromaYvalue = chromaIndexY;
				chromaContribution  = chromaProportion * chromaComponent;
			}
			
			distance = (*firstEnergyVector)[x] * (*secondEnergyVector)[y];//energy similarity
			distance *= energyProportion;
			distance += chromaContribution;
			
			distance = chromaComponent;
			
			d.push_back( distance);	
		}	//end for y
		
		(*simMatrix).push_back(d);
		
	}//end for x
	
	
	printf("..part JOINT sim size: %i, height: %i \n", (int) (*simMatrix).size(), (int) (*simMatrix)[0].size());
	
}//end self sim





void TimeWarp::calculatePartAlignmentMatrix(int endIndexX, int endIndexY, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix){
	printf("starting PART Alignment calculation : sim matrix size %i %i\n", (int)(*simMatrix).size(), (int)(*simMatrix)[0].size());
	//initialise alignment
	alignmentMatrix->clear();
	
	DoubleVector d;
	d.push_back(getDistanceFromMatrix(0,0, simMatrix));
		printf("first distance\n");
	(*alignmentMatrix).push_back(d);
	
	bool chromaCalculated = false;
	bool secondCalculated = false;
	
	while (!chromaCalculated || !secondCalculated) {
		
		if (!chromaCalculated)
			chromaCalculated = extendRestrictedAlignmentAlong(endIndexX, alignmentMatrix, simMatrix);
		
		if (!secondCalculated)
			secondCalculated = extendRestrictedAlignmentUp(endIndexY, alignmentMatrix, simMatrix);
		
	}
	printf("PART Alignment matrix calculated, size %i by %i\n", (int) (*alignmentMatrix).size() , (int) (*alignmentMatrix)[0].size());
}






bool TimeWarp::extendRestrictedAlignmentUp(int endIndexY, DoubleMatrix *alignmentMatrix, DoubleMatrix* simMatrix){
	//adds one more value to all the columns after startX
	
	DoubleVector d;
	d = (*alignmentMatrix)[0];//alignmentMatrix[0];//
	int heightSize = d.size();
	if (heightSize < endIndexY){
		//would change 0 to startX if we varied this
		for (int i = 0;i < (*alignmentMatrix).size();i++){//was < (*alignmentMatrix).size()
		//	printf("restruicted up  %i, %i\n", i, heightSize);
			double value = getDistanceFromMatrix(i, heightSize, simMatrix);
			value += getMinimumFromMatrix(i, heightSize, value, alignmentMatrix);//min values 0	
			(*alignmentMatrix)[i].push_back(value);//
		}
	}
	if ((*alignmentMatrix)[0].size() == endIndexY)
		return true;
	else
		return false;
	
}


bool TimeWarp::extendRestrictedAlignmentAlong(int endIndexX, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix){
	DoubleVector d;
	//firstMatrix.size()
	int widthSize = (*alignmentMatrix).size();
	if (widthSize < endIndexX){//firstMatrix.size()
	//	printf("restruicted along %i\n", widthSize);
		//then we can extend along
		double value = getDistanceFromMatrix(widthSize, 0, simMatrix);
		value += getMinimumFromMatrix(widthSize, 0, value, alignmentMatrix);
		
		d.push_back(value);
		(*alignmentMatrix).push_back(d);
		
		for (int j = 1;j < (*alignmentMatrix)[widthSize - 1].size();j++){
		//	printf("restruicted along %i %i\n", widthSize, j);
			value = getDistanceFromMatrix(widthSize, j, simMatrix);
			value += getMinimumFromMatrix(widthSize, j, value, alignmentMatrix);
			(*alignmentMatrix)[widthSize].push_back(value);
		}
		
	}
	
	if ((*alignmentMatrix).size() == endIndexX)
		return true;
	else
		return false;
	
}



void TimeWarp::calculateMinimumAlignmentPath(DoubleMatrix* alignmentMatrix, IntMatrix* backPath, bool pickMinimumFlag){
	//this requires one pass of the DTW algorithm and then works backwards from (N,M)
	//to find the optimal path to (0,0), where N and M are the lengths of the two chromoVectors respectively
	
	(*backPath).clear();
	
//	printf("Finding minimum Path %i vs sim size %i\n", (int)chromaMatrix.size(), (int)similarityMatrix.size() );
	
	printf("Finding minimum Path of alignment matrix %i vs sim size %i\n", (int)(*alignmentMatrix).size(), (int)(*alignmentMatrix)[0].size() );	
//	printf("compares to sim %ix%i\n", similarityMatrix.size()-1, similarityMatrix[0].size()-1);
	IntVector v;
//	v.push_back(similarityMatrix.size()-1);//chromaMatrix.size()-1 - old way
	
	//here we start at the corner of the alignment matrix
	//could change to greedy? - i.e. the best / minimum of the last vector
	v.push_back((*alignmentMatrix).size()-1);
	(*backPath).push_back(v);
	v.clear();
	//v.push_back(similarityMatrix[0].size()-1);//secondMatrix
	int endIndex = (*alignmentMatrix)[(*alignmentMatrix).size()-1].size()-1;
	if (pickMinimumFlag){
		endIndex = getMinimumIndexOfColumnFromMatrix((int)(*alignmentMatrix).size()-1, alignmentMatrix);
		//i.e. get index of minimum in the last column
	}
		v.push_back(endIndex);//and the y size
	printf("CALUCLATE MINIMUM PUSHED BACK %i\n", endIndex);
	
	
	(*backPath).push_back(v);
	//so now backwards path[0][0] = size(chroma) and path[1][0] = size(secondMatrix)
printf("backwards path initialised to %i : %i \n", (*backPath)[0][0], (*backPath)[1][0]);
	
	
	int indexOfBackwardsPath = 0;
	while (!findPreviousMinimumInBackwardsPath(alignmentMatrix, backPath))	{
		indexOfBackwardsPath++;
//		printf("backwards path index %i:  path: %i : %i \n", indexOfBackwardsPath, backwardsAlignmentPath[0][indexOfBackwardsPath], backwardsAlignmentPath[1][indexOfBackwardsPath]);
		
	}
	printf("final index of backwards path is %i and i is %i \n", (int) (*backPath)[0].size()-1, indexOfBackwardsPath);
	
	//	backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;//remember that this goes backwards!
	
}



bool TimeWarp::findPreviousMinimumInBackwardsPath(DoubleMatrix* alignmentMatrix, IntMatrix* backPath){
	int chromaPosition, secondPosition;
	int i,j;
	i = (*backPath)[0][(*backPath)[0].size()-1];
	j  = (*backPath)[1][(*backPath)[1].size()-1];
	//printf("FIND PREVIOUS MINIMUM %i %i \n", i, j);
	
	double newMinimum;
	double *ptr;
	ptr = &newMinimum;
	newMinimum = (*alignmentMatrix)[i][j];
	DoubleVector d;
	
	
	bool finishedAligning = true;
	
	if (i > 0){
		if (testForNewAlignmentMinimum(ptr, i-1, j, alignmentMatrix)){
			chromaPosition = i-1;
			secondPosition = j;
			finishedAligning = false;
		}
		
		if (j>0 && testForNewAlignmentMinimum(ptr, i-1, j-1, alignmentMatrix)){
			chromaPosition = i-1;
			secondPosition = j-1;
			finishedAligning = false;
		}
	}
	
	if (j > 0 && testForNewAlignmentMinimum(ptr, i, j-1, alignmentMatrix)){
		chromaPosition = i;
		secondPosition = j-1;
		//newMinimum = alignmentMeasureMatrix[chromaPosition][secondPosition];
		finishedAligning = false;
	}
	
	if (!finishedAligning){
		(*backPath)[0].push_back(chromaPosition);
		(*backPath)[1].push_back(secondPosition);
	}
	
	return finishedAligning;
	
}	



bool TimeWarp::testForNewAlignmentMinimum(double *previousMinimum, int i, int j, DoubleMatrix* alignmentMatrix){
	bool newMinimumFound = false;
	if ((*alignmentMatrix)[i][j] < *previousMinimum){
		*previousMinimum = (*alignmentMatrix)[i][j];							   
		newMinimumFound = true;
	}
	
	return newMinimumFound;							   
}															





void TimeWarp::printBackwardsPath(int startIndex, int endIndex, const IntMatrix* backPath){
	if (endIndex <= (*backPath)[0].size()){
		printf("size of path is %i by %i\n", (int) (*backPath).size(), (int) (*backPath)[0].size());
	for (int i = startIndex;i < endIndex;i++){
		printf("Path[%i]:: %i : %i \n", i, (*backPath)[0][i], (*backPath)[1][i]);
		}
	}
}


void TimeWarp::extendForwardAlignmentPath(int endX, IntMatrix* backPath, int anchorPointX, int anchorPointY){
	//andchor points are the starting index so if we have already done up to 
int forwardsIndex = forwardsAlignmentPath.size();
	int indexX = (*backPath)[0].size() - 1;
	
	if (forwardsIndex == 0){
		printf("initialise forwards path..\n");
	IntVector v;
	
	v.push_back((*backPath)[0][indexX]);//chromaMatrix.size()-1
	forwardsAlignmentPath.push_back(v);
	v.clear();
	v.push_back(forwardsAlignmentPath[0][indexX]);//secondMatrix
	forwardsAlignmentPath.push_back(v);
	indexX--;
	printf("FORWARDS PATH STARTED AS %i, %i\n", forwardsAlignmentPath[0][0], forwardsAlignmentPath[0][1]);
	}
	else{
	//forwards path has been started and we need anchor point
		
	}
	
	
	while ((*backPath)[0][indexX] <= endX){
		addNewForwardsPath(indexX, backPath, anchorPointX, anchorPointY);
	//	printf("Forwards path from index %i:: path %i : %i\n", indexX, forwardsAlignmentPath[0][forwardsIndex], forwardsAlignmentPath[1][forwardsIndex]);
		indexX--;
		forwardsIndex++;	   
	}

}

void TimeWarp::addNewForwardsPath(int indexX, IntMatrix* backPath, int anchorPointX, int anchorPointY){
	//pushes back
	if (indexX < (*backPath)[0].size()){
	forwardsAlignmentPath[0].push_back((int)(*backPath)[0][indexX]+ anchorPointX);
	forwardsAlignmentPath[1].push_back((int)(*backPath)[1][indexX] + anchorPointY);
	}
}



void TimeWarp::copyForwardsPathToBackwardsPath(){

	backwardsAlignmentPath.clear();
	
	int index = forwardsAlignmentPath[0].size()-1;
	printf("COPY FORWARDS INDEX %i\n", index);
	IntVector d;
	d.push_back(forwardsAlignmentPath[0][index]);
	backwardsAlignmentPath.push_back(d);
	d.clear();
	d.push_back(forwardsAlignmentPath[1][index]);
	backwardsAlignmentPath.push_back(d);
	
	while (index > 0){
		index--;
	//	IntVector d;
	//	d.push_back(forwardsAlignmentPath[0][index]);
	//	d.push_back(forwardsAlignmentPath[1][index]);
		backwardsAlignmentPath[0].push_back(forwardsAlignmentPath[0][index]);
		backwardsAlignmentPath[1].push_back(forwardsAlignmentPath[1][index]);
	}
	
}

/*
//DONT NEED THIS
void TimeWarp::calculatePartMinimumAlignmentPath(int startX, int startY, int endX, int endY, DoubleMatrix alignmentMatrix){
	//this requires one pass of the DTW algorithm and then works backwards from (N,M)
	//to find the optimal path to (0,0), where N and M are the lengths of the two chromoVectors respectively
	
	partBackwardsAlignmentPath.clear();
	
	printf("Finding PART minimum Path %i vs sim size %i\n", (int)chromaMatrix.size(), (int)similarityMatrix.size() );
	
	if (endX < similarityMatrix.size()){
		IntVector v;
		v.push_back(endX);
		partBackwardsAlignmentPath.push_back(v);
		
		v.clear();
	
		if (endY < similarityMatrix[0].size()){
		v.push_back(endY);
		partBackwardsAlignmentPath.push_back(v);
		}
	//so now backwards path[0][0] = endX and path[1][0] = endY
		
	printf("PART backwards path %i : %i \n", partBackwardsAlignmentPath[0][0], partBackwardsAlignmentPath[1][0]);
	int indexOfPartBackwardsPath = 0;
	
	
	while (!findPreviousMinimumInBackwardsPath())	{
		indexOfBackwardsPath++;
		//		printf("backwards path %i : %i \n", backwardsAlignmentPath[0][indexOfBackwardsPath], backwardsAlignmentPath[1][indexOfBackwardsPath]);
		
	}
	 
	printf("final index of backwards path is %i and i is %i \n", (int) backwardsAlignmentPath[0].size()-1, indexOfPartBackwardsPath);
	}//end  if endX within size
	
	//	backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;//remember that this goes backwards!
	
}

*/


int TimeWarp::findMinimumOfVector(DoubleVector *d){
	int minimumIndex = 0;
	double minimumValue = (*d)[0];
	for (int i = 0;i < d->size();i++){
		if ((*d)[i] < minimumValue){
			minimumIndex = i;
			minimumValue = (*d)[i];
		}
	}
	
	return minimumIndex;
}



int TimeWarp::findMinimumOfMatrixColumn(DoubleMatrix d, int column){
	int minimumIndex = 0;
	
	double minimumValue = d[column][0];
	for (int i = 0;i < d.size();i++){
		if (d[column][i] < minimumValue){
			minimumIndex = i;
			minimumValue = d[column][i];
		}
	}
	
	return minimumIndex;
}



double TimeWarp::getDistance(int i, int j){
	return (1 - similarityMatrix[i][j]);
}

double TimeWarp::getMinimum(int i, int j, float newValue){
	double minimumValue = 0;
	
	if (i > 0){
		minimumValue = alignmentMeasureMatrix[i-1][j];
		if (j > 0){
			minimumValue = min(minimumValue, alignmentMeasureMatrix[i-1][j-1] + newValue ) ;//penalises diagonal by 2
			minimumValue = min(minimumValue, alignmentMeasureMatrix[i][j-1]);
		}
	}
	else{//i.e. i == 0 
		if (j > 0)
			minimumValue = alignmentMeasureMatrix[i][j-1];
	}
	
	return minimumValue;
}




double TimeWarp::getDistanceFromMatrix(int i, int j, DoubleMatrix* simMatrix){
	//	return (1 - (*simMatrix)[i][j]);
	
	if (i < (*simMatrix).size() && j < (*simMatrix)[i].size()){
	//	printf("distance returning %i% i : %f\n", i, j, (1 - (*simMatrix)[i][j]) );
		return (1 - (*simMatrix)[i][j]);
	}
	else{
		printf("ERROR IN MATRIX CALCULATION - OUT OF LIMITS! %i, %i, size of sim matrix: %ix%i\n", i, j, (int)(*simMatrix).size(), (int)(*simMatrix)[i].size());
		return 0;
	}
	 
}

double TimeWarp::getMinimumFromMatrix(int i, int j, float newValue, DoubleMatrix* alignMatrix){
	double minimumValue = 0;
	if (i > 0){
		minimumValue = (*alignMatrix)[i-1][j];
		if (j > 0){
			//minimumValue = min(minimumValue, (*alignMatrix)[i-1][j-1] + newValue ) ;//penalises diagonal by 2
			minimumValue = min(minimumValue, (*alignMatrix)[i-1][j-1]  ) ;//favours diagonal
			minimumValue = min(minimumValue, (*alignMatrix)[i][j-1]);
		}
	}
	else{//i.e. i == 0 
		if (j > 0)
			minimumValue = (*alignMatrix)[i][j-1];
	}
	return minimumValue;
}




int TimeWarp::getMinimumIndexOfColumnFromMatrix(int i, DoubleMatrix* matrix){
	int minimumIndex = 0;
	double minimumValue;
	if (i >= 0 && i < (*matrix).size()){
		int height = (*matrix)[i].size();
		int j = 0;
		minimumValue = (*matrix)[i][j];
		while (j < height){
			if ((*matrix)[i][j] < minimumValue){
			minimumIndex = j;
			minimumValue = (*matrix)[i][j];
			}//end if new value
			j++;
		}
	}else{
		printf("ERROR FROM GETTING MINIMIM!!! - zero - out of bounds, received %i and size is %i\n", i, (*matrix).size());
	}
		return minimumIndex;
}



double TimeWarp::getRestrictedMinimum(int i, int j, float newValue, int minX, int minY){
	double minimumValue = 0;
	
	if (i > minX){
		minimumValue = alignmentMeasureMatrix[i-1][j];
		if (j > minY){
			minimumValue = min(minimumValue, alignmentMeasureMatrix[i-1][j-1] + newValue ) ;//penalises diagonal by 2
			minimumValue = min(minimumValue, alignmentMeasureMatrix[i][j-1]);
		}
	}
	else{//i.e. i == 0 
		if (j > minY)
			minimumValue = alignmentMeasureMatrix[i][j-1];
	}
	
	return minimumValue;
}



//--------------part backwards alignment------------------------
/*
bool TimeWarp::findPreviousMinimumInPartBackwardsPath(){
	
	int firstPosition, secondPosition;
	int i,j;
	i = backwardsAlignmentPath[0][backwardsAlignmentPath[0].size()-1];
	j  = backwardsAlignmentPath[1][backwardsAlignmentPath[1].size()-1];
	
	double newMinimum;
	double *ptr;
	ptr = &newMinimum;
	newMinimum = alignmentMeasureMatrix[i][j];
	DoubleVector d;
	
	
	bool finishedAligning = true;
	
	if (i > 0){
		if (testForNewAlignmentMinimum(ptr, i-1, j)){
			firstPosition = i-1;
			secondPosition = j;
			finishedAligning = false;
		}
		
		if (j>0 && testForNewAlignmentMinimum(ptr, i-1, j-1)){
			firstPosition = i-1;
			secondPosition = j-1;
			finishedAligning = false;
		}
	}
	
	if (j > 0 && testForNewAlignmentMinimum(ptr, i, j-1)){
		firstPosition = i;
		secondPosition = j-1;
		//newMinimum = alignmentMeasureMatrix[chromaPosition][secondPosition];
		finishedAligning = false;
	}
	
	if (!finishedAligning){
		backwardsAlignmentPath[0].push_back(firstPosition);
		backwardsAlignmentPath[1].push_back(secondPosition);
	}
	
	return finishedAligning;
	
}	

*/




//--------------------------------------------------------------
/*
void TimeWarp::update(){
	textString = "energy index [";
	textString += ofToString(xIndex);
	textString += "] = ";
	textString += ofToString(energy[xIndex]);
	
	chordString = "Chord : ";
	chordString += ofToString(rootChord[currentPlayingFrame/CHROMA_CONVERSION_FACTOR]);
	
	if (firstAudioFilePlaying){
		audioPosition = (*playingAudio).getPosition() * firstEnergyVector.size();
		updateAlignmentPathIndex(0);
	}
	else {
		audioPosition = (*playingAudio).getPosition() * secondEnergyVector.size();	
		updateAlignmentPathIndex(1);
	}
	
	//if(!audioPaused)
	//printScoreForRow(audioPosition/CHROMA_CONVERSION_FACTOR, (audioPosition/CHROMA_CONVERSION_FACTOR)+10);
	
	//the position in number of frames
	//totalNumberOfFrames was used but is the most recently loaded file length
	
	currentPlayingFrame = audioPosition;
	audioPosition = (int) audioPosition % scrollWidth ;
	audioPosition /= scrollWidth;
	
	ofSoundUpdate();
	
	
}
*/

/*
 void TimeWarp::updateAlignmentPathIndex(int identifier){
	
//	int chromaPosition = audioPosition/CHROMA_CONVERSION_FACTOR;
	
	while (backwardsAlignmentPath[identifier][backwardsAlignmentIndex] < chromaPosition)
	{
		backwardsAlignmentIndex--;
	}
	
}
 */

//--------------------------------------------------------------
/*
 void TimeWarp::draw(){
	
	if (drawSimilarity)
		drawSimilarityMatrix();
	else
		drawChromoGram();
	
	
}


void TimeWarp::drawEnergyVectorFromPointer(DoubleVector* energyVec){
	
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 8;
	int i, j, startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	
	for (i = 0; i < scrollWidth - 1; i++){
		j = i + startingFrame;
		ofLine(i*screenWidth/scrollWidth, screenHeight - ((*energyVec)[j]*screenHeight/heightFactor),
			   screenWidth*(i+1)/scrollWidth, screenHeight - ((*energyVec)[j+1]*screenHeight/heightFactor));
		
	}
}

void TimeWarp::drawSpectralDifference(DoubleMatrix* dMatrix){
	if ((*dMatrix).size()>0){
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();
		float heightFactor = 8;
		double difference;
		int i, j, startingFrame;
		startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= CHROMA_CONVERSION_FACTOR; //in terms of chroma frames
		
		
		for (i = 1; i < chromoLength; i++){//changed to add 1
			j = i + startingFrame;
			for (int y = 0;y < 12;y++){			
				difference = (*dMatrix)[j][11-y] - (*dMatrix)[j-1][11-y];
				if (difference < 0)
					difference = 0;//half wave rectify
				
				ofSetColor(0,0,255 * difference);//, 0;
				ofRect(i*screenWidth/chromoLength,y*screenHeight/12,screenWidth/chromoLength,screenHeight/12);
			}//end y
		}//end i
		
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
}


void TimeWarp::drawChromoGram(){
	
	DoubleMatrix* dptr;
	DoubleVector* eptr;
	string whichFileString;
	
	if (drawSecondMatrix){
		
		dptr = &secondMatrix;
		
		eptr = &secondEnergyVector;
		
		whichFileString = "second file";
		
	}else {
		
		dptr = &chromaMatrix;
		eptr = &firstEnergyVector;
		whichFileString = "first file";
	}
	
	
	
	if (drawSpectralDifferenceFunction)
		drawSpectralDifference(dptr);
	else
		drawDoubleMatrix(dptr);
	
	ofSetColor(0xFF6666);
	drawEnergyVectorFromPointer(eptr);
	
	ofDrawBitmapString(textString,80,480);
	
	
	ofSetColor(0xFFFFFF);
	ofLine(audioPosition*width, 0, audioPosition*width, height);
	
	
	ofDrawBitmapString(chordString,80,580);
	
	ofDrawBitmapString(soundFileName,80,480);
	
	ofDrawBitmapString(whichFileString,80,80);
	
}

void TimeWarp::drawDoubleMatrix(DoubleMatrix* dMatrix){
	if ((*dMatrix).size()>0){
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();
		float heightFactor = 8;
		int i, j, startingFrame;
		startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= CHROMA_CONVERSION_FACTOR; //in terms of chroma frames
		
		float chromoLength = scrollWidth/CHROMA_CONVERSION_FACTOR;
		for (i = 0; i < chromoLength; i++){
			j = i + startingFrame;
			for (int y = 0;y < 12;y++){
				ofSetColor(0,0,255 * (*dMatrix)[j][11-y]);//, 0;
				ofRect(i*screenWidth/chromoLength,y*screenHeight/12,screenWidth/chromoLength,screenHeight/12);
			}//end y
		}//end i
		
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
	
}


void TimeWarp::drawSimilarityMatrix(){
	
	int simHeight = (similarityMatrix[0]).size();
	int simWidth = similarityMatrix.size();
	
	int sizeOfMatrix = chromaMatrix.size();
	int sizeOfSecondMatrix = secondMatrix.size();
	
	int startingXframe = backwardsAlignmentPath[0][backwardsAlignmentIndex] / (scrollWidth/CHROMA_CONVERSION_FACTOR);
	int startingYframe = backwardsAlignmentPath[1][backwardsAlignmentIndex] / (scrollWidth/CHROMA_CONVERSION_FACTOR);
	
	int startingFrame = findStartWidthFrame();
	startingFrame = numberOfScrollWidthsForFirstFile * scrollWidth/CHROMA_CONVERSION_FACTOR;
	
	startingXframe = startingXframe * scrollWidth/CHROMA_CONVERSION_FACTOR;
	startingYframe = startingYframe * scrollWidth/CHROMA_CONVERSION_FACTOR;
	//need to fix for second file too
	
	int *indexOfAlignmentPathTested;
	int lengthOfPath = backwardsAlignmentPath[0].size()-1;
	indexOfAlignmentPathTested = &lengthOfPath;
	
	int xcoord;
	for (int x = 0;x < screenWidth;x++)
	{
		for (int y =0;y < screenHeight;y++){
			
			xcoord = (x / screenWidth) * chromoLength;//was simWidth
			//xcoord += startingFrame;
			xcoord += startingXframe;
			
			int ycoord = y * chromoLength/ screenHeight;
			//ycoord += startingFrame;
			ycoord += startingYframe;
			
			int colour = 0;
			//int ycoord = y * simHeight/ screenHeight;
			//y += startingFrame;
			if (xcoord < sizeOfMatrix && ycoord < sizeOfSecondMatrix)
				colour = similarityMatrix[xcoord][ycoord]*255;
			
			
			ofSetColor(colour,0,0);
			
			ofRect(x,y,1,1);
			
		}
	}
	
	drawAlignmentPath(startingXframe, startingYframe);
	
	//SET TEXT
	string textString;
	textString = "width : ";
	textString += ofToString(simWidth);
	
	textString += "  height : ";
	textString += ofToString(simHeight);
	
	textString += "  startframe : ";
	textString += ofToString(startingFrame);
	
	textString += "  Xframe : ";
	textString += ofToString(startingXframe);
	
	textString += "  Yframe : ";
	textString += ofToString(startingYframe);
	
	textString += "  currentFrame : ";
	textString += ofToString(currentPlayingFrame);
	
	textString += "  scrollwidth: ";
	textString += ofToString(scrollWidth);
	
	textString += "  xcoord: ";
	textString += ofToString(xcoord);
	
	textString += "  Clength: ";
	textString += ofToString(chromoLength);
	
	textString += "  no.Scrolls: ";
	textString += ofToString(numberOfScrollWidthsForFirstFile);
	//END SET TEXT
	
	ofSetColor(0x0000FF);
	if (firstAudioFilePlaying){
		ofLine(audioPosition*screenWidth, 0, audioPosition*screenWidth, height);
		checkIfAudioPositionExceedsWidthForFirstFile();	
		
		//draw values:
		xcoord = currentPlayingFrame / CHROMA_CONVERSION_FACTOR;
		ofSetColor(255, 255, 255);
		for (int y = 0;y < chromoLength; y+=max(1, (int)(20 * chromoLength / screenHeight))){
			
			float value = alignmentMeasureMatrix[xcoord][y+startingYframe];
			int ycoord = y * screenHeight/chromoLength;
			ofDrawBitmapString(ofToString(value, 2) , audioPosition*screenWidth , ycoord);
		}
	}
	else{
		ofLine(0, audioPosition*screenHeight, screenWidth, audioPosition*screenHeight);	
	}
	
	ofDrawBitmapString(textString,80,580);
	
	ofDrawBitmapString(userInfoString,80,80);
	
}
 
 
 
 void TimeWarp::drawAlignmentPath(int startingChromaXFrame, int startingChromaYFrame){
 //draw alignment path
 int endingChromaXFrame = startingChromaXFrame + chromoLength;
 int endingChromaYFrame = startingChromaYFrame + chromoLength;
 
 float chromoWidth = screenWidth / chromoLength;
 float chromoHeight = screenHeight / chromoLength;
 
 int index = backwardsAlignmentPath[0].size()-1;
 //OPTIMISE XXX
 
 
 while (backwardsAlignmentPath[0][index] < startingChromaXFrame){
 index --;
 }
 
 int printIndex = index;
 int backAlign = backwardsAlignmentPath[0][index];
 int printxcoord;
 int xcoord;
 
 while (backwardsAlignmentPath[0][index] < endingChromaXFrame) {
 xcoord = backwardsAlignmentPath[0][index];
 int ycoord = backwardsAlignmentPath[1][index];
 
 printxcoord = xcoord;
 int colour = similarityMatrix[xcoord][ycoord]*255;
 
 float value = alignmentMeasureMatrix[xcoord][ycoord] ;
 
 
 xcoord -= startingChromaXFrame;
 ycoord -= startingChromaYFrame;
 ofSetColor(0,0,colour);
 ofRect(xcoord*chromoWidth, ycoord*chromoHeight, chromoWidth, chromoHeight);
 //		ofSetColor(255, 255, 255);
 //		ofDrawBitmapString(ofToString(value, 2), xcoord*chromoWidth, ycoord*chromoHeight);
 index--;
 }
 
 //	drawHoverAlignmentValues();
 //	printf("ALIGN score :[%i] : %f \n", backwardsAlignmentPath[1][backwardsAlignmentIndex], alignmentMeasureMatrix[ backwardsAlignmentPath[0][backwardsAlignmentIndex] ][ (int) backwardsAlignmentPath[1][backwardsAlignmentIndex] ]);
 
 
 //SET TEXT
 string textString;
 textString = "ALIGNMENT PATH  ";
 
 textString += "backward A index ";
 textString += ofToString(backwardsAlignmentIndex);
 
 textString += "  starting X frame ";
 textString += ofToString(startingChromaXFrame);
 
 textString += "  initial xcoord ";
 textString += ofToString(printxcoord);
 
 textString += "  first index ";
 textString += ofToString(printIndex);
 
 textString += "  backalign[index] ";
 textString += ofToString(backAlign);
 
 textString += "  final xcoord ";
 textString += ofToString(xcoord);
 
 
 
 
 ofSetColor(255,255,255); 
 ofDrawBitmapString(textString,80,640);
 
 }
 
 
 
*/
/*
void  TimeWarp::checkIfAudioPositionExceedsWidthForFirstFile()
{
	if (currentPlayingFrame > scrollWidth*(numberOfScrollWidthsForFirstFile+1))
		numberOfScrollWidthsForFirstFile++;
}

int TimeWarp::findStartWidthFrame(){
	int startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;//starting frame in terms of energy frames
	startingFrame /= CHROMA_CONVERSION_FACTOR; 
	
	return startingFrame;
}

*/

/*
void TimeWarp::loadSoundFiles(){
	
	//assume libsndfile looks in the folder where the app is run
	//therefore ../../../ gets to the bin folder
	//we then need data/sounds/to get to the sound folder
	//this is different to the usual OF default folder
	//was const char	
	const char	*infilename = "../../../data/sound/1-01BachBWV 846.wav";	
	loadLibSndFile(infilename);
	
	string loadfilename = "sound/1-01BachBWV 846.wav";//PicturesMixer6.aif";	
	loadedAudio.loadSound(loadfilename);
	playingAudio = &loadedAudio;
	
}

void TimeWarp::loadLibSndFile(const char *infilename){
	
	if (!sf_close(infile)){
		printf("closed sndfile okay \n");
	}
	
	// Open Input File with lib snd file
    if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
    {   // Open failed
        printf ("SF OPEN routine Not able to open input file %s.\n", infilename) ;
        // Print the error message from libsndfile. 
        puts (sf_strerror (NULL)) ;
		
	} else{
		printf("SF OPEN opened file %s okay.\n", infilename);
		sndfileInfoString = "Opened okay ";
		
	};
	
}
 */
/*
void TimeWarp::processAudioToDoubleMatrix(Chromagram* chromaG, DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	//wendy
	myDoubleMatrix->clear();
	energyVector->clear();
	
	energyIndex = 0;
	
	chromaG->initialise(FRAMESIZE,2048);//framesize 512 and hopsize 2048 
	chromaG->maximumChromaValue = 0;
	
	int readcount = 1; // counts number of samples read from sound file
	printf("processing audio from doublematrix \n");
	printf("readcount %i", readcount);
	while(readcount != 0 && moveOn == true)
	{
		
		// read FRAMESIZE samples from 'infile' and save in 'data'
		readcount = sf_read_float(infile, frame, FRAMESIZE);
		//processing frame - downsampled to 11025Hz
		//8192 samples per chroma frame
		
		chromaG->processframe(frame);
		
		if (chromaG->chromaready)
		{
			DoubleVector d;
			
			for (int i = 0;i<12;i++){
				//chromoGramVector[chromaIndex][i] = chromoGramm.rawChroma[i] / chromoGramm.maximumChromaValue;
				d.push_back(chromaG->rawChroma[i]);// / chromaG->maximumChromaValue);	
				
			}	
			//this would do chord detection
			
			myDoubleMatrix->push_back(d);
			
			
		}//end if chromagRamm ready
		

		
		putEnergyInFrame();
		//get energy of the current frame and wait
		double energyValue = getEnergyOfFrame();
		energyVector->push_back(energyValue);
		
		
	}//end while readcount
	
	printf("Max chroma value is %f \n", chromaG->maximumChromaValue);
	
	//normalise
	int length = myDoubleMatrix->size();
	printf("length of chromagram is %d frames\n", length);
	length = (*myDoubleMatrix)[0].size();
	printf("height of dmatrix is %d\n", length);
	
	for (int i = 0; i < myDoubleMatrix->size();i++){
		for (int j = 0; j < ((*myDoubleMatrix)[0]).size();j++){
			(*myDoubleMatrix)[i][j] /= chromaG->maximumChromaValue;	
		}
	}
	
	int size;
	size = energyVector->size();
	printf("size of energy vector is %d \n", size);
	
	
	//	int size = myDoubleMatrix->size() * CHROMA_CONVERSION_FACTOR;
	//	printf("size of double matrix is %d and frame index %d", size, frameIndex);
	
//	printf("Total frames %i energy index %i and Chroma index %i \n", frameIndex, energyIndex, chromaIndex);
	
	
}
*/

//--------------------------------------------------------------
/*
void TimeWarp::keyPressed  (int key){
	if (key == '-'){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+'){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	
	if (key == OF_KEY_DOWN){
		if (scrollWidth > 600)
			scrollWidth += 400;
		else
			scrollWidth *= 2;
		
		chromoLength = scrollWidth/CHROMA_CONVERSION_FACTOR;
	}
	
	if (key == OF_KEY_UP){
		if (scrollWidth > 600)
			scrollWidth -= 400;
		else
			scrollWidth /= 2;
		
		chromoLength = scrollWidth/CHROMA_CONVERSION_FACTOR;
	}
	
	if (key == OF_KEY_LEFT){
		
		(*playingAudio).setSpeed(-2);
		backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;
	}
	
	if (key == OF_KEY_RIGHT){
		
		(*playingAudio).setSpeed(2);
	}
	
	if (key == OF_KEY_RETURN){
		loadedAudio.stop();
		audioPlaying = false;
		audioPaused = true;
		initialiseVariables();
	}
	
	if (key == ' '){
		if (!audioPlaying) {
			(*playingAudio).play();
			(*playingAudio).setPaused(false);
			secondAudio.play();
			secondAudio.setPaused(true);
			
			firstAudioFilePlaying = true; 
			
			audioPlaying = true;
			audioPaused = false;
		}
		else{
			audioPaused = !audioPaused;
			(*playingAudio).setPaused(audioPaused);
		}
		
	}
	
	if (key == 'p'){
		swapBetweenPlayingFilesUsingAlignmentMatch();
		
	}
	
	if (key == 'o'){
		openNewAudioFileWithdialogBox();
	}
	
	if (key == 'l'){
		//open audio file
		string *filePtr, secondFileName;
		filePtr = &secondFileName;
		//so filePtr points to secondFileName
		
		if (getFilenameFromDialogBox(filePtr)){
			printf("Loaded name okay :\n'%s' \n", secondFileName.c_str());	
		}
		
		loadSecondAudio(secondFileName);
		
		calculateSimilarityMatrix();
		calculateAlignmentMatrix();
		calculateMinimumAlignmentPath();
		
	}
	
	if (key == 's'){
		drawSimilarity = !drawSimilarity;
	}
	
	
	if (key == 'm'){
		drawSecondMatrix = !drawSecondMatrix;
	}
	
	if (key == 'd'){
		drawSpectralDifferenceFunction = !drawSpectralDifferenceFunction;
	}
	
}

//--------------------------------------------------------------
void TimeWarp::keyReleased  (int key){
	if (key == OF_KEY_LEFT || OF_KEY_RIGHT){
		(*playingAudio).setSpeed(1);
		backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;
	}
	
}
*/
/*
void TimeWarp::openNewAudioFileWithdialogBox(){
	
	//open audio file
	string *filePtr;
	filePtr = &soundFileName;	
	
	if (getFilenameFromDialogBox(filePtr)){
		printf("Mainfile: Loaded name okay :\n'%s' \n", soundFileName.c_str());	
	}
	
	//openFileDialogBox(); - replaced this lone by call to openFile Dialoguebox
	loadNewAudio(soundFileName);
	
}
 */
/*
//--------------------------------------------------------------
void TimeWarp::mouseMoved(int x, int y ){
	width = ofGetWidth();
	pan = (float)x / (float)width;
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	targetFrequency = 2000.0f * heightPct;
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	xIndex = (int)(pan*ENERGY_LENGTH);
}

//--------------------------------------------------------------
void TimeWarp::mouseDragged(int x, int y, int button){
	width = ofGetWidth();
	pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void TimeWarp::mousePressed(int x, int y, int button){
	bNoise = true;
	moveOn = true;
}


//--------------------------------------------------------------
void TimeWarp::mouseReleased(int x, int y, int button){
	bNoise = false;
}

//--------------------------------------------------------------
void TimeWarp::windowResized(int w, int h){
	width = w;
	height = h;
	screenHeight = ofGetHeight() ;
	screenWidth = ofGetWidth();
	
}
*/

//--------------------------------------------------------------
/*

bool TimeWarp::getFilenameFromDialogBox(string* fileNameToSave){
	//this uses a pointer structure within the loader and returns true if the dialogue box was used successfully
	// first, create a string that will hold the URL
	string URL;
	
	// openFile(string& URL) returns 1 if a file was picked
	// returns 0 when something went wrong or the user pressed 'cancel'
	int response = ofxFileDialogOSX::openFile(URL);
	if(response){
		// now you can use the URL 
		*fileNameToSave = URL;
		//printf("\n filename is %s \n", soundFileName.c_str());
		return true;
	}
	else {
		//	soundFileName = "OPEN canceled. ";
		printf("\n open file cancelled \n");
		return false;
	}
	
}
*/


/*
void TimeWarp::putEnergyInFrame(){
	
	
	float totalEnergyInFrame = 0;
	
	for (int i = 0;i<FRAMESIZE;i++){
		
		totalEnergyInFrame += (frame[i] * frame[i]);
		
	}
	totalEnergyInFrame = sqrt(totalEnergyInFrame);
	
	if (energyIndex < ENERGY_LENGTH){
		energy[energyIndex] = totalEnergyInFrame;
		energyIndex++;
	}
	
}
 */



/*
void TimeWarp::printAlignmentMatrix(){
	
	int size = alignmentMeasureMatrix.size();
	printf("\n _ _ _ _\n");
	printf("align size is %i \n", size);
	
	int i,j;
	DoubleVector d;
	int rowSize = alignmentMeasureMatrix.size();
	d = alignmentMeasureMatrix[0];//choose initial size
	
	for (int j = 0;j < d.size();j++){
		printf("row %i : ", j);
		
		for (i = 0;i < rowSize;i++){
			d = alignmentMeasureMatrix[i];
			
			//	printf("row %i , col %i, val : %f \n", i, j, alignmentMeasureMatrix[i][j] );
			printf("%f , ", alignmentMeasureMatrix[i][j] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}


void TimeWarp::printScoreForRow(int row, int max){
	printf("alignment scores row %i \n", row);
	float minimum = alignmentMeasureMatrix[row][0];
	int minimumIndex = 0;
	for (int i =0;i < max;i++){
		printf("[%i] %f ", i, alignmentMeasureMatrix[row][i]);
		if (alignmentMeasureMatrix[row][i] < minimum)
		{
			minimum = alignmentMeasureMatrix[row][i] ;
			minimumIndex = i;
		}
		printf(" \n");
	}
	printf("Minimum [%i] : %f \n", minimumIndex, minimum);
	printf("ALIGN score :[%i] : %f \n", backwardsAlignmentPath[1][backwardsAlignmentIndex], alignmentMeasureMatrix[ backwardsAlignmentPath[0][backwardsAlignmentIndex] ][ (int) backwardsAlignmentPath[1][backwardsAlignmentIndex] ]);
	
	
}
*/




/*
 void TimeWarp::swapBetweenPlayingFilesUsingAlignmentMatch(){
 ofSoundUpdate();
 //swapping between files
 //printf("current playing (energy scale) frame was %i \n", currentPlayingFrame);
 float oldPosition = (*playingAudio).getPosition();
 printf("playing position is %f \n", (*playingAudio).getPosition());
 
 //(*playingAudio).stop(); 
 (*playingAudio).setPaused(true);
 int newIndicator;
 if (firstAudioFilePlaying){
 playingAudio = &secondAudio;
 newIndicator = 1;	
 }
 else{
 playingAudio = &loadedAudio;
 newIndicator = 0;
 }
 printf("new indicator %i \n", newIndicator);
 
 printf("playing pos according to energy frames is %f \n ", 
 (currentPlayingFrame/((float)backwardsAlignmentPath[1-newIndicator][0]*CHROMA_CONVERSION_FACTOR)) );
 
 printf("predicts frame to be %f \n", (oldPosition*backwardsAlignmentPath[1-newIndicator][0]));
 
 currentChromaFrame = oldPosition * (float) backwardsAlignmentPath[1-newIndicator][0];
 printf("current chroma frame %i and using energy frames would have been %i \n", currentChromaFrame, currentPlayingFrame / CHROMA_CONVERSION_FACTOR);
 int matchingFrame = findMatchFromAlignment(firstAudioFilePlaying);	
 
 float relativePosition = matchingFrame / (float) backwardsAlignmentPath[newIndicator][0];
 //i.e. the position as float [0,1] 0:beginning, 1 is end
 
 (*playingAudio).setPaused(false);
 //	secondAudio.setPosition(relativePosition);//XXX tmp line
 (*playingAudio).setPosition(relativePosition);
 
 printf("matching frame is %i and length is %i \n", matchingFrame, backwardsAlignmentPath[newIndicator][0]);
 printf("new playing position is %f \n", (*playingAudio).getPosition());
 
 firstAudioFilePlaying = !firstAudioFilePlaying;
 
 
 }
 */
/*
 int TimeWarp::findMatchFromAlignment(bool whichFileToTest){
 //could use technique from middle of file and go either way to reduce latency for long search? 
 //- (not that this is a problem yet)
 int indicator;
 if (whichFileToTest)
 indicator = 0;
 else
 indicator = 1;
 
 int oppositeIndicator = 1 - indicator;
 
 int frame = backwardsAlignmentPath[indicator].size()-1;
 
 //	int currentChromaFrame = currentPlayingFrame / CHROMA_CONVERSION_FACTOR;
 //trying this instead
 
 
 
 while (backwardsAlignmentPath[indicator][frame] < currentChromaFrame){
 frame--;
 }
 //printf("frame found is %i \n", frame);
 int frameToSwitchTo = backwardsAlignmentPath[oppositeIndicator][frame];
 
 float calculatedPosition = (currentChromaFrame / (float) backwardsAlignmentPath[indicator][0]);
 
 printf("(length was %i)\n",  backwardsAlignmentPath[indicator][0]);
 
 printf("compares to position calculated from chroma length %f \n", calculatedPosition);
 printf("current frame %i maps to new frame %i \n", currentChromaFrame, frameToSwitchTo);
 printf("relative position of new frame is %f \n", (frameToSwitchTo / (float) backwardsAlignmentPath[oppositeIndicator][0]) );
 return frameToSwitchTo; 
 
 }
 
 */



/*
 double TimeWarp::getEnergyOfFrame(){
 
 
 float totalEnergyInFrame = 0;
 
 for (int i = 0;i<FRAMESIZE;i++){
 
 totalEnergyInFrame += (frame[i] * frame[i]);
 
 }
 totalEnergyInFrame = sqrt(totalEnergyInFrame);
 
 return totalEnergyInFrame;
 }
 */

