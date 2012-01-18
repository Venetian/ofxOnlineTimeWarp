#include "testApp.h"


void testApp::setup(){

	warpHolder = new OnlineWarpHolder();
	warpHolder->setup();
	
	ofBackground(255,255,255);

	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	


//DONT NEED ANY OF THIS
	sampleRate 			= 44100;

	lAudio = new float[256];
	rAudio = new float[256];
	ofSoundStreamSetup(2, 0, this, sampleRate,256, 4);
//UNTIL HERE
	
	
	ofSetFrameRate(30);
	
		
}
 
void testApp::exit(){
	warpHolder->exit();
}



//--------------------------------------------------------------
void testApp::update(){
	
	warpHolder->update();

	ofSoundUpdate();
	
	
}
//-------------------------------------------------
void testApp::draw(){
	warpHolder->draw();

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	warpHolder->keyPressed(key);

}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){
	warpHolder->keyReleased(key);
		
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	warpHolder->mouseMoved(x, y);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	warpHolder->mouseDragged(x, y, button);
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	warpHolder->bNoise = true;
	
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	warpHolder->bNoise = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	warpHolder->windowResized(w, h);
	
}
//--------------------------------------------------------------
void testApp::audioRequested 	(float * output, int bufferSize, int nChannels){

}




