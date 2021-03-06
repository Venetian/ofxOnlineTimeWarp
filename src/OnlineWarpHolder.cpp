/*
 *  OnlineWarpHolder.cpp
 *  ofxOnlineTimeWarp
 *
 *  Created by Andrew on 18/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#include "OnlineWarpHolder.h"

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

/*
 Main functions here:
 
 Load file from a dialogue box with LibSndFile
 We then iterate through all the samples and call our relevant functions in the timeWarp object - i.e. onset, chroma
 and do the calculations for similarity and alignment
 
 
 TO DO:
 Find continual alignment between the playing file and the non-playing file
 - show the current location in the energy amd chroma draw fn for the non-playing file
 
 simplify the loading procedure
 
 */

OnlineWarpHolder::OnlineWarpHolder(){
	playingAudio = &loadedAudio;
	scrollWidth = 1000;
}

void OnlineWarpHolder::setup(){
	sequentialAlignment = true;
	sequentialBlockRatio = 3;
	
	restrictedChromaCalculation = true;
	
	soundFileLoader = new  ofxSoundFileLoader();
	
	alignmentHopsize = 100;
	alignmentFramesize = 600;
	screenToDraw = 0;
	
	doCausalAlignment = true;
	
	ofBackground(255,255,255);
	
	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	
	//	ofSetDataPathRoot("../bin/data/");
	
	//DONT NEED ANY OF THIS
//	sampleRate 			= 44100;
	
	volume				= 0.1f;
	bNoise 				= false;
//	lAudio = new float[256];
//	rAudio = new float[256];
//	ofSoundStreamSetup(2, 0, this, sampleRate,256, 4);
	//UNTIL HERE
	
	
	ofSetFrameRate(60);
	
	
	fvec_t * my_fvec_t;
	aubio_onset_t* my_aubio_result;
	aubio_onsetdetection_t * my_onset_detection;
	
	scrollWidth = 1600;	
	conversionFactor = 1.0;//CHROMA_CONVERSION_FACTOR;
	chromoLength = (float) scrollWidth / conversionFactor;// CHROMA_CONVERSION_FACTOR;
	//	chromaConversionRatio = scrollWidth / (float) conversionFactor;
	//	printf("initial CHROMOLENGTH %f , conv ratio %f\n", chromoLength, chromaConversionRatio);
	
	sfinfo.format = 0;
	
	moveOn = true;
//	informationString = "";
	
	
	chromaG.initialise(FRAMESIZE, CHROMAGRAM_FRAMESIZE);
	onset = new OnsetDetectionFunction(512,1024,6,1);
	
	//set not to play
	audioPlaying = false;
	
	drawSecondMatrix = true;
	drawSpectralDifferenceFunction = false;
	drawSimilarity = true;
	
	screenHeight = ofGetHeight() ;
	screenWidth = ofGetWidth();
	
	//loading audio files
	//loadSoundFiles();	
	//this os load soundfiles
	
	string fullFileName = "/Users/andrew/Documents/work/programming/of_preRelease_v007_osx/apps/myOpenFrameworks007/ofxOnlineTimeWarpPortAudioClass/bin/data/sound/Bach_short1.wav";
	secondFileName = "/Users/andrew/Documents/work/programming/of_preRelease_v007_osx/apps/myOpenFrameworks007/ofxOnlineTimeWarpPortAudioClass/bin/data/sound/Bach_short2.wav";	
	
	tw.initialiseVariables();
	
	loadFirstAudio(fullFileName);
	
	//resetSequentialAnalysis();
	
	loadSecondAudio(secondFileName);//i.e. load same as first file	

	if (!sequentialAlignment)
	calculateSimilarityAndAlignment();

	
	initialiseVariables();
	startingXframe = 0;
	startingYframe = 0;
	
	
}

void OnlineWarpHolder::exit(){
	delete soundFileLoader;
}

void OnlineWarpHolder::calculateSimilarityAndAlignment(){
	
	
	
	//here is the main TimeWarp similarity matrix calc, the minimum alignment matrix via dtw and then the backwards path estimate 
	double timeBefore = ofGetElapsedTimef();
	
	printf("FIRST THE CAUSAL WAY\n");
	printChromaSimilarityMatrix(20);
	
//	for (int i = 0;i < tw.chromaSimilarityMatrix.size();i++){
//		printf ("SiZe of chromasim[%i] is %i\n", i, (int)tw.chromaSimilarityMatrix[i].size());
//	}
	
	printf("CHROMA SIZE HERE IS %i \n", (int) tw.chromaSimilarityMatrix.size());
	printf(" by %i\n",  (int) tw.chromaSimilarityMatrix[0].size());
//	tw.initialiseVariables(); //zaps everything - now called before the fn
	if (1 == 2){
		//redo chroma sim - the offline way
	tw.chromaSimilarityMatrix.clear();
	tw.calculateChromaSimilarityMatrix(&tw.chromaMatrix,  &tw.secondMatrix, &tw.chromaSimilarityMatrix);
	printChromaSimilarityMatrix(20);
	}
	
	double elapsedTime = ofGetElapsedTimef() - timeBefore;
	printf("CHROMA SIMILARITY ONLY TAKES %2.2f seconds\n", elapsedTime);
	
	//dontDoJunkAlignment();i.e. was here

	
	if (doCausalAlignment)
		calculateCausalAlignment();
	else{	
		tw.calculateAlignmentMatrix(tw.chromaMatrix, tw.secondMatrix, &tw.alignmentMeasureMatrix);	
		tw.calculateMinimumAlignmentPathColumn(&tw.alignmentMeasureMatrix, &tw.backwardsAlignmentPath, false);
	}
	
	backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	printf("index size is %i\n", backwardsAlignmentIndex);
	
	
//	setConversionRatio();
	
	printVariousMatrixInfo();
}

void OnlineWarpHolder::doPathBugCheck(){
	//bug check
	//printf("\n\n\nDOING BUG CJECK!\n\n\n");
	
	if (tw.forwardsAlignmentPath.size() > 0 && tw.forwardsAlignmentPath[1][0] != 0){
		tw.forwardsAlignmentPath[1][0] = 0;//sometimes is large rndm number
		for (int i = 0;i < 1000;i++)
		printf("BUG IN FORWARDS PATH FIXED!!!\n");
	}
}


void OnlineWarpHolder::calculateCausalAlignment(){
	calculateSecondForwardsAlignment();
	doPathBugCheck();//fixes problem if first entry not 0
	tw.copyForwardsPathToBackwardsPath();
}


void OnlineWarpHolder::setConversionRatio(){
	if (tw.firstEnergyVector.size() > 0){
	conversionFactor = (int) round(tw.firstEnergyVector.size() / tw.chromaMatrix.size());
	//chromaConversionRatio = (int) round(tw.firstEnergyVector.size() / tw.chromaMatrix.size());
	chromoLength = scrollWidth / (float)conversionFactor;// CHROMA_CONVERSION_FACTOR;
		printf("scrollwidth %i, conversion factor %i, chromo length %i\n", scrollWidth, (int)conversionFactor, (int)chromoLength);
	}
}

void OnlineWarpHolder::printVariousMatrixInfo(){
	
	printf("large SIM SIZE%i, chrom size %ix%i\n", (int)tw.similarityMatrix.size(), (int)tw.chromaMatrix.size(), (int)tw.chromaMatrix[0].size());
	if (tw.similarityMatrix.size() > 0 &&  tw.chromaSimilarityMatrix.size() > 0){
		printf("SIM SIZE %i, and %i \n",(int) tw.similarityMatrix.size(), (int) tw.similarityMatrix[0].size());
		printf("chomra sim size %i, and %i\n", (int) tw.chromaSimilarityMatrix.size(), (int)tw.chromaSimilarityMatrix[0].size());
	}
	
	
	//tw.printBackwardsPath(0, tw.forwardsAlignmentPath[0].size()-1, &tw.forwardsAlignmentPath);
	
	
	printf("backwards path size is [0]:%i, [1]%i, \n", (int)tw.backwardsAlignmentPath[0].size(), (int)tw.backwardsAlignmentPath[1].size());
	
	printf("backwards path size is %i, FORWARDS SIZE IS %i\n", (int)tw.backwardsAlignmentPath[0].size(), (int)tw.forwardsAlignmentPath[0].size());
	//printf("BACKWARDS PATH::\n");
	//tw.printBackwardsPath(0, (int)tw.backwardsAlignmentPath[0].size(), &tw.backwardsAlignmentPath);
}

void OnlineWarpHolder::initialiseVariables(){
	
	chromaIndex = 0;
	//	chromoGramm.initialise(FRAMESIZE,2048);//framesize 512 and hopsize 2048
	audioPosition = 0;
	if (tw.backwardsAlignmentPath.size() > 0)
		backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;//go back to beginning for drawing
	
	numberOfScrollWidthsForFirstFile = 0;
	numberOfScrollWidthsForSecondFile = 0;
	
	(*playingAudio).setPaused(true);	
	audioPlaying = false;
	audioPaused = true;
	

	
}

