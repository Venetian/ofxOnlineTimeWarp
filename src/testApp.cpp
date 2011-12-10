#include "testApp.h"
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


void testApp::setup(){

	doCausalAlignment = true;
	
	ofBackground(255,255,255);

	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	

//	ofSetDataPathRoot("../bin/data/");

//DONT NEED ANY OF THIS
	sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	lAudio = new float[256];
	rAudio = new float[256];
	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);
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
	
	
	chromaG.initialise(FRAMESIZE, CHROMAGRAM_FRAMESIZE);
	onset = new OnsetDetectionFunction(512,1024,6,1);
	
	//loading audio files
	//loadSoundFiles();	
	//this os load soundfiles
	
	string fullFileName = "/Users/andrew/Documents/work/programming/of_preRelease_v007_osx/apps/myOpenFrameworks007/ofxOnlineTimeWarp/bin/data/sound/Bach_short1.wav";
	secondFileName = "/Users/andrew/Documents/work/programming/of_preRelease_v007_osx/apps/myOpenFrameworks007/ofxOnlineTimeWarp/bin/data/sound/Bach_short2.wav";	

	
	loadNewAudio(fullFileName);
	/*
	const char	*infile_name = fullFileName.c_str();// "../../../data/sound/Bach_short1.wav";	//
	
	 loadLibSndFile(infile_name);
	 
	string loadfilename = fullFileName;//"sound/Bach_short1.wav";//
	 loadedAudio.loadSound(loadfilename);
	playingAudio = &loadedAudio;
	 //end load soninf ifiels

	processAudioToDoubleMatrix(&tw.chromaMatrix, &tw.firstEnergyVector);
	
	*/
	
//	secondFileName = "../../../data/sound/Bach_short2.wav"; 
	 


	loadSecondAudio(secondFileName);//i.e. load same as first file
	
	backwardsAlignmentIndex = 0;//remember that this goes backwards!

	tw.initialiseVariables();
	
	calculateSimilarityAndAlignment();
	
	//set not to play
	audioPlaying = false;

	drawSecondMatrix = false;
	drawSpectralDifferenceFunction = false;
	drawSimilarity = true;

	screenHeight = ofGetHeight() ;
	screenWidth = ofGetWidth();
	

	initialiseVariables();
	
	
}
 


void testApp::calculateSimilarityAndAlignment(){
	
	tw.initialiseVariables();
	
	//here is the main TimeWarp similarity matrix calc, the minimum alignment matrix via dtw and then the backwards path estimate 
	double timeBefore = ofGetElapsedTimef();
	tw.calculateChromaSimilarityMatrix(&tw.chromaMatrix,  &tw.secondMatrix, &tw.chromaSimilarityMatrix);
	
	double elapsedTime = ofGetElapsedTimef() - timeBefore;
	printf("CHROMA SIMILARITY ONLY TAKES %2.2f seconds\n", elapsedTime);
	
	dontDoJunkAlignment();

	if (doCausalAlignment)
		calculateCausalAlignment();
	else{
			//tw.calculateAlignmentMatrix(tw.firstChromaEnergyMatrix, tw.secondChromaEnergyMatrix, &tw.alignmentMeasureMatrix);	
			tw.calculateAlignmentMatrix(tw.chromaMatrix, tw.secondMatrix, &tw.alignmentMeasureMatrix);	
			tw.calculateMinimumAlignmentPath(&tw.alignmentMeasureMatrix, &tw.backwardsAlignmentPath, false);
	}
	
	backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	printf("index size is %i\n", backwardsAlignmentIndex);

	


	setConversionRatio();
	
	printVariousMatrixInfo();
}

void testApp::doPathBugCheck(){
//bug check
if (tw.forwardsAlignmentPath.size() > 0 && tw.forwardsAlignmentPath[1][0] != 0){
	tw.forwardsAlignmentPath[1][0] = 0;//sometimes is large rndm number
	printf("BUG IN FORWARDS PATH FIXED!!!\n");
}
}


void testApp::calculateCausalAlignment(){
	calculateForwardsAlignment();
	doPathBugCheck();
	tw.copyForwardsPathToBackwardsPath();
}


