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
#include <assert.h>

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
	
	useDotProduct = true;////true - dot, falseo: Euclidean dist
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
	//chromaSimilarityMatrix.clear();
	tmpSimilarityMatrix.clear();
	alignmentMeasureMatrix.clear();
	tmpAlignmentMeasureMatrix.clear();	
	minimumAlignmentPath.clear();
	partBackwardsAlignmentPath.clear();
	forwardsAlignmentPath.clear();
	anchorPoints.clear();
}
	
void TimeWarp::initialiseVariables(){
	printf("TW initialise called\n");
	
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


void TimeWarp::calculateCausalChromaSimilarityMatrix(DoubleMatrix& firstChromaMatrix, DoubleMatrix& secondChromaMatrix, DoubleMatrix& simMatrix){
	//calculates the chroma only similarity matrix 
	//but we have already done some, so is extending it...
	
	int size = 0;
	if (simMatrix.size() > 0){
		size = simMatrix[0].size();
	}
	
	if (secondChromaMatrix.size() > size ){
	
	for (int x = 0;x < firstChromaMatrix.size();x++){
		
		if (x < simMatrix.size()){
			//i.e. entries already exist
				for (int y = (int)simMatrix[x].size();y < secondChromaMatrix.size();y++){
					double distance;
					if (useDotProduct)
					distance = getChromaSimilarity(x, y, &firstChromaMatrix, &secondChromaMatrix);
					else
					distance = getEuclideanDistance(x, y, &firstChromaMatrix, &secondChromaMatrix);
				
				//	printf("X %i Y %i dist %f\n", x, y, distance);
					simMatrix[x].push_back(distance);
				}
			}
		else {
			DoubleVector d;
			for (int y = 0;y < secondChromaMatrix.size();y++){
					double distance;
					if (useDotProduct)
					distance = getChromaSimilarity(x, y, &firstChromaMatrix, &secondChromaMatrix);
					else
					distance = getEuclideanDistance(x, y, &firstChromaMatrix, &secondChromaMatrix);
			
				//	printf("new row X %i Y %i dist %f\n", x, y, distance);
					d.push_back( distance);	
				}
				simMatrix.push_back(d);
			}
		}
	}
		//	if (size > 0)
		//		printf("Causial CHROMA ONLY SIM SIZE %i x %i; ", (int)simMatrix.size(), (int) simMatrix[0].size());
		//printf("First matrix SIZE %i , SECOND size %i\n", (int)firstChromaMatrix.size(), (int) secondChromaMatrix.size());	

	
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
	printf("OFFLINE CHROMA ONLY SIM SIZE %i x %i\n", (int)(*simMatrix).size(), (int)(*simMatrix)[0].size());
	printf("First matrix SIZE %i , SECOND size %i\n", (int)(*firstChromaMatrix).size(), (int)(*secondChromaMatrix).size());
	
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
		distance /=  (*firstChromaMatrix)[x].size();
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

bool TimeWarp::extendAlignmentUp(const int& endIndexY, DoubleMatrix *alignmentMatrix){
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


bool TimeWarp::extendAlignmentAlong(const int& endIndexX, DoubleMatrix* alignmentMatrix){
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




void TimeWarp::calculatePartJointSimilarityMatrix(DoubleVector* firstEnergyVector, DoubleVector* secondEnergyVector, DoubleMatrix* chromaSimMatrix, DoubleMatrix* simMatrix, const int& startX, const int& startY, int endX, int endY){
//	printf("PART SIM CALC Calculate similarity : pointers : startX %i startY %i, endX %i endY %i  ", startX, startY, endX, endY);
//	printf("PART SIM ENERGY VEC LENGTHS %i and %i\n", (int) (*firstEnergyVector).size(), (int) (*secondEnergyVector).size());
	
	conversionFactor = (int) round((*firstEnergyVector).size() / (*chromaSimMatrix).size() );
//	printf("conversion fac %i", conversionFactor);
	
	simMatrix->clear();
	
	double energyProportion = 0.2;
	double chromaProportion = 1 - energyProportion;
	double distance, firstSum, secondSum;
	
	endX = min (endX, (int)(*firstEnergyVector).size()-1);//in case out of size
	endY = min( endY+1, (int)(*secondEnergyVector).size());
//	endY = min( endY, (int)(*secondEnergyVector).size()-1);//added to switch 

	int lastChromaYvalue = 0;
	int chromaIndexY = 0;
	double chromaComponent = 0;
	double 	chromaContribution = 0;

	//int  chromaXsize = (int)(*chromaSimMatrix).size()-1;
	//int chromaYsize;
	
	DoubleVector d;	
	
	for (int x = startX;x <= endX;x++){
		d.clear();
		
		//now need the chroma part
		int chromaIndexX = min(x / conversionFactor, (int)(*chromaSimMatrix).size()-1);
		//chromaYsize = (int)(*chromaSimMatrix)[chromaIndexX].size()-1;
		
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
	
	
//	printf("..part JOINT sim size: %i, height: %i \n", (int) (*simMatrix).size(), (int) (*simMatrix)[0].size());
	
}//end self sim





void TimeWarp::calculatePartAlignmentMatrix(int endIndexX, int endIndexY, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix){
//	printf("starting PART Alignment calculation : sim matrix size %i %i endX %i Y %i\n", (int)(*simMatrix).size(), (int)(*simMatrix)[0].size(), endIndexX, endIndexY);
	//initialise alignment
	alignmentMatrix->clear();
	
	DoubleVector d;
	d.push_back(getDistanceFromMatrix(0, 0, simMatrix));
//	printf("first distance %f\n", d[0]);
	(*alignmentMatrix).push_back(d);
	
	bool chromaCalculated = false;
	bool secondCalculated = false;
	
	while (!chromaCalculated || !secondCalculated) {
		
		if (!chromaCalculated)
			chromaCalculated = extendRestrictedAlignmentAlong(endIndexX, alignmentMatrix, simMatrix);
		
		if (!secondCalculated)
			secondCalculated = extendRestrictedAlignmentUp(endIndexY, alignmentMatrix, simMatrix);
		
	}
	
//	printf("PART Alignment matrix calculated, size %i by %i\n", (int) (*alignmentMatrix).size() , (int) (*alignmentMatrix)[0].size());
}






bool TimeWarp::extendRestrictedAlignmentUp(const int& endIndexY, DoubleMatrix *alignmentMatrix, DoubleMatrix* simMatrix){
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


bool TimeWarp::extendRestrictedAlignmentAlong(const int& endIndexX, DoubleMatrix* alignmentMatrix, DoubleMatrix* simMatrix){
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


void TimeWarp::calculateMinimumAlignmentPathRow(DoubleMatrix* alignmentMatrix, IntMatrix* backPath, bool pickMinimumFlag){
	//this requires one pass of the DTW algorithm and then works backwards from (N,M)
	//to find the optimal path to (0,0), where N and M are the lengths of the two chromoVectors respectively
	
	(*backPath).clear();
	
	//	printf("Finding minimum Path %i vs sim size %i\n", (int)chromaMatrix.size(), (int)similarityMatrix.size() );
	
	//printf("Finding minimum ROW Path of alignment matrix %i vs sim size %i\n", (int)(*alignmentMatrix).size(), (int)(*alignmentMatrix)[0].size() );	
	//	printf("compares to sim %ix%i\n", similarityMatrix.size()-1, similarityMatrix[0].size()-1);
	IntVector v;
	//	v.push_back(similarityMatrix.size()-1);//chromaMatrix.size()-1 - old way
	int endIndex = (*alignmentMatrix).size()-1;
	if (pickMinimumFlag){
		endIndex = getMinimumIndexOfRowFromMatrix((int)((*alignmentMatrix)[0].size()-1), *alignmentMatrix);
		printf("minimum of row is %i\n", endIndex);
	}
	v.push_back(endIndex);
	printf("ROW PUSH end index %i ", endIndex);
	(*backPath).push_back(v);
	v.clear();
	v.push_back((*alignmentMatrix)[0].size()-1);
	printf("by %i ", (int)(*alignmentMatrix).size()-1);
	(*backPath).push_back(v);

	printf("ROW: backwards path initialised to %i : %i \n", (*backPath)[0][0], (*backPath)[1][0]);
	
	
	int indexOfBackwardsPath = 0;
	while (!findPreviousMinimumInBackwardsPath(alignmentMatrix, backPath))	{
		
	//	if (indexOfBackwardsPath < backwardsAlignmentPath[0].size()){
	//		printf("backwards path index %i:  path: %i : %i \n", 
	//			   indexOfBackwardsPath, backwardsAlignmentPath[0][indexOfBackwardsPath], backwardsAlignmentPath[1][indexOfBackwardsPath]);
	//	}
		
		indexOfBackwardsPath++;
	}
	
	printf("ROW_final index of backwards path is %i and i is %i \n", (int) (*backPath)[0].size()-1, indexOfBackwardsPath);
	printBackwardsPath(0, (*backPath)[0].size(), backPath);
	
	if (backwardsAlignmentPath.size() > 0)
	backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;//remember that this goes backwards!
	
}



void TimeWarp::calculateMinimumAlignmentPathColumn(DoubleMatrix* alignmentMatrix, IntMatrix* backPath, bool pickMinimumFlag){
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
	printf("COLUMN: backwards path initialised to %i : %i \n", (*backPath)[0][0], (*backPath)[1][0]);
	
	
	int indexOfBackwardsPath = 0;
	while (!findPreviousMinimumInBackwardsPath(alignmentMatrix, backPath))	{
		indexOfBackwardsPath++;
	//	printf("backwards path index %i:  path: %i : %i \n", 
		//dindexOfBackwardsPath, backwardsAlignmentPath[0][indexOfBackwardsPath], backwardsAlignmentPath[1][indexOfBackwardsPath]);
		
	}
//	printf("final index of backwards path is %i and i is %i \n", (int) (*backPath)[0].size()-1, indexOfBackwardsPath);
	
	//	backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;//remember that this goes backwards!
	
}



bool TimeWarp::findPreviousMinimumInBackwardsPath(DoubleMatrix* alignmentMatrix, IntMatrix* backPath){
	int chromaPosition, secondPosition;
	int i,j;
	i = (*backPath)[0][(*backPath)[0].size()-1];
	j  = (*backPath)[1][(*backPath)[1].size()-1];
	///printf("FIND PREVIOUS MINIMUM %i %i \n", i, j);
	
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
	//	printf("(%i,%i) = %f, ", i-1, j, (*alignmentMatrix)[i-1][j]);
	//	if (j > 0)
	//	printf("(%i,%i) = %f, ", i-1, j-1, (*alignmentMatrix)[i-1][j-1]);
	}
	
	//if (j > 0)
	//	printf("(%i,%i) = %f, ", i, j-1, (*alignmentMatrix)[i][j-1]);
	
	if (j > 0 && testForNewAlignmentMinimum(ptr, i, j-1, alignmentMatrix)){
		chromaPosition = i;
		secondPosition = j-1;
		finishedAligning = false;
		
	}
	
	if (!finishedAligning){
		(*backPath)[0].push_back(chromaPosition);
		(*backPath)[1].push_back(secondPosition);
		//printf("PUSHING BACK %i and %i IN BACK PATH MINIMUM %f, i %i, j %i\n", chromaPosition, secondPosition, newMinimum, i, j);
	}else{
		//printf("finished at minimum %f i %i j %i\n", newMinimum, i, j);
	}
	
	return finishedAligning;
	
}	



bool TimeWarp::testForNewAlignmentMinimum(double *previousMinimum, const int& i, const int& j, DoubleMatrix* alignmentMatrix){
	bool newMinimumFound = false;
	
	if ((*alignmentMatrix)[i][j] <= *previousMinimum){
		*previousMinimum = (*alignmentMatrix)[i][j];							   
		newMinimumFound = true;
	}
	
	return newMinimumFound;							   
}															


void TimeWarp::printForwardsPath(){
	if (forwardsAlignmentPath.size() > 0){
		for (int i = 0;i < forwardsAlignmentPath[0].size();i++){
		printf("FORWARDS PATH [%i] : %i , %i\n", i, forwardsAlignmentPath[0][i], forwardsAlignmentPath[1][i]);
		}
	}
}


void TimeWarp::printBackwardsPath(int startIndex, int endIndex, const IntMatrix* backPath){
	if (endIndex <= (*backPath)[0].size()){
		printf("PrintBackwards: %i to %i size of path is %i by %i\n", startIndex, endIndex, (int) (*backPath).size(), (int) (*backPath)[0].size());
	for (int i = startIndex;i < endIndex;i++){
		printf("BackPath[%i]:: %i : %i \n", i, (*backPath)[0][i], (*backPath)[1][i]);
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
	//v.push_back(forwardsAlignmentPath[0][indexX]);//secondMatrix
	v.push_back((*backPath)[1][indexX]);//new 
	forwardsAlignmentPath.push_back(v);
	indexX--;
	printf("FORWARDS PATH STARTED AS %i, %i\n", forwardsAlignmentPath[0][0], forwardsAlignmentPath[1][0]);
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



void TimeWarp::extendForwardAlignmentPathToYanchor(int endY, IntMatrix* backPath, int anchorPointX, int anchorPointY){
	//andchor points are the starting index so if we have already done up to 
	int forwardsIndex = forwardsAlignmentPath.size();
//	int indexX = (*backPath)[0].size() - 1;
	int indexY = (*backPath)[1].size() - 1;
	
	if (forwardsIndex == 0){
		printf("Y_AnchorExtend_initialise forwards path.(%i).\n", indexY);
		IntVector v;
		//printf("aim to start with %i and %i, anchors %i,%i\n", (*backPath)[0][indexX], (*backPath)[1][indexX], anchorPointX, anchorPointY);
		v.push_back((*backPath)[0][indexY]);//chromaMatrix.size()-1
		forwardsAlignmentPath.push_back(v);
		v.clear();
		//v.push_back(forwardsAlignmentPath[0][indexX]);
		v.push_back((*backPath)[1][indexY]);
		forwardsAlignmentPath.push_back(v);
		indexY--;
		//printf("FORWARDS PATH STARTED AS %i, %i\n", forwardsAlignmentPath[0][0], forwardsAlignmentPath[1][0]);
	}
	else{
		//forwards path has been started and we need anchor point
		printf("backpath index %i x %i y %i\n", indexY, (*backPath)[0][indexY], (*backPath)[1][indexY]);
		
	}
	
	printf("about to pop %i vs %i\n", (int) (*backPath)[1][indexY],  endY);
	printBackwardsPath(0, (*backPath)[1].size()-1, backPath);
	printf("back path to pop above!");
	
	while ((*backPath)[1][indexY] <= endY){	
		addNewForwardsPathFromYindex(indexY, backPath, anchorPointX, anchorPointY);
	//	printf("Forwards path from index %i:: path %i : %i\n", indexY, forwardsAlignmentPath[0][forwardsIndex], forwardsAlignmentPath[1][forwardsIndex]);
		indexY--;
		forwardsIndex++;	   
	}
	printf("\n\nEND POPPING %i\n\n", endY);
	
}

void TimeWarp::addNewForwardsPathFromYindex(const int& indexY, IntMatrix* backPath, const int& anchorPointX, const int& anchorPointY){
	//pushes back
	if (indexY < (*backPath)[1].size()){
		forwardsAlignmentPath[0].push_back((int)(*backPath)[0][indexY]+ anchorPointX);
		forwardsAlignmentPath[1].push_back((int)(*backPath)[1][indexY] + anchorPointY);
		printf("popping back backpath FORWARDS %i : %i,%i\n", indexY, (int)(*backPath)[0][indexY]+ anchorPointX, (int)(*backPath)[1][indexY]+ anchorPointY);
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
//	printf("COPY FORWARDS INDEX %i\n", index);
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
	printf("bxckweards path size ids %i\n", (int) backwardsAlignmentPath[0].size());
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

int TimeWarp::getMinimumIndexOfRowFromMatrix(int j, DoubleMatrix& matrix){
	int minimumIndex = 0;
	
	double minimumValue = 0;
	
	int width = matrix.size();
	if (j >= 0 && width > 0 && j < matrix[0].size()){//&& i < matrix.size()
		//int height = (*matrix)[i].size();
		
		int i = 0;
		minimumValue = matrix[i][j];
		while (i < width){
			if (matrix[i][j] < minimumValue){
				minimumIndex = i;
				minimumValue = matrix[i][j];
			}//end if new value
			i++;
		}
	}else{
		printf("ERROR FROM GETTING MINIMIM!!! - zero - out of bounds, received %i and size is %i\n", j, (int)matrix.size());
	}
	
//	printf("minimum row index is %i\n", minimumIndex);
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