void OnlineWarpHolder::resetSequentialAnalysis(){
	resetMatrix(&tw.secondMatrix, &tw.secondEnergyVector);
	tw.chromaSimilarityMatrix.clear();
	resetForwardsPath();
}


void OnlineWarpHolder::resetForwardsPath(){
	tw.forwardsAlignmentPath.clear();
	//causal part
	backwardsAlignmentIndex = 0;
	tw.backwardsAlignmentPath.clear();

	tw.anchorPoints.clear();
	anchorStartFrameY = 0;
	anchorStartFrameX = 0;
	tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
	
}


void OnlineWarpHolder::calculateFirstForwardsAlignment(){
	
	//resetForwardsPath() - moved for reset on loading second file - online needs to happen when we start aligning
	//int anchorStartFrameY = 0;
	for (anchorStartFrameX = 0;anchorStartFrameX < tw.firstEnergyVector.size(); anchorStartFrameX += alignmentHopsize){
		
		tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
		printf("\nADD ANCHOR POINTS %i and %i\n", anchorStartFrameX, anchorStartFrameY);
		
		computeAlignmentForFirstBlock(anchorStartFrameX);
		anchorStartFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
		
		
	}//end for startFrameX
	
	//	alternativeCausalForwardsAlignment();
}

void OnlineWarpHolder::calculateSecondForwardsAlignment(){
	
	//resetForwardsPath() - moved for reset on loading second file - online needs to happen when we start aligning
	
	for (;anchorStartFrameY < tw.secondEnergyVector.size(); anchorStartFrameY += alignmentHopsize){
		
		
		printf("\n2nd ADD ANCHOR POINTS %i and %i\n", anchorStartFrameX, anchorStartFrameY);

		tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
		computeAlignmentForSecondBlock(anchorStartFrameY);
		anchorStartFrameX = tw.forwardsAlignmentPath[0][(tw.forwardsAlignmentPath[0].size()-1)];
		//anchorStartFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
		printf("\n2nd AFTER COMPUTATION: ANCHOR POINTS %i and %i\n", anchorStartFrameX, tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)]);
		//tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
		
	}//end for startFrameX
	
//	alternativeCausalForwardsAlignment();
}
/*
void  OnlineWarpHolder::alternativeCausalForwardsAlignment(){
	tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
	
	for (int i =0;i < tw.firstEnergyVector.size();i++){
		//if ()
	}

}

void  OnlineWarpHolder::newAnchorPointReached(){
	tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
	printf("\nADD ANCHOR POINTS %i and %i\n", anchorStartFrameX, anchorStartFrameY);
	
	computeAlignmentForBlock(anchorStartFrameX);
	anchorStartFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
}
*/

void OnlineWarpHolder::computeAlignmentForSecondBlock(const int& startFrameY){
	//NEW FUNCTION - calls only the energy and uses the stored chromagram	
	int startFrameX = 0;
	
	if (tw.anchorPoints.size() > 0){
		startFrameX = tw.anchorPoints[tw.anchorPoints.size()-1][0];
		//printf("2nd BLOCK COMPUTE - starting X is %i (startY %i)\n", startFrameX, startFrameY);
	}
	
	double timeBefore = ofGetElapsedTimef();
	
	tw.calculatePartJointSimilarityMatrix(&tw.firstEnergyVector, &tw.secondEnergyVector, &tw.chromaSimilarityMatrix, &tw.tmpSimilarityMatrix, 
										  startFrameX, startFrameY, startFrameX + sequentialBlockRatio*alignmentFramesize, startFrameY + alignmentFramesize);
	
	//printf("\nTMP SIM MATRIX\n");printAlignmentMatrix(tw.tmpSimilarityMatrix, 80);-printing TMP SIM matrix
	//	printf("TMP size of tmp sim is %i\n", (int)tw.tmpSimilarityMatrix.size());	
	double elapsedTime = ofGetElapsedTimef() - timeBefore;//	printf("PART SIM MATRIX CAL TAKES %f\n", elapsedTime);
//	printf("TMP ALIGN MATRIX restricted only by %i x %i \n", (int)tw.tmpSimilarityMatrix.size()-1, (int) tw.tmpSimilarityMatrix[0].size()-1);
	
	tw.calculatePartAlignmentMatrix(tw.tmpSimilarityMatrix.size()-1, tw.tmpSimilarityMatrix[0].size()-1, &tw.tmpAlignmentMeasureMatrix, &tw.tmpSimilarityMatrix);
	
	//printAlignmentMatrix(tw.tmpAlignmentMeasureMatrix, 20);
	//	printf("\n CALC PART ALIGNMENT MIN PATH\n");
	
	tw.calculateMinimumAlignmentPathRow(&tw.tmpAlignmentMeasureMatrix, &tw.tmpBackwardsPath, true);//true is for greedy calculation
	
//	printf("size of TMP BACK PATH %i\n", (int) tw.tmpBackwardsPath[0].size());
//	printf("\n PART ALIGNMENT GENERATES THIS BACKWARDS PATH:: \n");
//	tw.printForwardsPath(); //MAIN PRINTING OF FORWARDS PATH GENERATED
	
	tw.extendForwardAlignmentPathToYanchor(alignmentHopsize, &tw.tmpBackwardsPath, startFrameX, startFrameY);
	
	
}

