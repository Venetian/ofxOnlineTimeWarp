#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "OnlineWarpHolder.h"

class testApp : public ofBaseApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		void exit();
	
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	void drawAudioInput();
	
	
		void audioIn(float * input, int bufferSize, int nChannels); 
	
		vector <float> left;//input
		vector <float> right;
		vector <float> volHistory;
	
	void audioOut(float * output, int bufferSize, int nChannels);
	vector <float> lAudio;//output
	vector <float> rAudio;
	bool 	bNoise;
	float pan;
	//------------------- for the simple sine wave synthesis
	float 	targetFrequency;
	float 	phase;
	float 	phaseAdder;
	float 	phaseAdderTarget;
	float volume;
			int		sampleRate;
	
		int 	bufferCounter;
		int 	drawCounter;
		
		float smoothedVol;
		float scaledVol;
		
		ofSoundStream soundStream;
		ofSoundStream outputStream;
	bool outputSameAsInput;
	
	
	OnlineWarpHolder* warpHolder;
	bool drawWarpMatrix;
	
	int audioListenerIndex ;
	bool causalAnalysisStarted;
	bool onlineAnalysis;
	
	bool inputReceived;
	bool outputRequested;
	
};

#endif	

