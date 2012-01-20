/*
 *  OnlineWarpHolder.h
 *  ofxOnlineTimeWarp
 *
 *  Created by Andrew on 18/01/2012.
 *  Copyright 2012 QMUL. All rights reserved.
 *
 */

#ifndef _ONLINE_WARP_HOLDER
#define _ONLINE_WARP_HOLDER


#include "ofMain.h"
#include "chromaGram.h"
#include "ChordDetect.h"
#include "sndfile.h"
#include "ofxFileDialogOSX.h"
#include "timeWarp.h"
#include "OnsetDetectionFunction.h"
#include "ofxSoundFileLoader.h"



#define FRAMESIZE 512
#define ENERGY_LENGTH 80000
#define CHROMA_LENGTH 12000
#define CHROMA_CONVERSION_FACTOR 16 //16 times as many frames in energy as in chroma
#define CHROMAGRAM_FRAMESIZE 2048
#define FILE_LIMIT 600000

//length in terms of frames (at 512 samples per frame - there are 90 per second) => 900: 10 seconds

class OnlineWarpHolder{
	
public:
	
	OnlineWarpHolder();
	
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
//	void audioRequested 	(float * input, int bufferSize, int nChannels);
	void loadSndfile();
	//double getEnergyOfFrame();
	
	void drawAlignmentmeasureValues(const int& startingYframe);
	void drawChromoGram();
	

	void initialiseVariables();
	
	void clearVectors();
	
	void calculateSimilarityAndAlignment();
	
	typedef std::vector<double> DoubleVector;
	typedef std::vector<DoubleVector> DoubleMatrix;
	
	typedef std::vector<int> IntVector;
	typedef std::vector<IntVector> IntMatrix;
	
	DoubleMatrix* matrixPtr;
	
	void drawDoubleMatrix(DoubleMatrix* dMatrix);//DoubleMatrix* dMatrix); WOULD BE NICE TO USE POINTER BUT NOT WORKING YET
	void drawSpectralDifference(DoubleMatrix* dMatrix);
	
	//	DoubleVector firstEnergyVector;
	//	DoubleVector secondEnergyVector;	
	
	
	//	DoubleMatrix similarityMatrix;
	void calculateSimilarityMatrix();
	void calculateFirstForwardsAlignment();//forwards causal alignment version
	void calculateSecondForwardsAlignment();//based on Y
	void resetForwardsPath();
	void computeAlignmentForFirstBlock(const int& startFrameX);
	void computeAlignmentForSecondBlock(const int& startFrameY);
	
	bool checkAlignmentWindow();
	void updateCausalAlignment();
	
	
	int alignmentHopsize, alignmentFramesize;
	bool drawSimilarity;
	void drawSimilarityMatrix();
	void printSimilarityMatrix(int sizeToPrint);
	void printChromaSimilarityMatrix(int sizeToPrint);
	void printChromagramMatrix(int sizeToPrint, DoubleMatrix& matrix);
	void setConversionRatio();
	void printVariousMatrixInfo();
	void doPathBugCheck();
	
	void printForwardsPath();
	void printAlignmentMatrix(const DoubleMatrix& alignmentMatrix, int sizeToPrint);
	void drawChromaSimilarityMatrix();
	
	//	DoubleMatrix alignmentMeasureMatrix;
	
	//	DoubleVector minimumAlignmentPath;
	
	void drawAlignmentPath(int startingChromaXFrame, int startingChromaYFrame, IntMatrix* backPath);
	void drawForwardsAlignmentPath(int startingChromaXFrame, int startingChromaYFrame);
	int findStartWidthFrame();	  
	
	void printScoreForRow(int row, int max);
	
	int numberOfScrollWidthsForFirstFile;
	int numberOfScrollWidthsForSecondFile;
	
	void checkIfAudioPositionExceedsWidthForFirstFile();
	
	
	//	IntMatrix backwardsAlignmentPath;
	int backwardsAlignmentIndex;//used for drawing the path
	
	void updateAlignmentPathIndex(int idenifier);
	