void OnlineWarpHolder::computeAlignmentForFirstBlock(const int& startFrameX){
	int startFrameY = 0;
	if (tw.anchorPoints.size() > 0)
		startFrameY = tw.anchorPoints[tw.anchorPoints.size()-1][1];
	
	//NEED TO ASSUME WE DON'T HAVE 
	double timeBefore = ofGetElapsedTimef();
//	printf("PART SIM: startFrameX %i, startFrameY: %i\n", startFrameX, startFrameY);
	//NEW FUNCTION - calls only the energy and uses the stored chromagram	
	
	tw.calculatePartJointSimilarityMatrix(&tw.firstEnergyVector, &tw.secondEnergyVector, &tw.chromaSimilarityMatrix, &tw.tmpSimilarityMatrix, 
										  startFrameX, startFrameY, startFrameX+3*alignmentFramesize, startFrameY + alignmentFramesize);
	
	
	
	
	//	printf("TMP size of tmp sim is %i\n", (int)tw.tmpSimilarityMatrix.size());	
	double elapsedTime = ofGetElapsedTimef() - timeBefore;
	//	printf("PART SIM MATRIX CAL TAKES %f\n", elapsedTime);
	
	//change part sim to have a y limit too
	//check if we can not calculate alignment matrix for chunks of the sim matrix where it is off diagonal
	
	tw.calculatePartAlignmentMatrix(tw.tmpSimilarityMatrix.size()-1, tw.tmpSimilarityMatrix[0].size()-1, &tw.tmpAlignmentMeasureMatrix, &tw.tmpSimilarityMatrix);
	
	//get alignment measure minimum
	//find minimum path between only the section we are interested in
	//write new function to find minimum backwards path from the index we choose (not the final corner)
	//		int myIndex = tw.findMinimumOfMatrixColumn(tw.tmpAlignmentMeasureMatrix, tw.tmpSimilarityMatrix.size()-1);
	/*		printf("my index is %i\n", myIndex); 
	 for (int i = 0;i< tw.alignmentMeasureMatrix[tw.similarityMatrix.size()-1].size() - 1;i++){
	 printf("Alignment[%i] : %f\n", i, tw.alignmentMeasureMatrix[tw.similarityMatrix.size()-1][i]);
	 }
	 */
//	printf("\n CALC PART ALIGNMENT MIN PATH\n");
	tw.calculateMinimumAlignmentPathColumn(&tw.tmpAlignmentMeasureMatrix, &tw.tmpBackwardsPath, true);//true is for greedy calculation
//	printf("\n PART ALIGNMENT GENERATES THIS BACKWARDS PATH:: \n");
	tw.extendForwardAlignmentPath(alignmentHopsize, &tw.tmpBackwardsPath, startFrameX, startFrameY);
	
	//startFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
	
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::update(){

	
	//	chordString = "Chord : ";
	//	chordString += ofToString(rootChord[currentPlayingFrame/conversionFactor]);//CHROMA_CONVERSION_FACTOR]);
	

	
	//if(!audioPause]d)
	//printScoreForRow(audioPosition/CHROMA_CONVERSION_FACTOR, (audioPosition/CHROMA_CONVERSION_FACTOR)+10);
	
	if (!*realTimeAnalysisMode){
		
		audioPosition = (*playingAudio).getPosition();
		if (firstAudioFilePlaying){
			audioPosition *= tw.firstEnergyVector.size();
			updateAlignmentPathIndex(0);
		}
		else {
			audioPosition *= tw.secondEnergyVector.size();	
			updateAlignmentPathIndex(1);
		}
		
		currentPlayingFrame = audioPosition;
		audioPosition = (int) audioPosition % scrollWidth ;
		audioPosition /= scrollWidth;
	
	//	ofSoundUpdate();
		
	}else{
		//audioPosition = anchorStartFrameX;
		currentPlayingFrame = anchorStartFrameX;
	}
	
	updateStartingFrame();
	
	
	
	
}


void OnlineWarpHolder::updateStartingFrame(){
	startingXframe = (tw.firstEnergyVector.size() / scrollWidth);
	startingYframe = (tw.secondEnergyVector.size() / scrollWidth);//secondMatrix
	
	//	printf("DRAW SIM SIZE start frames  %i x %i \n", startingXframe, startingYframe);
	if (!*realTimeAnalysisMode && tw.backwardsAlignmentPath.size() > 0 ){ 
		startingXframe = (tw.backwardsAlignmentPath[0][backwardsAlignmentIndex]/ scrollWidth);
		startingYframe = max(0	, (int)(tw.backwardsAlignmentPath[1][backwardsAlignmentIndex]/ scrollWidth));//*conversionFactor 
		//FIX THE 1 - ASDDED AS DEBUG
		//	printf("alignment index %i, VERSUS DRAW SIM SIZE %i x %i \n", backwardsAlignmentIndex, startingXframe, startingYframe);
	}else{
		startingXframe = anchorStartFrameX / scrollWidth;//current position for us to show recent anchors ON the screen
		startingYframe = anchorStartFrameY / scrollWidth;
	}
	
	//PROBLEM IS THAT THE y value startYframe is not correctly incremented
	
	//tmp 
	//	startingXframe = 0;
	// 	startingYframe = 0;
	
	//	int startingFrame = findStartWidthFrame();
	//	startingFrame = numberOfScrollWidthsForFirstFile * scrollWidth/conversionFactor;
	
	startingXframe *= scrollWidth;// /conversionFactor;
	startingYframe *= scrollWidth;// /conversionFactor;
	//need to fix for second file too
}

void OnlineWarpHolder::updateAlignmentPathIndex(int identifier){
	
	
	if (tw.backwardsAlignmentPath.size() > 0){
		
		if (backwardsAlignmentIndex >= tw.backwardsAlignmentPath[identifier].size())
			backwardsAlignmentIndex = tw.backwardsAlignmentPath[identifier].size()-1;
		
		//this is the alignment where we are currently playing - i.e. switching between files
		
		int chromaPosition = audioPosition;///conversionFactor;//CHROMA_CONVERSION_FACTOR;
		
		while (tw.backwardsAlignmentPath[identifier][backwardsAlignmentIndex] < chromaPosition && backwardsAlignmentIndex > 0)
		{
			backwardsAlignmentIndex--;
		}
	}
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::draw(){
	
	switch (screenToDraw) {
		case 0:
			drawChromaSimilarityMatrix();
			break;
		case 1:
			drawDoubleMatrix(tw.tmpSimilarityMatrix);
			break;
		case 2:
			drawChromoGram();
			break;
		default:
			break;
			
			
	}
//	ofSetHexColor(0xFFFFFF);
//	ofDrawBitmapString(informationString, 20, 20);	
	/*
	if (drawSimilarity){
		//drawSimilarityMatrix();
		drawChromaSimilarityMatrix();//new fn with alignment causal including energy vector
		//but chroma sim used for representation
		
	}
	else{
		drawDoubleMatrix(tw.tmpSimilarityMatrix);
	//	drawDoubleMatrix(tw.chromaSimilarityMatrix);
	}
	//drawChromoGram();
	
	*/
}


void OnlineWarpHolder::drawEnergyVector(const DoubleVector& energyVec){
	
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 1.0;
	int i, j;//, startingFrame;
//	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
//	startingFrame *= scrollWidth;

	for (i = 0; i < scrollWidth - 1; i++){
		j = min(i + startingXframe, (int)energyVec.size()-1);
		ofLine(i*screenWidth/scrollWidth, screenHeight - (energyVec[j]*screenHeight/heightFactor),
			   screenWidth*(i+1)/scrollWidth, screenHeight - (energyVec[j+1]*screenHeight/heightFactor));
		
	}
	
	//informationString = "start frame "+ofToString(startingXframe)+" and here "+ofToString(startingFrame);
}

void OnlineWarpHolder::drawSpectralDifference(const DoubleMatrix& dMatrix){
	if (dMatrix.size()>0){
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();
		float heightFactor = 8;
		double difference;
		int i, j, startingFrame;
		startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= conversionFactor;// CHROMA_CONVERSION_FACTOR; //in terms of chroma frames
		
		
		for (i = 1; i < chromoLength; i++){//changed to add 1
			j = min(i + startingFrame, (int) dMatrix.size()-1 );//in case out of range
			for (int y = 0;y < 12;y++){			
				difference = dMatrix[j][11-y] - dMatrix[j-1][11-y];
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


void OnlineWarpHolder::drawChromoGram(){
	
	DoubleMatrix* dptr;
	DoubleVector* eptr;
	string whichFileString;
	
	if (drawSecondMatrix){
		
		dptr = &tw.secondMatrix;
		eptr = &tw.secondEnergyVector;
		whichFileString = "second file";
		
	}else {
		
		dptr = &tw.chromaMatrix;
		eptr = &tw.firstEnergyVector;
		whichFileString = "first file";
	}
	
	
	
	if (drawSpectralDifferenceFunction)
		drawSpectralDifference(*dptr);
	else{
		//drawDoubleMatrix(*dptr);
		drawScrollingChromagram(*dptr);
	}
	
	ofSetHexColor(0xFF6666);
	drawEnergyVector(*eptr);
	
	ofDrawBitmapString(textString,80,480);
	
	
	ofSetHexColor(0xFFFFFF);
	ofLine(audioPosition*width, 0, audioPosition*width, height);
	
	
	ofDrawBitmapString(chordString,80,580);
	
	ofDrawBitmapString(soundFileName,80,480);
	
	ofDrawBitmapString(whichFileString,20,80);
	
}

void OnlineWarpHolder::drawDoubleMatrix(const DoubleMatrix& dMatrix){
	
	ofBackground(0,0,0);
	if (dMatrix.size()>0){
		int matrixWidth = dMatrix.size();
		int matrixHeight = dMatrix[0].size();
		
		float screenHeight = ofGetHeight() ;
		float screenWidth = ofGetWidth();

		float heightFactor = screenHeight/matrixHeight;
		float widthFactor = screenWidth/matrixWidth;

		for (int xIndex = 0;xIndex < matrixWidth;xIndex++){
			
			for (int yIndex = 0;yIndex < matrixHeight;yIndex++){
				//int xIndex = (int)(x*widthRatio);
				//int yIndex = (int)(y*heightRatio);
				
				int y = (float)yIndex*heightFactor;
				int x = xIndex*widthFactor;
				
			//	if (xIndex >= 0 && xIndex < matrixWidth )
					ofSetColor(0,0,255 * dMatrix[xIndex][yIndex]);//, 0;
			//	else 
			//		ofSetColor(0,0,0);
				
				ofRect(x,y,widthFactor,heightFactor);//widthRatio, heightRatio);//screenWidth/matrixWidth,screenHeight/matrixHeight);
				
			//	ofSetColor(255,255,255);
			//	ofDrawBitmapString(ofToString(yIndex), 20, y);
				
			}//end y
			
		}//end i
		
//	informationString = "Double matrix width "+ofToString(matrixWidth)+"  heioght "+ofToString(matrixHeight);
//		informationString += "\nmax energy "+ofToString(energyMaximumValue);
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
	
}


void OnlineWarpHolder::drawScrollingChromagram(const DoubleMatrix& dMatrix){
	//starts at startingXframe in energy
	int chromaStartXframe = startingXframe / conversionFactor;
	//ends at scrollwidth
	float chromaScrollWidth = scrollWidth / conversionFactor;
	int chromaIndex;
	
	float widthFactor = ofGetWidth()/chromaScrollWidth;
	
	
	for (int i = 0;i < (int)chromaScrollWidth;i++){
		chromaIndex = chromaStartXframe + i;
		if (chromaIndex < dMatrix.size()){
			//then we can draw
			float heightFactor = ofGetHeight() / dMatrix[chromaIndex].size();
			for (int j = 0;j < dMatrix[j].size();j++){
				ofSetColor(0,0,255*dMatrix[chromaIndex][j]);
				ofRect(i * widthFactor, heightFactor*j, widthFactor, heightFactor);
			}
		
		}
	} 
}




void OnlineWarpHolder::drawChromaSimilarityMatrix(){
	//scrollwidth is width in terms of the similarity matrix size (or energy size if we are being accurate)
	//need to get rid of dependency on sim matirx and only use chrom sim matrix
	
	//	int simHeight = (tw.similarityMatrix[0]).size();
	//	int simWidth = tw.similarityMatrix.size();
	
	//	int sizeOfMatrix = (int) tw.similarityMatrix.size();//tw.chromaMatrix.size();
	//	int sizeOfSecondMatrix = (int) tw.similarityMatrix[0].size();
	
	float chromogramScrollWidth = (scrollWidth/conversionFactor);
	//frames needed in energy still
	//in chromagram frames

	//updateStartingFrame(); - done in update routine
	
//	informationString = "start X "+ofToString(startingXframe)+", Y "+ofToString(startingYframe);
	
	int *indexOfAlignmentPathTested;
	int lengthOfPath = 0;
	
	if (tw.backwardsAlignmentPath.size() > 0)
		lengthOfPath = tw.backwardsAlignmentPath[0].size()-1;
	
	indexOfAlignmentPathTested = &lengthOfPath;
	
	int xcoord, ycoord;
	
	ofFill();
	
	float tmpChromoFactor = chromoLength * conversionFactor/ screenHeight;
	
	int resolution = 4; 
	for (int x = 0;x < screenWidth;x+=resolution){
		
		xcoord = (x / screenWidth) * scrollWidth;//i.e.  chromoLength * conversionFactor;
		xcoord += startingXframe;
		int xChromaCoord = xcoord / conversionFactor;
		
		for (int y =0;y < screenHeight;y+=resolution){
			
			ycoord = y * tmpChromoFactor;
			ycoord += startingYframe;
			
			int yChromaCoord = ycoord / conversionFactor; 
			
			int colour = 0;//0x006644;
			
	
			if (xChromaCoord < tw.chromaSimilarityMatrix.size() && yChromaCoord < tw.chromaSimilarityMatrix[0].size()){
				colour = tw.chromaSimilarityMatrix[xChromaCoord][yChromaCoord]*255;
			}
			
			
			ofSetColor(colour,0,0);
			
			ofRect(x,y,resolution,resolution);
			
		}
	}
	
	
	
	//		ofSetColor(0,0,255);
	//		drawAlignmentPath(startingXframe, startingYframe, &tw.backwardsAlignmentPath);
	//ALIGNMENT PATH doesnt work yet - scae rpobllem
	
	
	ofSetColor(0,255,255);
	//	drawAlignmentPath(startingXframe, startingYframe, &tw.tmpBackwardsPath);
	
	drawForwardsAlignmentPathOnChromaSimilarity(startingXframe, startingYframe);
	
	drawAnchorPointsOnChromaSimilarity(startingXframe, startingYframe);
	
	
	//SET TEXT
	ofSetColor(255 ,255,255);

/*	string textString;

	
	textString += "  Xframe : ";
	textString += ofToString(startingXframe);
	
	textString += "  back[0] "; 
	textString += ofToString(tw.backwardsAlignmentPath[0][backwardsAlignmentIndex]);
	
	textString += "  Yframe : ";
	textString += ofToString(startingYframe);
	
	
	textString += "  back[1] "; 
	textString += ofToString(tw.backwardsAlignmentPath[1][backwardsAlignmentIndex]);
	
	
	textString += "  currentFrame : ";
	textString += ofToString(currentPlayingFrame);
	
	
	textString += "  backalign index : ";
	textString += ofToString(backwardsAlignmentIndex);
	
	
	textString += "  scrollwidth: ";
	textString += ofToString(scrollWidth);
	
	textString += "  xcoord: ";
	textString += ofToString(xcoord);
	
	textString += "  Clength: ";
	textString += ofToString(chromoLength);
	
	textString += "  no.Scrolls: ";
	textString += ofToString(numberOfScrollWidthsForFirstFile);
	//END SET TEXT
 */
	
	ofSetHexColor(0x0000FF);// && tw.backwardsAlignmentPath.size() > 0  
	if (firstAudioFilePlaying){// && tw.alignmentMeasureMatrix.size() > 0 
		ofLine(audioPosition*screenWidth, 0, audioPosition*screenWidth, height);
		checkIfAudioPositionExceedsWidthForFirstFile();	
		//	drawAlignmentmeasureValues(currentPlayingFrame);
	}
	else{
		ofLine(0, audioPosition*screenHeight, screenWidth, audioPosition*screenHeight);	
	}
	
//	ofDrawBitmapString(textString,80,580);
	
//	ofDrawBitmapString(userInfoString,80,80);
	
}









void OnlineWarpHolder::drawAlignmentmeasureValues(const int& startingYframe){
	//draw values:
	/*
	 int xcoord = currentPlayingFrame / conversionFactor;
	 ofSetColor(255, 255, 255);
	 for (int y = 0;y < chromoLength; y+=max(1, (int)(20 * chromoLength / screenHeight))){
	 
	 float value = tw.alignmentMeasureMatrix[xcoord][y+startingYframe];
	 int ycoord = y * screenHeight/chromoLength;
	 ofDrawBitmapString(ofToString(value, 2) , audioPosition*screenWidth , ycoord);
	 }
	 */
}

void  OnlineWarpHolder::checkIfAudioPositionExceedsWidthForFirstFile()
{
	if (currentPlayingFrame > scrollWidth*(numberOfScrollWidthsForFirstFile+1))
		numberOfScrollWidthsForFirstFile++;
}

int OnlineWarpHolder::findStartWidthFrame(){
	int startingFrame;
	/*
	 startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	 startingFrame *= scrollWidth;//starting frame in terms of energy frames
	 startingFrame /= conversionFactor;// CHROMA_CONVERSION_FACTOR; 
	 */
	return startingFrame;
}

void OnlineWarpHolder::drawAlignmentPath(int startingChromaXFrame, int startingChromaYFrame, IntMatrix* backPath){
	//	if (tw.alignmentMeasureMatrix.size() > 0){
	if ((*backPath).size() > 0){
		//draw alignment path
		int endingChromaXFrame = startingChromaXFrame + chromoLength;
		int endingChromaYFrame = startingChromaYFrame + chromoLength;
		
		float chromoWidth = screenWidth / chromoLength;
		float chromoHeight = screenHeight / chromoLength;
		
		int index = (*backPath)[0].size()-1;
		//OPTIMISE XXX
		
		
		while ((*backPath)[0][index] < startingChromaXFrame){
			index --;
		}
		
		int printIndex = index;
		int backAlign = (*backPath)[0][index];
		int printxcoord;
		int xcoord;
		
		while ((*backPath)[0][index] < endingChromaXFrame) {
			//	xcoord = min((int)(tw.similarityMatrix.size())-1,(*backPath)[0][index]);
			xcoord = (*backPath)[0][index];
			//int ycoord = min((int)(*backPath)[1][index], (int)(tw.alignmentMeasureMatrix[0].size())-1);
			int ycoord = (int)(*backPath)[1][index];	
			printxcoord = xcoord;
			//	int colour = tw.similarityMatrix[xcoord][ycoord]*255;
			
			//float value = 0;
			//if (xcoord < tw.alignmentMeasureMatrix.size() && ycoord < tw.alignmentMeasureMatrix[xcoord].size())
			//float value = tw.alignmentMeasureMatrix[xcoord][ycoord] ;
			
			
			xcoord -= startingChromaXFrame;
			ycoord -= startingChromaYFrame;
			
			ofRect(xcoord*chromoWidth, ycoord*chromoHeight, chromoWidth, chromoHeight);
			
			index--;
		}
		
		//	drawHoverAlignmentValues();
		//	printf("ALIGN score :[%i] : %f \n", backwardsAlignmentPath[1][backwardsAlignmentIndex],  ureMatrix[ backwardsAlignmentPath[0][backwardsAlignmentIndex] ][ (int) backwardsAlignmentPath[1][backwardsAlignmentIndex] ]);
		
		/*
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
		 
		 */
		
		
		ofSetColor(255,255,255); 
		ofDrawBitmapString(textString,80,640);
	}//end if alignment path > 0
	//else{
	//	printf("al measure mat not in range \n");
	//	}
}







void OnlineWarpHolder::drawForwardsAlignmentPath(int startingChromaXFrame, int startingChromaYFrame){
	
	//using starting frame in energy vector
	
	if (tw.forwardsAlignmentPath.size() > 0){
		int endingChromaXFrame = startingChromaXFrame + chromoLength;
		int endingChromaYFrame = startingChromaYFrame + chromoLength;
		
		float chromoWidth = screenWidth / chromoLength;
		float chromoHeight = screenHeight / chromoLength;
		
		int index = 0;
		//OPTIMISE XXX
		
		while (tw.forwardsAlignmentPath[0][index] < startingChromaXFrame){
			//get to NOW
			index ++;
		}
		
		int xcoord;
		//replacing		tw.similarityMatrix.size() by first e vec below
		
		while (index < tw.forwardsAlignmentPath[0].size() && tw.forwardsAlignmentPath[0][index] < endingChromaXFrame) {
			xcoord = min((int)(tw.firstEnergyVector.size())-1,tw.forwardsAlignmentPath[0][index]);
			int ycoord = (int)tw.forwardsAlignmentPath[1][index];
			
			if (tw.alignmentMeasureMatrix.size() > 0)
				ycoord = min((int)tw.forwardsAlignmentPath[1][index], (int)(tw.alignmentMeasureMatrix[0].size())-1);
			
			//		printxcoord = xcoord;
			int colour = 255;//tw.similarityMatrix[xcoord][ycoord]*255;
			//float value = tw.alignmentMeasureMatrix[xcoord][ycoord] ;
			
			xcoord -= startingChromaXFrame;
			ycoord -= startingChromaYFrame;
			ofSetColor(0,colour,0);
			ofRect(xcoord*chromoWidth, ycoord*chromoHeight, chromoWidth, chromoHeight);
			index++;
		}//end while
		
	}//end if forwards path exista
	
}





void OnlineWarpHolder::drawForwardsAlignmentPathOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame){
	
	//using starting frame in energy vector
	
	if (tw.forwardsAlignmentPath.size() > 0){
		int endingChromaXFrame = startingXFrame + scrollWidth;
		int endingChromaYFrame = startingYFrame + scrollWidth;
		
		float energyVectorWidth = screenWidth / scrollWidth;
		float energyVectorHeight = screenHeight / scrollWidth;
		
		int index = max(0, (int)tw.backwardsAlignmentPath[0].size() - backwardsAlignmentIndex);
		//OPTIMISE XXX
		while (index > 0 && tw.forwardsAlignmentPath[0][index] > startingXFrame){
			//get to NOW
			index --;
		}
		
		while (index < tw.forwardsAlignmentPath[0].size() && tw.forwardsAlignmentPath[0][index] < startingXFrame){
			//get to NOW
			index ++;
		}
		
		int xcoord;
		//replacing tw.similarityMatrix.size() by fiorst e vec
		while (index < tw.forwardsAlignmentPath[0].size() && tw.forwardsAlignmentPath[0][index] < endingChromaXFrame) {
			xcoord = min((int)(tw.firstEnergyVector.size())-1,tw.forwardsAlignmentPath[0][index]);
			int ycoord = (int)tw.forwardsAlignmentPath[1][index];
			
			if (tw.alignmentMeasureMatrix.size() > 0)
				ycoord = min((int)tw.forwardsAlignmentPath[1][index], (int)(tw.alignmentMeasureMatrix[0].size())-1);
			
			//		printxcoord = xcoord;
			int colour = 255;//tw.similarityMatrix[xcoord][ycoord]*255;
			//float value = tw.alignmentMeasureMatrix[xcoord][ycoord] ;
			
			xcoord -= startingXFrame;
			ycoord -= startingYFrame;
			ofSetColor(0,colour,0);
			ofRect(xcoord*energyVectorWidth, ycoord*energyVectorHeight, energyVectorWidth, energyVectorHeight);
			index++;
		}//end while
		
	}//end if forwards path exista
	
}



void OnlineWarpHolder::drawAnchorPointsOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame){
	
	//using starting frame in energy vector
	
	if (tw.anchorPoints.size() > 0){
		int endingChromaXFrame = startingXFrame + scrollWidth;
		int endingChromaYFrame = startingYFrame + scrollWidth;
		
		float energyVectorWidth = screenWidth / scrollWidth;
		float energyVectorHeight = screenHeight / scrollWidth;
		
		int index = 0;
		
		while (index < tw.anchorPoints.size() && tw.anchorPoints[index][0] < startingXFrame){
			index ++;
		}
		
		int xcoord;
		
		while (index < tw.anchorPoints.size() && tw.anchorPoints[index][0] < endingChromaXFrame) {
			xcoord = min((int)(tw.firstEnergyVector.size())-1,tw.anchorPoints[index][0]);
			int ycoord = (int)tw.anchorPoints[index][1];
			
			if (tw.alignmentMeasureMatrix.size() > 0)
				ycoord = min((int)tw.anchorPoints[index][1], (int)(tw.alignmentMeasureMatrix[0].size())-1);
			
			int colour = 255;//tw.similarityMatrix[xcoord][ycoord]*255;
			//float value = tw.alignmentMeasureMatrix[xcoord][ycoord] ;
			
			xcoord -= startingXFrame;
			ycoord -= startingYFrame;
			ofSetColor(colour,colour,colour);
			ofRect((xcoord-1)*energyVectorWidth, (ycoord-1)*energyVectorHeight, 3*energyVectorWidth, 3*energyVectorHeight);
			index++;
		}//end while
		
	}//end if
	
}






void OnlineWarpHolder::loadSoundFiles(){
	
	//assume libsndfile looks in the folder where the app is run
	//therefore ../../../ gets to the bin folder
	//we then need data/sounds/to get to the sound folder
	//this is different to the usual OF default folder
	//was const char	
	const char	*infilename = "../../../data/sound/Bach_short1.wav";	
	loadLibSndFile(infilename);
	
	string loadfilename = "sound/Bach_short1.wav";//PicturesMixer6.aif";	
	loadedAudio.loadSound(loadfilename);
	playingAudio = &loadedAudio;
	
}

void OnlineWarpHolder::loadLibSndFile(const char *infilename){
	
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

void OnlineWarpHolder::processAudioToDoubleMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	//wendy
	
	resetMatrix(myDoubleMatrix, energyVector);
		
	int readcount = 1; // counts number of samples read from sound file
	printf("processing audio from doublematrix \n");
	printf("readcount %i", readcount);
	while(readcount != 0 && moveOn == true && energyVector->size() < FILE_LIMIT)
	{
		
		// read FRAMESIZE samples from 'infile' and save in 'data'
		readcount = sf_read_float(infile, frame, FRAMESIZE);
		
		double doubleFrame[FRAMESIZE];
		for (int k = 0;k< FRAMESIZE;k++){
			doubleFrame[k] = frame[k];
		}
		
		
		
		//8192 samples per chroma frame  //processing frame - downsampled to 11025Hz
		chromaG.processframe(frame);
		
		if (chromaG.chromaready)
		{
			DoubleVector d;
			
			for (int i = 0;i<12;i++){
				d.push_back(chromaG.rawChroma[i]);// / chromaG->maximumChromaValue);	
				
			}	
			//this would do chord detection
			
			myDoubleMatrix->push_back(d);
			//so now is storing at d[i][current_index]
			
		}//end if chromagRamm ready
		
		
		//	double energyValue = getEnergyOfFrame();
		double energyValue = onset->getDFsample(doubleFrame);
		energyVector->push_back(energyValue);
		
		if (energyValue > energyMaximumValue)
			energyMaximumValue = energyValue;
		
		
	}//end while readcount
	
	normaliseChromaMatrix(*myDoubleMatrix);
	normaliseEnergyVector(*energyVector);
	
	//	totalNumberOfFrames = (int)energyVector->size();
	//chromaConversionRatio = myDoubleMatrix->size() / (int)energyVector->size();
	
	//	int size = myDoubleMatrix->size() * CHROMA_CONVERSION_FACTOR;
	
	printf("1st file: maximum chroma value %f and energy %f\n", chromaG.maximumChromaValue, energyMaximumValue);
	
}


void OnlineWarpHolder::normaliseChromaMatrix(DoubleMatrix& myDoubleMatrix){
	printf("NORMALISING Max chroma value is %f \n", chromaG.maximumChromaValue);
	printf("length of chromagram is %d frames\n", (int)myDoubleMatrix.size());
	if (myDoubleMatrix.size() > 0)
	printf("height of dmatrix is %d\n", (int)myDoubleMatrix[0].size());
	//normalise chroma matrix	
	for (int i = 0; i < myDoubleMatrix.size();i++){
		for (int j = 0; j < (myDoubleMatrix[0]).size();j++){
			//non-causal normalisation
			myDoubleMatrix[i][j] /= chromaG.maximumChromaValue;	
		}
	}
}

void OnlineWarpHolder::normaliseEnergyVector(DoubleVector& energyVector){
	
	printf("size of energy vector is %d and maximum energy value is %f\n", (int)energyVector.size(), energyMaximumValue);	
	//non causal normalisation
	for (int i = 0; i < energyVector.size();i++){
		energyVector[i] /= energyMaximumValue;
	}
}

void OnlineWarpHolder::processAudioToMatrixWithCausalAlignment(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	resetMatrix(myDoubleMatrix, energyVector);
	iterateThroughAudioMatrix(myDoubleMatrix, energyVector);
}


void OnlineWarpHolder::resetMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){

	printf("Resetting a matrix, chroma max is %f\n", chromaG.maximumChromaValue);
	myDoubleMatrix->clear();
	energyVector->clear();
	
	chromaG.initialise(FRAMESIZE, CHROMAGRAM_FRAMESIZE);//framesize 512 and hopsize 2048 - already done
	chromaG.maximumChromaValue = 1.0;
	energyMaximumValue = 5;

}

void OnlineWarpHolder::iterateThroughAudioMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	int readcount = 1;
	printf("iterate through second audio\n");
	
	while(readcount != 0 && moveOn == true && energyVector->size() < FILE_LIMIT)
	{
		
		// read FRAMESIZE samples from 'infile' and save in 'data'
		readcount = sf_read_float(infile, frame, FRAMESIZE);
		
		doSequentialAnalysis(frame, myDoubleMatrix, energyVector);
		
	}//end while readcount
	
	
	if (sequentialAlignment){
		printf("END part CAUSAL ALIGNMENT\n");
		updateCausalAlignment();//do end part
		setConversionRatio();
	//backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	//printf("index size is %i\n", backwardsAlignmentIndex);
	
	}
	
	//printChromagramMatrix(tw.secondMatrix.size(), tw.secondMatrix);
	printMatrixData(myDoubleMatrix, energyVector);
	
	//	totalNumberOfFrames = (int)energyVector->size();
	//chromaConversionRatio = myDoubleMatrix->size() / (int)energyVector->size();
	
	printf("2nd file: maximum chroma value %f and energy %f\n", chromaG.maximumChromaValue, energyMaximumValue);
	printChromagramMatrix(tw.secondMatrix.size(), tw.secondMatrix);
}

void OnlineWarpHolder::doSequentialAnalysis(float* frame, DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	if (processFrameToMatrix(frame, myDoubleMatrix, energyVector)){//i.e. new chromagram calculated
		extendChromaSimilarityMatrix(myDoubleMatrix, energyVector);
		if (sequentialAlignment && checkAlignmentWindow()){
			updateCausalAlignment();
		//	printf("updating sequential %i\n", (int)tw.backwardsAlignmentPath[0].size());
		}
	}
	
}

bool OnlineWarpHolder::checkAlignmentWindow(){
//	printf("checking size %i vs alignment pt %i\n", (int) tw.secondEnergyVector.size(), anchorStartFrameY + alignmentFramesize);
	if (tw.secondEnergyVector.size() > anchorStartFrameY + alignmentFramesize)
		return true;
	else
		return false;
}

void OnlineWarpHolder::updateCausalAlignment(){
	//printf("SEQUENTIAL STARTING ANCHORS %i,%i\n", anchorStartFrameX, anchorStartFrameY);
	computeAlignmentForSecondBlock(anchorStartFrameY);
	
	anchorStartFrameX = tw.forwardsAlignmentPath[0][(tw.forwardsAlignmentPath[0].size()-1)];
	anchorStartFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
	//printf("SEQUENTIAL ALIGNMENT ANCHORS %i,%i\n", anchorStartFrameX, anchorStartFrameY);
//	informationString = "Doing sequential alignment, anchors "+ofToString(anchorStartFrameX)+" , "+ofToString(anchorStartFrameY);
	//anchorStartFrameY += alignmentHopsize;
	tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
	tw.copyForwardsPathToBackwardsPath();
}

void OnlineWarpHolder::extendChromaSimilarityMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	if (restrictedChromaCalculation)
	tw.calculateRestrictedCausalChromaSimilarityMatrix(tw.chromaMatrix, tw.secondMatrix, tw.chromaSimilarityMatrix, anchorStartFrameX/conversionFactor, anchorStartFrameY/conversionFactor, (anchorStartFrameX+ sequentialBlockRatio*alignmentFramesize)/conversionFactor, (anchorStartFrameY+alignmentFramesize)/conversionFactor) ;//whichever order as one is extended
	else
	tw.calculateCausalChromaSimilarityMatrix(tw.chromaMatrix, tw.secondMatrix, tw.chromaSimilarityMatrix);//whichever order as one is extended
	
}

bool OnlineWarpHolder::processFrameToMatrix(float newframe[], DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	//printf("processing [%f]\n", newframe[0]);
	bool chromaReady = false;
	
	double doubleFrame[FRAMESIZE];
	for (int k = 0;k< FRAMESIZE;k++){
		doubleFrame[k] = newframe[k];
	}
	
	//8192 samples per chroma frame  //processing frame - downsampled to 11025Hz
	chromaG.processframe(newframe);
	
	if (chromaG.chromaready)
	{
		DoubleVector d;
		
		for (int i = 0;i<12;i++){
			d.push_back(chromaG.rawChroma[i] / chromaG.maximumChromaValue);//causal normalisation of chroma
		//	printf("ITERATION raw value %f and max %f and push back %f\n", chromaG.rawChroma[i], chromaG.maximumChromaValue, d[d.size()-1]);
		}	
		//could do chord detection here too 	
		
		myDoubleMatrix->push_back(d);
		chromaReady = true;
		
		//so now is storing at d[i][current_index]
		
	}//end if chromagRamm ready
	
	
	//	double energyValue = getEnergyOfFrame();
	double energyValue = onset->getDFsample(doubleFrame);
	if (energyValue > energyMaximumValue)
		energyMaximumValue = energyValue;
	
	energyVector->push_back(energyValue/energyMaximumValue);//causal normalisation of energy
	
	//note energy vector is NOT normalised
	

	
	return chromaReady;
	
}

void OnlineWarpHolder::printMatrixData(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	
	printf("Max chroma value is %f \n", chromaG.maximumChromaValue);
	printf("length of chromagram is %d frames\n", (int)myDoubleMatrix->size());
	printf("height of dmatrix is %d\n", (int)(*myDoubleMatrix)[0].size());
	printf("size of energy vector is %d \n", (int)energyVector->size());	

}

//--------------------------------------------------------------
void OnlineWarpHolder::keyPressed  (int key){
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
		
		chromoLength = scrollWidth/conversionFactor;// CHROMA_CONVERSION_FACTOR;
	}
	
	if (key == OF_KEY_UP){
		if (scrollWidth > 600)
			scrollWidth -= 400;
		else
			scrollWidth /= 2;
		
		chromoLength = scrollWidth/conversionFactor;// CHROMA_CONVERSION_FACTOR;
	}
	
	if (key == OF_KEY_LEFT){
		
		(*playingAudio).setSpeed(-4);
		backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	}
	
	if (key == OF_KEY_RIGHT){
		
		(*playingAudio).setSpeed(4);
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
		tw.clearVectors();
		
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
	
		
		
		initialiseVariables();
		backwardsAlignmentIndex = 0;
		tw.chromaSimilarityMatrix.clear();
		tw.initialiseVariables();
		
		loadSecondAudio(secondFileName);
	
		
	}
	
	if (key == '['){
		screenToDraw--;
		screenToDraw = (screenToDraw+NUMBER_OF_SCREENS) % NUMBER_OF_SCREENS;
		printf("screento draw %i\n", screenToDraw);
	}
	
	if (key == ']'){
		screenToDraw++;
		screenToDraw = screenToDraw % NUMBER_OF_SCREENS;
	}
	
	
	if (key == 'f'){
		tw.printBackwardsPath(0, (int) tw.forwardsAlignmentPath[0].size(), &tw.forwardsAlignmentPath);
	}
	
	if (key == 'b'){
		tw.printBackwardsPath(0, (int) tw.backwardsAlignmentPath[0].size(), &tw.backwardsAlignmentPath);
	}
	
	
	if (key == 'h'){
		drawSimilarity = !drawSimilarity;
	}
	
	
	if (key == 'm'){
		drawSecondMatrix = !drawSecondMatrix;
	}
	
	if (key == 's'){
		drawSpectralDifferenceFunction = !drawSpectralDifferenceFunction;
	}
	
	if (key == 'k'){
		printChromagramMatrix(tw.secondMatrix.size(), tw.secondMatrix);
	}
	
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::keyReleased  (int key){
	if (key == OF_KEY_LEFT || OF_KEY_RIGHT){
		(*playingAudio).setSpeed(1);
		if (tw.backwardsAlignmentPath.size() > 0)
			backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	}
	
}

void OnlineWarpHolder::openNewAudioFileWithdialogBox(){
	//loads first audio
	
	//open audio file
	string *filePtr;
	filePtr = &soundFileName;	
	
	if (getFilenameFromDialogBox(filePtr)){
		printf("Mainfile: Loaded name okay :\n'%s' \n", soundFileName.c_str());	
	}
	
	//openFileDialogBox(); - replaced this lone by call to openFile Dialoguebox
	loadFirstAudio(soundFileName);
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::mouseMoved(int x, int y ){
	width = ofGetWidth();
	
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::mouseDragged(int x, int y, int button){
	
	
}

//--------------------------------------------------------------
void OnlineWarpHolder::mousePressed(int x, int y, int button){
//	bNoise = true;
	
	printf("mouse clicked %i,%i\n", x, y);
	printf("X = %i, ", (int)(startingXframe+(float)(x*scrollWidth)/screenWidth));
	printf("Y = %i\n", (int)(startingYframe+(float)(y*scrollWidth)/screenHeight));
	//moveOn = true;
	anchorStartFrameX = (int)(startingXframe+(float)(x*scrollWidth)/screenWidth);
	anchorStartFrameY = (int)(startingYframe+(float)(y*scrollWidth)/screenHeight);
	tw.addAnchorPoints(anchorStartFrameX, anchorStartFrameY);
}


//--------------------------------------------------------------
void OnlineWarpHolder::mouseReleased(int x, int y, int button){
//	bNoise = false;
}

//--------------------------------------------------------------
void OnlineWarpHolder::windowResized(int w, int h){
	width = w;
	height = h;
	screenHeight = ofGetHeight() ;
	screenWidth = ofGetWidth();
	
}



//--------------------------------------------------------------


bool OnlineWarpHolder::getFilenameFromDialogBox(string* fileNameToSave){
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

void OnlineWarpHolder::openFileDialogBox(){
	
	// first, create a string that will hold the URL
	string URL;
	
	// openFile(string& URL) returns 1 if a file was picked
	// returns 0 when something went wrong or the user pressed 'cancel'
	int response = ofxFileDialogOSX::openFile(URL);
	if(response){
		// now you can use the URL 
		soundFileName = URL;//"URL to open: \n "+URL;
	}else {
		soundFileName = "OPEN cancelled. ";
	}
	
	
	
}


void OnlineWarpHolder::loadFirstAudio(string soundFileName){

	tw.clearVectors();//clear ALL info in timeWarp - chromaSim and alignment etc

	loadedAudio.loadSound(soundFileName);
	playingAudio = &loadedAudio;	//load sound for playback


	const char	*infilename = soundFileName.c_str() ;
	loadLibSndFile(infilename);
	//	loadFirstAudioFile();
	
	printf("Load FIRST file %s\n", soundFileName.c_str());

//	informationString = "Loading first file.."+ soundFileName;
	
	tw.initialiseVariables();
	tw.clearVectors();
	
	processAudioToDoubleMatrix(&tw.chromaMatrix, &tw.firstEnergyVector);//non causal way for first audio (ref) file
	
	soundFileLoader->loadLibSndFile(infilename);//LOADS IT INTO SOUNDFILE.AUDIOHOLDER.AUDIOVECTOR
	
	audioPlaying = false;
	
	setConversionRatio();
	
}



void OnlineWarpHolder::loadSecondAudio(string sndFileName){
	
	secondAudio.loadSound(sndFileName);
	
	const char	*infilenme = sndFileName.c_str() ;	
	loadLibSndFile(infilenme);
	printf("Load SECOND file\n");
//	informationString = "Loading second file.."+ soundFileName;
	
	resetSequentialAnalysis();
	
	//the 'live' file to be analysed
	processAudioToMatrixWithCausalAlignment(&tw.secondMatrix, &tw.secondEnergyVector);
		
	
	
	printChromagramMatrix(20, tw.secondMatrix);
	
	//	processAudioToDoubleMatrix(&tw.secondMatrix, &tw.secondEnergyVector); - I guess non causal way
	//	calculateSimilarityAndAlignment(); - now done in process to matrix
}



void OnlineWarpHolder::swapBetweenPlayingFilesUsingAlignmentMatch(){
	float tmpConvFac = conversionFactor;
	conversionFactor = 1.0;
	ofSoundUpdate();
	//swapping between files
	//printf("current playing (energy scale) frame was %i \n", currentPlayingFrame); 
	float oldPosition = (*playingAudio).getPosition();
	printf("\n playing position is %f \n", oldPosition);//and conv factor %f \n", (*playingAudio).getPosition(), conversionFactor);
	float currentPlayingFileLength;
	float newFileLength;
	//(*playingAudio).stop(); 
	(*playingAudio).setPaused(true);
	int newIndicator;
	if (firstAudioFilePlaying){
		playingAudio = &secondAudio;
		newIndicator = 1;	
		currentPlayingFileLength = tw.firstEnergyVector.size();
		newFileLength = tw.secondEnergyVector.size();
	}
	else{
		playingAudio = &loadedAudio;
		newIndicator = 0;
		currentPlayingFileLength = tw.secondEnergyVector.size();
		newFileLength =  tw.firstEnergyVector.size();
	}
	
	//printf("new indicator %i \n", newIndicator);
	printf("playing pos according to energy frames is %f; ", 
		   (currentPlayingFrame/((float)tw.backwardsAlignmentPath[1-newIndicator][0])) );//*  conversionFactor)) );//CHROMA_CONVERSION_FACTOR
	printf("Current frame %f, predicts new frame to be roughly %f \n", 
		   (oldPosition*tw.backwardsAlignmentPath[newIndicator][0]), (oldPosition*tw.backwardsAlignmentPath[1-newIndicator][0]));
	printf("file lenbgths are now %i and other %i\n", tw.backwardsAlignmentPath[newIndicator][0], tw.backwardsAlignmentPath[1-newIndicator][0]);
	printf("compared to energy vec lengths %i and %i\n", tw.firstEnergyVector.size(), tw.secondEnergyVector.size());
	
	currentChromaFrame = round(oldPosition * currentPlayingFileLength);
	//	currentChromaFrame = currentPlayingFrame;// / conversionFactor;
	printf("current chroma frame %i \n", currentChromaFrame);//and using energy frames would have been %i \n", currentChromaFrame, currentPlayingFrame / conversionFactor);//CHROMA_CONVERSION_FACTOR);
	
	int matchingFrame = findMatchFromAlignment(firstAudioFilePlaying);		
	float relativePosition = matchingFrame / newFileLength;// tw.backwardsAlignmentPath[newIndicator][0];
	//i.e. the position as float [0,1] 0:beginning, 1 is end
	
	(*playingAudio).setPaused(false);
	(*playingAudio).setPosition(relativePosition);
	
	printf("matching frame is %i \n", matchingFrame, tw.backwardsAlignmentPath[newIndicator][0]);
	printf("new playing position is %f and back align index %i \n", (*playingAudio).getPosition(), backwardsAlignmentIndex);
	
	firstAudioFilePlaying = !firstAudioFilePlaying;
	
	conversionFactor = tmpConvFac;
}

int OnlineWarpHolder::findMatchFromAlignment(bool whichFileToTest){
	//could use technique from middle of file and go either way to reduce latency for long search? 
	//- (not that this is a problem yet)
	int indicator;
	if (whichFileToTest)
		indicator = 0;
	else
		indicator = 1;
	
	int oppositeIndicator = 1 - indicator;
	
	int frame = backwardsAlignmentIndex;//	tw.backwardsAlignmentPath[indicator].size()-1;
	
	while (tw.backwardsAlignmentPath[indicator][frame] > currentChromaFrame && backwardsAlignmentIndex < tw.backwardsAlignmentPath[indicator].size()-1){
		frame++;
	}
	
	while (tw.backwardsAlignmentPath[indicator][frame] < currentChromaFrame && frame > 0){
		frame--;
	}
	//printf("frame found is %i \n", frame);
	int frameToSwitchTo = tw.backwardsAlignmentPath[oppositeIndicator][frame];
	
	float calculatedPosition = (currentChromaFrame / (float) tw.backwardsAlignmentPath[indicator][0]);
	
	printf("(length was %i)\n",  tw.backwardsAlignmentPath[indicator][0]);
	
	printf("compares to position calculated from chroma length %f \n", calculatedPosition);
	printf("current frame %i maps to new frame %i \n", currentChromaFrame, frameToSwitchTo);
	printf("relative position of new frame is %f \n", (frameToSwitchTo / (float) tw.backwardsAlignmentPath[oppositeIndicator][0]) );
	return frameToSwitchTo; 
	
}


void OnlineWarpHolder::printChromagramMatrix(int sizeToPrint, DoubleMatrix& matrix){
	
	printf("\n _ _ _ _\n");
	printf("ChromaGram Matrix \n");
	int i,j;
	double tmpMax = 0;
	DoubleVector d;
	int rowSize = min(sizeToPrint, (int) matrix.size());
	
	for (int j = 0;j < rowSize;j++){
		printf("row %i : ", j);
		
		for (i = 0;i < 12;i++){			
			printf("%f , ", matrix[j][i] );
			if (matrix[j][i] > tmpMax)
				tmpMax = matrix[j][i];
		}
		printf("\n");
	}
	printf("...............\n");
	printf("Max is %f\n", tmpMax);
	
}

void OnlineWarpHolder::printSimilarityMatrix(int sizeToPrint){
	
	printf("\n _ _ _ _\n");
	printf("Similarity Matrix \n");
	int i,j;
	DoubleVector d;
	int rowSize = sizeToPrint;
	
	for (int j = 0;j < rowSize;j++){
		printf("row %i : ", j);
		
		for (i = 0;i < rowSize;i++){			
			printf("%2.5f , ", tw.similarityMatrix[i][j] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}

void OnlineWarpHolder::printChromaSimilarityMatrix(int sizeToPrint){
	
	printf("\n _ _ _ _\n");
	printf("Similarity Matrix \n");
	int i,j;
	DoubleVector d;
	int rowSize = min(sizeToPrint, (int) tw.chromaSimilarityMatrix.size());
	
	for (int j = 0;j < rowSize;j++){
		printf("row %i : ", j);
		
		for (i = 0;i < min(sizeToPrint, (int) tw.chromaSimilarityMatrix[j].size());i++){			
			printf("%f , ", tw.chromaSimilarityMatrix[j][i] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}



void OnlineWarpHolder::printAlignmentMatrix(const DoubleMatrix& alignmentMatrix){
	
	int size = alignmentMatrix.size();
	printf("\n _ _ _ _\n");
	printf("align size is %i \n", size);
	
	int i,j;
	DoubleVector d;
	int rowSize = alignmentMatrix.size();
	d = alignmentMatrix[0];//choose initial size
	
	for (int j = 0;j < d.size();j++){
		printf("row %i : ", j);
		
		for (i = 0;i < rowSize;i++){
			d = alignmentMatrix[i];
			
			//	printf("row %i , col %i, val : %f \n", i, j, alignmentMeasureMatrix[i][j] );
			printf("%f , ", alignmentMatrix[i][j] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}


void OnlineWarpHolder::printAlignmentMatrix(const DoubleMatrix& alignmentMatrix, int sizeToPrint){
	
	int size = alignmentMatrix.size();
	printf("\n _ _ _ _\n");
	printf("align size is %i \n", size);
	
	int i,j;
	DoubleVector d;
	int rowSize = min(sizeToPrint, (int)alignmentMatrix.size());
	d = alignmentMatrix[0];//choose initial size
	
	for (int j = 0;j < min(rowSize, (int)d.size());j++){
		printf("row %i : ", j);
		
		for (i = 0;i < rowSize;i++){
			d = alignmentMatrix[i];
			
			//	printf("row %i , col %i, val : %f \n", i, j, alignmentMeasureMatrix[i][j] );
			printf("%f , ", alignmentMatrix[i][j] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}



void OnlineWarpHolder::printScoreForRow(int row, int max){
	printf("alignment scores row %i \n", row);
	float minimum = tw.alignmentMeasureMatrix[row][0];
	int minimumIndex = 0;
	for (int i =0;i < max;i++){
		printf("[%i] %f ", i, tw.alignmentMeasureMatrix[row][i]);
		if (tw.alignmentMeasureMatrix[row][i] < minimum)
		{
			minimum = tw.alignmentMeasureMatrix[row][i] ;
			minimumIndex = i;
		}
		printf(" \n");
	}
	printf("Minimum [%i] : %f \n", minimumIndex, minimum);
	printf("ALIGN score :[%i] : %f \n", tw.backwardsAlignmentPath[1][backwardsAlignmentIndex], tw.alignmentMeasureMatrix[tw.backwardsAlignmentPath[0][backwardsAlignmentIndex] ][ (int) tw.backwardsAlignmentPath[1][backwardsAlignmentIndex] ]);
	
}






void OnlineWarpHolder::dontDoJunkAlignment(){
	
	if (1 == 2){
		//dont do joint as matrix is too large - energyvec x energyvec
		//needs doing in parts
		double timeBefore = ofGetElapsedTimef();
		tw.calculateJointSimilarityMatrix(&tw.firstEnergyVector,  &tw.secondEnergyVector, &tw.chromaSimilarityMatrix, &tw.similarityMatrix);
		double elapsedTime = ofGetElapsedTimef() - timeBefore;
		printf("JOINT SIMILARITY TAKES %6.2f seconds\n", elapsedTime);
		//this has been replaced by the joint function
	}
	
	
	
	//	the big similarity measure
	//timeBefore = ofGetElapsedTimef();
	//tw.calculateSimilarityMatrixWithPointers(&tw.firstChromaEnergyMatrix, &tw.secondChromaEnergyMatrix, &tw.similarityMatrix);
	//elapsedTime = ofGetElapsedTimef() - timeBefore;
	//printf("CREATING BIG SIMILARITY MATRIX TAKES %f\n", elapsedTime);
	//bug is here above - needed to make sim matrix visible
	
	//new fn
	//	tw.calculateSimilarityMatrixFromChromagramAndEnergy(&tw.firstEnergyVector, &tw.chromaMatrix, &tw.secondEnergyVector,  tw.secondMatrix, &tw.similarityMatrix);		
	//	tw.calculateAlignmentMatrix(tw.firstChromaEnergyMatrix, tw.secondChromaEnergyMatrix, &tw.alignmentMeasureMatrix);								
	//	tw.calculateMinimumAlignmentPath(&tw.alignmentMeasureMatrix, &tw.backwardsAlignmentPath, false);
	
}

/*
 const char	*infile_name = fullFileName.c_str();// "../../../data/sound/Bach_short1.wav";	//
 
 loadLibSndFile(infile_name);
 
 string loadfilename = fullFileName;//"sound/Bach_short1.wav";//
 loadedAudio.loadSound(loadfilename);
 playingAudio = &loadedAudio;
 //end load soninf ifiels
 
 processAudioToDoubleMatrix(&tw.chromaMatrix, &tw.firstEnergyVector);
 
 */
/*
 
 void OnlineWarpHolder::drawSimilarityMatrix(){
 
 int simHeight = (tw.similarityMatrix[0]).size();
 int simWidth = tw.similarityMatrix.size();
 
 int sizeOfMatrix = (int) tw.similarityMatrix.size();//tw.chromaMatrix.size();
 int sizeOfSecondMatrix = (int) tw.similarityMatrix[0].size();
 
 //in chromagram frames
 startingXframe = tw.chromaSimilarityMatrix.size() / (scrollWidth/conversionFactor);
 startingYframe = tw.chromaSimilarityMatrix[0].size() / (scrollWidth/conversionFactor);
 
 
 if (tw.backwardsAlignmentPath.size() > 0){
 startingXframe = tw.backwardsAlignmentPath[0][backwardsAlignmentIndex] / (scrollWidth/conversionFactor);
 startingYframe = tw.backwardsAlignmentPath[1][backwardsAlignmentIndex] / (scrollWidth/conversionFactor);
 }
 
 startingXframe = startingXframe * scrollWidth/conversionFactor;
 startingYframe = startingYframe * scrollWidth/conversionFactor;
 
 //	int startingFrame = findStartWidthFrame();
 //	startingFrame = numberOfScrollWidthsForFirstFile * scrollWidth/conversionFactor;
 
 
 //need to fix for second file too
 
 int *indexOfAlignmentPathTested;
 int lengthOfPath = 0;
 if (tw.backwardsAlignmentPath.size() > 0)
 lengthOfPath = tw.backwardsAlignmentPath[0].size()-1;
 
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
 colour = tw.similarityMatrix[xcoord][ycoord]*255;
 
 
 
 ofSetColor(colour,0,0);
 
 ofRect(x,y,1,1);
 
 }
 }
 
 
 
 ofSetColor(0,255,255);
 //	drawAlignmentPath(startingXframe, startingYframe, &tw.tmpBackwardsPath);
 drawForwardsAlignmentPath(startingXframe, startingYframe);
 
 
 //	ofSetColor(0,0,255);
 //	drawAlignmentPath(startingXframe, startingYframe, &tw.backwardsAlignmentPath);
 
 
 //SET TEXT
 ofSetColor(255 ,255,255);
 string textString;
 textString = "width : ";
 textString += ofToString(simWidth);
 
 textString += "  height : ";
 textString += ofToString(simHeight);
 
 //	textString += "  startframe : ";
 //	textString += ofToString(startingFrame);
 
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
 
 ofSetColor(0x0000FF);// && tw.backwardsAlignmentPath.size() > 0  
 if (firstAudioFilePlaying){// && tw.alignmentMeasureMatrix.size() > 0 
 ofLine(audioPosition*screenWidth, 0, audioPosition*screenWidth, height);
 checkIfAudioPositionExceedsWidthForFirstFile();	
 //	drawAlignmentmeasureValues(currentPlayingFrame);
 }
 else{
 ofLine(0, audioPosition*screenHeight, screenWidth, audioPosition*screenHeight);	
 }
 
 ofDrawBitmapString(textString,80,580);
 
 ofDrawBitmapString(userInfoString,80,80);
 
 }
 
 
 */


