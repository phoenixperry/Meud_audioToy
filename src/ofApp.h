#pragma once
#define NUM_ARDS 4 

#include "ofMain.h"
#include "ofxTonic.h"
using namespace Tonic;

class ofApp : public ofBaseApp{
private:
    ofxTonicSynth synth;
    int scaleDegree;
    void trigger();
    void setScaleDegreeBasedOnMouseX();

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void audioRequested (float * output, int bufferSize, int nChannels);
    
        void setupArd(const int &version);
        ofArduino ard;
        void analogPinChanged(const int &pinNum);
        float lightLevel;
        vector<float> ardVals;
        ofSoundPlayer note1;
        ofSoundPlayer note2;
        ofSoundPlayer note3;
        ofSoundPlayer note4;
        ofSoundPlayer player;
};