	//	bool checkWhetherOnAlignmentPath(int xcoord, int ycoord, int *indexOfAlignmentPathTested);
	
	bool findPreviousMinimumInBackwardsPath();
	bool testForNewAlignmentMinimum(double *previousMinimum, int i, int j);	
	
	void calculateAlignmentMatrix();
	//	void performNextAlignment();
	double getDistance(int i, int j);
	void printAlignmentMatrix(const DoubleMatrix& alignmentMatrix);
	double getMinimum(int i, int j, float newValue);
	bool extendAlignmentUp();
	bool extendAlignmentAlong();
	void calculateMinimumAlignmentPath();
	int findMinimumOfVector(DoubleVector *d);
	void swapBetweenPlayingFilesUsingAlignmentMatch();
	int findMatchFromAlignment(bool whichFileToTest);
	
	void drawEnergyVectorFromPointer(DoubleVector* energyVec);
	void drawForwardsAlignmentPathOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame);
	void drawAnchorPointsOnChromaSimilarity(const int& startingXFrame, const int& startingYFrame);
	void doSequentialAnalysis(float* frame, DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	void resetSequentialAnalysis();
	
//	void alternativeCausalForwardsAlignment();
//	void newAnchorPointReached();
	
	void processAudioToDoubleMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	void processAudioToMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	//online version
	bool processFrameToMatrix(float newframe[], DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	
	void iterateThroughAudioMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	void extendChromaSimilarityMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	
	void resetMatrix(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	void printMatrixData(DoubleMatrix* myDoubleMatrix, DoubleVector* energyVector);
	
	void loadFirstAudio(string soundFileName);
	void loadSecondAudio(string sndFileName);
	
	void loadSoundFiles();
	void openFileDialogBox();
	void loadLibSndFile(const char * filename);
	bool getFilenameFromDialogBox(string* fileNameToSave);
	void openNewAudioFileWithdialogBox();		
	
	//int* firstAudioLength, secondAudioLength;
	
	string firstFileName, secondFileName, soundFileName;
	
	float screenHeight, screenWidth;
	
	double energyMaximumValue;
	
//	float 	pan;
//	int		sampleRate;
	bool 	bNoise;
	float 	volume;
	
//	float 	* lAudio;
//	float   * rAudio;
	
	bool moveOn;
	bool drawSpectralDifferenceFunction;
	float frame[FRAMESIZE]; 
	int frameIndex;
	float energy[ENERGY_LENGTH];
	float secondEnergy[ENERGY_LENGTH];	
	
	float chromoGramVector[CHROMA_LENGTH][12];
	int rootChord[CHROMA_LENGTH];
	
	//	int energyIndex;
	//	int totalFrames;
	
	int scrollWidth;// 1600
	float chromoLength;
	
	bool audioPlaying, audioPaused;
	bool drawSecondMatrix;
	
	float diagonalPenalty;
	
	
	
	string sndfileInfoString, textString;
	int xIndex;
	
	bool firstAudioFilePlaying;
	ofSoundPlayer loadedAudio;
	ofSoundPlayer secondAudio;
	ofSoundPlayer *playingAudio;
	
	float audioPosition;
	float width, height;
	int chromaIndex;	
	//	int totalNumberOfFrames;
	int currentPlayingFrame;
	int currentChromaFrame ;
	string chordString;
	
	//Chromagram* chromoGrammPtr;
	Chromagram chromoGramm;
	Chromagram secondChromoGramm;
	string userInfoString;
	ChordDetect chord;
	//sndfile part
	SNDFILE *infile; // define input and output sound files
	SF_INFO sfinfo ; // struct to hold info about sound file
	
	float chromaConversionRatio;//not needed but could be useful
	TimeWarp tw;
	Chromagram chromaG;
	OnsetDetectionFunction* onset;
	
	float conversionFactor;
	
	void dontDoJunkAlignment();
	void calculateCausalAlignment();
	bool doCausalAlignment;
	
	int anchorStartFrameX;
	int anchorStartFrameY;
	bool sequentialAlignment;
	float sequentialBlockRatio;
	
	ofxSoundFileLoader* soundFileLoader;
};

#endif
