#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){	 
	
	
	
	warpHolder = new OnlineWarpHolder();
	warpHolder->setup();
	
	drawWarpMatrix = true;
	onlineAnalysis = true;
	causalAnalysisStarted = false;
	warpHolder->realTimeAnalysisMode = &causalAnalysisStarted;
	
	inputReceived = false;
	outputRequested = true;
	
	ofSetVerticalSync(true);
	ofSetCircleResolution(80);
	ofBackground(54, 54, 54);	
	
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	soundStream.listDevices();
	soundStream.setDeviceID(1);//this now uses the audio input rather than mic input for mac 
	outputStream.setDeviceID(2);
	//if you want to set a different device id 
	//soundStream.setDeviceID(0); //bear in mind the device id corresponds to all audio devices, including  input-only and output-only devices.
	
	int bufferSize = 512;
	
	sampleRate = 44100;
	
	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	
	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;
	smoothedVol     = 0.0;
	scaledVol		= 0.0;
	pan = 0.5f;
	//output
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume = 0.1f;
	//
	soundStream.setup(this, 0, 2, 44100, bufferSize, 4);
	outputStream.setup(this, 2, 0, 44100, bufferSize, 4);
	
	outputSameAsInput = true;
}

//--------------------------------------------------------------
void testApp::update(){
	
	warpHolder->update();
	/*
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
	//lets record the volume into an array
	volHistory.push_back( scaledVol );	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}
	*/
	
}

//--------------------------------------------------------------
void testApp::draw(){

	//drawAudioInput();
	warpHolder->draw();
	
}


void testApp::drawAudioInput(){
	
	ofSetColor(225);
	ofDrawBitmapString("AUDIO INPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\n'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 170, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Left Channel", 4, 18);
	
	ofSetLineWidth(1);	
	ofRect(0, 0, 512, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (int i = 0; i < left.size(); i++){
		ofVertex(i*2, 100 -left[i]*180.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw the right channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(32, 370, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Right Channel", 4, 18);
	
	ofSetLineWidth(1);	
	ofRect(0, 0, 512, 200);
	
	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);
	
	ofBeginShape();
	for (int i = 0; i < right.size(); i++){
		ofVertex(i*2, 100 -right[i]*180.0f);
	}
	ofEndShape(false);
	
	ofPopMatrix();
	ofPopStyle();
	
	// draw the average volume:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(565, 170, 0);
	
	ofSetColor(225);
	ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);
	ofRect(0, 0, 400, 400);
	
	ofSetColor(245, 58, 135);
	ofFill();		
	ofCircle(200, 200, scaledVol * 190.0f);
	
	//lets draw the volume history as a graph
	ofBeginShape();
	for (int i = 0; i < volHistory.size(); i++){
		if( i == 0 ) ofVertex(i, 400);
		
		ofVertex(i, 400 - volHistory[i] * 70);
		
		if( i == volHistory.size() -1 ) ofVertex(i, 400);
	}
	ofEndShape(false);		
	
	ofPopMatrix();
	ofPopStyle();
	
	drawCounter++;
	
	ofSetColor(225);
	string reportString = "buffers received: "+ofToString(bufferCounter)+"\ndraw routines called: "+ofToString(drawCounter)+"\nticks: " + ofToString(soundStream.getTickCount());
	ofDrawBitmapString(reportString, 32, 589);
	
	
	if( !bNoise ){
		reportString += "sine wave (" + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}else{
		reportString += "noise";	
	}
	ofDrawBitmapString(reportString, 32, 579);
	
	
}


//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){	
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2];
		right[i]	= input[i*2+1];
	}
	
	/*
	
//	if (outputRequested) {
//	printf("audio listener input\n");
//		inputReceived = true;
//		outputRequested = false;
	
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2];
		right[i]	= input[i*2+1];

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
	
	bufferCounter++;
	
*/
	if (causalAnalysisStarted){
		warpHolder->doSequentialAnalysis(&left[0], &warpHolder->tw.secondMatrix, &warpHolder->tw.secondEnergyVector);
	}
	
		
	
		
}


//--------------------------------------------------------------
void testApp::audioOut(float * output, int bufferSize, int nChannels){
	for (int i = 0; i < bufferSize; i++){
		lAudio[i] = output[i*nChannels    ] = left[i]; //* volume * leftScale;
		rAudio[i] = output[i*nChannels + 1] = right[i];// * volume * rightScale;
	}
	
/*
 //	if (inputReceived) {
//		inputReceived = false;
//		outputRequested = true;
	
	//pan = 0.5f;
//	printf("OUPUT CALLED\n");
	float leftScale = 1 - pan;
	float rightScale = pan;
	
	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	
	if (outputSameAsInput) {
		for (int i = 0; i < bufferSize; i++){
			lAudio[i] = output[i*nChannels    ] = left[i]; //* volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = right[i];// * volume * rightScale;
		}
		
	}
	else if ( bNoise == true){
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++){
			lAudio[i] = output[i*nChannels    ] = ofRandom(0, 1) * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = ofRandom(0, 1) * volume * rightScale;
		}
	} else {
		phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
		for (int i = 0; i < bufferSize; i++){
			phase += phaseAdder;
			float sample = sin(phase);
			lAudio[i] = output[i*nChannels    ] = sample * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = sample * volume * rightScale;
		}
	}
	*/
	
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
/*	if( key == 's' ){
		soundStream.start();
	}
	
	if( key == 'e' ){
		soundStream.stop();
	}
	if (key == 'i')
		outputSameAsInput = !outputSameAsInput;
	*/
	
	
	if( key == 'x' && !causalAnalysisStarted){
		printf("Start LIVE causal analysis\n");
		
		causalAnalysisStarted = true;
		warpHolder->resetSequentialAnalysis();
	}
	
	
	warpHolder->keyPressed(key);
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	warpHolder->keyReleased(key);
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	targetFrequency = 2000.0f * heightPct;
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	pan = (float)x / (float)ofGetWidth();
	
	warpHolder->mouseMoved(x, y);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	int width = ofGetWidth();
	pan = (float)x / (float)width;
	
	warpHolder->mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	bNoise = true;	
	warpHolder->mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	bNoise = false;
	warpHolder->mouseReleased(x, y, button);
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	warpHolder->windowResized(w, h);

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}


void testApp::exit(){
	warpHolder->exit();
}