void testApp::setConversionRatio(){
	conversionFactor = (int) round(tw.firstEnergyVector.size() / tw.chromaMatrix.size());
	chromaConversionRatio = (int) round(tw.firstEnergyVector.size() / tw.chromaMatrix.size());
	chromoLength = scrollWidth / (float)conversionFactor;// CHROMA_CONVERSION_FACTOR;
	printf("scrollwidth %i, conversion factor %i, chromo length %i\n", scrollWidth, (int)conversionFactor, (int)chromoLength);
}

void testApp::printVariousMatrixInfo(){
	
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

void testApp::initialiseVariables(){
	
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
	
//	tw.initialiseVariables();
	
}


void testApp::calculateForwardsAlignment(){
	
	tw.forwardsAlignmentPath.clear();
	//causal part
	int hopsize = 200;
	int frameSize = 800;
	int startFrameY = 0;
	int startFrameX = 0;
	tw.anchorPoints.clear();
	for (int startFrameX = 0;startFrameX < tw.firstEnergyVector.size(); startFrameX += hopsize){//tw.firstChromaEnergyMatrix.size()
		//replace this with a while startFrame < end of file
		tw.addAnchorPoints(startFrameX, startFrameY);
		
		//NEED TO ASSUME WE DON'T HAVE 
		double timeBefore = ofGetElapsedTimef();
		printf("PART SIM: startFrameX %i, startFrameY: %i\n", startFrameX, startFrameY);
			//NEW FUNCTION - calls only the energy and uses the stored chromagram	
		
		tw.calculatePartJointSimilarityMatrix(&tw.firstEnergyVector, &tw.secondEnergyVector, &tw.chromaSimilarityMatrix, &tw.tmpSimilarityMatrix, 
											  startFrameX, startFrameY, startFrameX+frameSize, startFrameY + 3*frameSize);
		
		
		
		
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
		printf("\n CALC PART ALIGNMENT MIN PATH\n");
		tw.calculateMinimumAlignmentPath(&tw.tmpAlignmentMeasureMatrix, &tw.tmpBackwardsPath, true);//true is for greedy calculation
		printf("\n PART ALIGNMENT GENERATES THIS BACKWARDS PATH:: \n");
		tw.extendForwardAlignmentPath(hopsize, &tw.tmpBackwardsPath, startFrameX, startFrameY);
		
		startFrameY = tw.forwardsAlignmentPath[1][(tw.forwardsAlignmentPath[0].size()-1)];
		
	}//end for startFrameX
	
}

//--------------------------------------------------------------
void testApp::update(){
	textString = "energy index [";
	textString += ofToString(xIndex);
	textString += "] = ";
	textString += ofToString(energy[xIndex]);
	
//	chordString = "Chord : ";
//	chordString += ofToString(rootChord[currentPlayingFrame/conversionFactor]);//CHROMA_CONVERSION_FACTOR]);
	
	audioPosition = (*playingAudio).getPosition();
	if (firstAudioFilePlaying){
		audioPosition *= tw.firstEnergyVector.size();
		updateAlignmentPathIndex(0);
	}
	else {
		audioPosition *= tw.secondEnergyVector.size();	
		updateAlignmentPathIndex(1);
	}
	
	//if(!audioPaused)
	//printScoreForRow(audioPosition/CHROMA_CONVERSION_FACTOR, (audioPosition/CHROMA_CONVERSION_FACTOR)+10);

	
	currentPlayingFrame = audioPosition;
	audioPosition = (int) audioPosition % scrollWidth ;
	audioPosition /= scrollWidth;
	
	ofSoundUpdate();
	
	
}

void testApp::updateAlignmentPathIndex(int identifier){

	
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
void testApp::draw(){

	if (drawSimilarity){
		//drawSimilarityMatrix();
		drawChromaSimilarityMatrix();//new fn with alignment causal including energy vector
		//but chroma sim used for representation
		
	}
	else
		drawDoubleMatrix(&tw.tmpSimilarityMatrix);
	//drawChromoGram();

		
}


void testApp::drawEnergyVectorFromPointer(DoubleVector* energyVec){
	
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();  
	float heightFactor = 1;
	int i, j, startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;
	
	for (i = 0; i < scrollWidth - 1; i++){
		j = min(i + startingFrame, (int)energyVec->size()-1);
		ofLine(i*screenWidth/scrollWidth, screenHeight - ((*energyVec)[j]*screenHeight/heightFactor),
			   screenWidth*(i+1)/scrollWidth, screenHeight - ((*energyVec)[j+1]*screenHeight/heightFactor));
		
	}
}

void testApp::drawSpectralDifference(DoubleMatrix* dMatrix){
	if ((*dMatrix).size()>0){
		
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();
	float heightFactor = 8;
	double difference;
	int i, j, startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= conversionFactor;// CHROMA_CONVERSION_FACTOR; //in terms of chroma frames
	
	
	for (i = 1; i < chromoLength; i++){//changed to add 1
		j = min(i + startingFrame, (int) dMatrix->size()-1 );//in case out of range
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


void testApp::drawChromoGram(){
	
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

void testApp::drawDoubleMatrix(DoubleMatrix* dMatrix){

	
	if ((*dMatrix).size()>0){
		int matrixWidth = (*dMatrix).size();
		int matrixHeight = (*dMatrix)[0].size();
		
	float screenHeight = ofGetHeight() ;
	float screenWidth = ofGetWidth();
/*	float heightFactor = 8;
	int i, j, startingFrame;
	startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
	startingFrame *= scrollWidth;//starting frame in terms of energy frames
		startingFrame /= conversionFactor;//CHROMA_CONVERSION_FACTOR; //in terms of chroma frames
	

 float chromoLength = scrollWidth/conversionFactor;// CHROMA_CONVERSION_FACTOR;
 for (i = 0; i < chromoLength; i++){
 j = min(i + startingFrame, (int) dMatrix->size()-1 ) ;
 for (int y = 0;y < 12;y++){
 ofSetColor(0,0,255 * (*dMatrix)[j][11-y]);//, 0;
 ofRect(i*screenWidth/chromoLength,y*screenHeight/12,screenWidth/chromoLength,screenHeight/12);
 }//end y
 }//end i
 
 */
		
		float ratio = max(matrixWidth/screenWidth,matrixHeight/screenHeight);
		for (int x = 0; x < screenWidth; x+= 5){
		//j = min(i + startingFrame, (int) dMatrix->size()-1 ) ;
		for (int y = 0;y < screenHeight;y+= 5){
			int xIndex = (int)(x*ratio);
			int yIndex = (int)(y*ratio);
			
			if (xIndex < matrixWidth && yIndex < matrixHeight)
				ofSetColor(0,0,255 * (*dMatrix)[xIndex][yIndex]);//, 0;
			else 
				ofSetColor(0,0,0);
			
			ofRect(x,y,5,5);//screenWidth/matrixWidth,screenHeight/matrixHeight);
		}//end y
	}//end i
	
	}///end if matrix has content
	else{
		printf("Error - please load audio first");
	}
	
	
}


void testApp::drawSimilarityMatrix(){

	int simHeight = (tw.similarityMatrix[0]).size();
	int simWidth = tw.similarityMatrix.size();
	
	int sizeOfMatrix = (int) tw.similarityMatrix.size();//tw.chromaMatrix.size();
	int sizeOfSecondMatrix = (int) tw.similarityMatrix[0].size();
	
	//in chromagram frames
	int startingXframe = tw.chromaSimilarityMatrix.size() / (scrollWidth/conversionFactor);
	int startingYframe = tw.chromaSimilarityMatrix[0].size() / (scrollWidth/conversionFactor);
	

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




void testApp::drawChromaSimilarityMatrix(){
	//scrollwidth is width in terms of the similarity matrix size (or energy size if we are being accurate)
	//need to get rid of dependency on sim matirx and only use chrom sim matrix
	
//	int simHeight = (tw.similarityMatrix[0]).size();
//	int simWidth = tw.similarityMatrix.size();
	
//	int sizeOfMatrix = (int) tw.similarityMatrix.size();//tw.chromaMatrix.size();
//	int sizeOfSecondMatrix = (int) tw.similarityMatrix[0].size();
	
	float chromogramScrollWidth = (scrollWidth/conversionFactor);
	//frames needed in energy still
	//in chromagram frames
	int startingXframe = (tw.firstEnergyVector.size() / scrollWidth);
	int startingYframe = (tw.secondEnergyVector.size() / scrollWidth);//secondMatrix
	
	//	printf("DRAW SIM SIZE start frames  %i x %i \n", startingXframe, startingYframe);
	if (tw.backwardsAlignmentPath.size() > 0 ){ 
		startingXframe = (tw.backwardsAlignmentPath[0][backwardsAlignmentIndex]/ scrollWidth);
		startingYframe = max(0	, (int)(tw.backwardsAlignmentPath[1][backwardsAlignmentIndex]/ scrollWidth));//*conversionFactor 
		//FIX THE 1 - ASDDED AS DEBUG
	//	printf("alignment index %i, VERSUS DRAW SIM SIZE %i x %i \n", backwardsAlignmentIndex, startingXframe, startingYframe);
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
	
	int *indexOfAlignmentPathTested;
	int lengthOfPath = 0;
	if (tw.backwardsAlignmentPath.size() > 0)
		lengthOfPath = tw.backwardsAlignmentPath[0].size()-1;
	
	indexOfAlignmentPathTested = &lengthOfPath;
	
	int xcoord, ycoord;
	for (int x = 0;x < screenWidth;x++)
	{
		for (int y =0;y < screenHeight;y++){
			
			xcoord = (x / screenWidth) * scrollWidth;//i.e.  chromoLength * conversionFactor;
			xcoord += startingXframe;
			
			int xChromaCoord = xcoord / conversionFactor;
			
			ycoord = y * chromoLength * conversionFactor/ screenHeight;
			ycoord += startingYframe;
			
			int yChromaCoord = ycoord / conversionFactor; 
			
			int colour = 0;//0x006644;

			
		/*	if (xcoord < tw.similarityMatrix.size() && ycoord < tw.similarityMatrix[0].size()){//
				//nb not optimised
				colour = tw.similarityMatrix[xcoord][ycoord]*255;
			}
		*/	
			if (xChromaCoord < tw.chromaSimilarityMatrix.size() && yChromaCoord < tw.chromaSimilarityMatrix[0].size()){
			colour = tw.chromaSimilarityMatrix[xChromaCoord][yChromaCoord]*255;
			}
			
			
			ofSetColor(colour,0,0);
			
			ofRect(x,y,1,1);
			
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
	string textString;
/*	textString = "width : ";
	textString += ofToString(simWidth);
	
	textString += "  height : ";
	textString += ofToString(simHeight);
*/	
//	textString += "  startframe : ";
//	textString += ofToString(startingFrame);
	
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









void testApp::drawAlignmentmeasureValues(const int& startingYframe){
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

void  testApp::checkIfAudioPositionExceedsWidthForFirstFile()
{
	if (currentPlayingFrame > scrollWidth*(numberOfScrollWidthsForFirstFile+1))
		numberOfScrollWidthsForFirstFile++;
}

int testApp::findStartWidthFrame(){
	int startingFrame;
	/*
		startingFrame = currentPlayingFrame / scrollWidth;//i.e. number of scroll widths in
		startingFrame *= scrollWidth;//starting frame in terms of energy frames
	startingFrame /= conversionFactor;// CHROMA_CONVERSION_FACTOR; 
*/
 return startingFrame;
}

void testApp::drawAlignmentPath(int startingChromaXFrame, int startingChromaYFrame, IntMatrix* backPath){
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







void testApp::drawForwardsAlignmentPath(int startingChromaXFrame, int startingChromaYFrame){
	
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





void testApp::drawForwardsAlignmentPathOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame){
	
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
		
		while (tw.forwardsAlignmentPath[0][index] < startingXFrame){
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



void testApp::drawAnchorPointsOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame){
	
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






void testApp::loadSoundFiles(){
		
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

void testApp::loadLibSndFile(const char *infilename){
	
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

void testApp::processAudioToDoubleMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector){
	//wendy
	myDoubleMatrix->clear();
	energyVector->clear();

	
	
	chromaG.initialise(FRAMESIZE, CHROMAGRAM_FRAMESIZE);//framesize 512 and hopsize 2048 - already done
	chromaG.maximumChromaValue = 1;
	double maximumEnergyValue = 1;
	
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
		if (energyValue > maximumEnergyValue)
			maximumEnergyValue = energyValue;
		
		
	}//end while readcount
	
	printf("Max chroma value is %f \n", chromaG.maximumChromaValue);
	printf("length of chromagram is %d frames\n", (int)myDoubleMatrix->size());
	printf("height of dmatrix is %d\n", (int)(*myDoubleMatrix)[0].size());
	//normalise chroma matrix	
	for (int i = 0; i < myDoubleMatrix->size();i++){
		for (int j = 0; j < ((*myDoubleMatrix)[0]).size();j++){
			//non-causal normalisation
			(*myDoubleMatrix)[i][j] /= chromaG.maximumChromaValue;	
		}
	}
	

	printf("size of energy vector is %d \n", (int)energyVector->size());	
	//non causal normalisation
	for (int i = 0; i < energyVector->size();i++){
		(*energyVector)[i] /= maximumEnergyValue;
	}
	
//	totalNumberOfFrames = (int)energyVector->size();
	chromaConversionRatio = myDoubleMatrix->size() / (int)energyVector->size();
	
//	int size = myDoubleMatrix->size() * CHROMA_CONVERSION_FACTOR;
	
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
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
		
		loadSecondAudio(secondFileName);
		
		initialiseVariables();
		backwardsAlignmentIndex = 0;
		
		calculateSimilarityAndAlignment();



	}
	
	if (key == 'f'){
		tw.printBackwardsPath(0, (int) tw.forwardsAlignmentPath[0].size(), &tw.forwardsAlignmentPath);
	}
	
	if (key == 'b'){
		tw.printBackwardsPath(0, (int) tw.backwardsAlignmentPath[0].size(), &tw.backwardsAlignmentPath);
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
void testApp::keyReleased  (int key){
	if (key == OF_KEY_LEFT || OF_KEY_RIGHT){
		(*playingAudio).setSpeed(1);
		if (tw.backwardsAlignmentPath.size() > 0)
		backwardsAlignmentIndex = tw.backwardsAlignmentPath[0].size()-1;
	}
	
}

void testApp::openNewAudioFileWithdialogBox(){
	
	//open audio file
	string *filePtr;
	filePtr = &soundFileName;	
	
	if (getFilenameFromDialogBox(filePtr)){
		printf("Mainfile: Loaded name okay :\n'%s' \n", soundFileName.c_str());	
	}
	
	//openFileDialogBox(); - replaced this lone by call to openFile Dialoguebox
	loadNewAudio(soundFileName);

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	width = ofGetWidth();
	pan = (float)x / (float)width;
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	targetFrequency = 2000.0f * heightPct;
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	xIndex = (int)(pan*ENERGY_LENGTH);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	width = ofGetWidth();
	pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	bNoise = true;
	//moveOn = true;
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	bNoise = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
width = w;
height = h;
	screenHeight = ofGetHeight() ;
	screenWidth = ofGetWidth();
	
}
//--------------------------------------------------------------
void testApp::audioRequested 	(float * output, int bufferSize, int nChannels){
	//pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}


}


//--------------------------------------------------------------


bool testApp::getFilenameFromDialogBox(string* fileNameToSave){
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

void testApp::openFileDialogBox(){

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


void testApp::loadNewAudio(string soundFileName){
	//need to add in clear fns
//	tw.chromaMatrix.clear();
//	tw.firstEnergyVector.clear();
	
	loadedAudio.loadSound(soundFileName);
	playingAudio = &loadedAudio;
	//snd file method
	const char	*infilename = soundFileName.c_str() ;
	loadLibSndFile(infilename);
	
//	loadFirstAudioFile();
	processAudioToDoubleMatrix(&tw.chromaMatrix, &tw.firstEnergyVector);
	audioPlaying = false;
}



void testApp::loadFirstAudioFile(){

	processAudioToDoubleMatrix(&tw.chromaMatrix, &tw.firstEnergyVector);

	
}

void testApp::loadSecondAudio(string sndFileName){

	secondAudio.loadSound(sndFileName);

	const char	*infilenme = sndFileName.c_str() ;	
	loadLibSndFile(infilenme);


	processAudioToDoubleMatrix(&tw.secondMatrix, &tw.secondEnergyVector);
	
}


void testApp::swapBetweenPlayingFilesUsingAlignmentMatch(){
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

int testApp::findMatchFromAlignment(bool whichFileToTest){
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

void testApp::printSimilarityMatrix(int sizeToPrint){
	
	printf("\n _ _ _ _\n");
	printf("Similarity Matrix \n");
	int i,j;
	DoubleVector d;
	int rowSize = sizeToPrint;
	
	for (int j = 0;j < rowSize;j++){
		printf("row %i : ", j);
		
		for (i = 0;i < rowSize;i++){			
			printf("%f , ", tw.similarityMatrix[i][j] );
		}
		printf("\n");
	}
	printf("...............\n");
	
}



void testApp::printAlignmentMatrix(const DoubleMatrix& alignmentMatrix){
	
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


void testApp::printScoreForRow(int row, int max){
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






void testApp::dontDoJunkAlignment(){
	
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
void testApp::pushSimCoxde(){
 /*	for (int i = 0;i < tw.tmpSimilarityMatrix.size();i++){
 DoubleVector v;
 v = tw.tmpSimilarityMatrix[i];
 tw.similarityMatrix.push_back(v);
 }
 */	
 
 /*
 void testApp::calculateSimilarityMatrix(){
 similarityMatrix.clear();
 printf("calculating similarity matrix...");
 userInfoString = "calculating similarity matrix...";
 
 double distance, firstSum, secondSum;
 
 for (int x = 0;x < tw.chromaMatrix.size();x++){
 DoubleVector d;
 for (int y = 0;y < tw.secondMatrix.size();y++){
 
 //d.push_back( drand48() );	
 
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
 userInfoString += "; size =";
 userInfoString += ofToString(similarityMatrix.size() , 0);
 printf("..sim size: %i, height: %i \n", similarityMatrix.size(), (chromaMatrix[0]).size());
 
 }//end self sim
 
 */
/*
 void testApp::calculateAlignmentMatrix(){
 
 //initialise alignment
 alignmentMeasureMatrix.clear();
 DoubleVector d;
 d.push_back(getDistance(0,0));
 alignmentMeasureMatrix.push_back(d);
 
 bool chromaCalculated = false;
 bool secondCalculated = false;
 
 while (!chromaCalculated || !secondCalculated) {
 
 if (!chromaCalculated)
 chromaCalculated = extendAlignmentAlong();
 
 if (!secondCalculated)
 secondCalculated = extendAlignmentUp();
 
 }
 
 }
 
 bool testApp::extendAlignmentUp(){
 DoubleVector d;
 d = alignmentMeasureMatrix[0];
 int heightSize = d.size();
 if (heightSize < secondMatrix.size()){
 //then we haven't finished yet
 for (int i = 0;i < alignmentMeasureMatrix.size();i++){
 double value = getDistance(i, heightSize);
 value += getMinimum(i, heightSize, value);	
 alignmentMeasureMatrix[i].push_back(value);
 }
 }
 if (alignmentMeasureMatrix[0].size() == secondMatrix.size())
 return true;
 else
 return false;
 
 }
 
 
 bool testApp::extendAlignmentAlong(){
 DoubleVector d;
 int widthSize = alignmentMeasureMatrix.size();
 if (widthSize < chromaMatrix.size()){
 //then we can extend along
 double value = getDistance(widthSize, 0);
 value += getMinimum(widthSize, 0, value);
 
 d.push_back(value);
 alignmentMeasureMatrix.push_back(d);
 
 for (int j = 1;j < alignmentMeasureMatrix[widthSize - 1].size();j++){
 value = getDistance(widthSize, j);
 value += getMinimum(widthSize, j, value);
 alignmentMeasureMatrix[widthSize].push_back(value);
 }
 
 //alignmentMeasureMatrix.push_back(d);
 }
 
 if (alignmentMeasureMatrix.size() == chromaMatrix.size())
 return true;
 else
 return false;
 
 }
 
 
 void testApp::calculateMinimumAlignmentPath(){
 //this requires one pass of the DTW algorithm and then works backwards from (N,M)
 //to find the optimal path to (0,0), where N and M are the lengths of the two chromoVectors respectively
 //minimumAlignmentPath.clear();
 backwardsAlignmentPath.clear();
 
 printf("Finding minimum Path \n");
 IntVector v;
 v.push_back(chromaMatrix.size()-1);
 backwardsAlignmentPath.push_back(v);
 v.clear();
 v.push_back(secondMatrix.size()-1);
 backwardsAlignmentPath.push_back(v);
 //so now backwards path[0][0] = size(chroma) and path[1][0] = size(secondMatrix)
 printf("backwards path %i : %i \n", backwardsAlignmentPath[0][0], backwardsAlignmentPath[1][0]);
 
 
 int indexOfBackwardsPath = 0;
 while (!findPreviousMinimumInBackwardsPath())	{
 indexOfBackwardsPath++;
 printf("backwards path %i : %i \n", backwardsAlignmentPath[0][indexOfBackwardsPath], backwardsAlignmentPath[1][indexOfBackwardsPath]);
 
 }
 //printf("final index of backwards path is %i and i is %i \n", backwardsAlignmentPath[0].size()-1, indexOfBackwardsPath);
 
 backwardsAlignmentIndex = backwardsAlignmentPath[0].size()-1;//remember that this goes backwards!
 
 }
 
 
 bool testApp::findPreviousMinimumInBackwardsPath(){
 int chromaPosition, secondPosition;
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
 chromaPosition = i-1;
 secondPosition = j;
 finishedAligning = false;
 }
 
 if (j>0 && testForNewAlignmentMinimum(ptr, i-1, j-1)){
 chromaPosition = i-1;
 secondPosition = j-1;
 finishedAligning = false;
 }
 }
 
 if (j > 0 && testForNewAlignmentMinimum(ptr, i, j-1)){
 chromaPosition = i;
 secondPosition = j-1;
 //newMinimum = alignmentMeasureMatrix[chromaPosition][secondPosition];
 finishedAligning = false;
 }
 
 if (!finishedAligning){
 backwardsAlignmentPath[0].push_back(chromaPosition);
 backwardsAlignmentPath[1].push_back(secondPosition);
 }
 
 return finishedAligning;
 
 }	
 
 
 
 bool testApp::testForNewAlignmentMinimum(double *previousMinimum, int i, int j){
 bool newMinimumFound = false;
 if (alignmentMeasureMatrix[i][j] < *previousMinimum){
 *previousMinimum = alignmentMeasureMatrix[i][j];							   
 newMinimumFound = true;
 }
 
 return newMinimumFound;							   
 }		
 */ 


/*
 int testApp::findMinimumOfVector(DoubleVector *d){
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
 */
/*
 double testApp::getDistance(int i, int j){
 return (1 - similarityMatrix[i][j]);
 }
 
 double testApp::getMinimum(int i, int j, float newValue){
 double minimumValue = 0;
 
 if (i > 0){
 minimumValue = tw.alignmentMeasureMatrix[i-1][j];
 if (j > 0){
 minimumValue = min(minimumValue, alignmentMeasureMatrix[i-1][j-1] + newValue ) ;//penalises diagonal by 2
 minimumValue = min(minimumValue, alignmentMeasureMatrix[i][j-1]);
 }
 }
 else{//i.e. i == 0 
 if (j > 0)
 minimumValue = tw.alignmentMeasureMatrix[i][j-1];
 }
 
 return minimumValue;
 }
 
 */

