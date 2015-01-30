#include "ofApp.h"
#define NUMBER_OF_KEYS 10 
#define NUMBER_OF_LIGHT_SENSORS 2


//--------------------------------------------------------------
void ofApp::setup(){
    //loading a sound in
    player.loadSound("Hello.mp3");
   // player.play();
    
    cout << "hello" << endl;
    
    ofSoundStreamSetup(2, 0, this, 44100, 256, 4);
    
    /*
     Generators and ControlGenerators both output a steady stream of data.
     Generators output at the sample rate (in this case, 44100 hertz.
     ControlGenerators output at the control rate, which is much lower than the sample rate.
     */
    
    // create a named parameter on the synth which we can set at runtime
    ControlGenerator midiNote = synth.addParameter("midiNumber");
    
    // convert a midi note to a frequency (plugging that parameter into another object)
    ControlGenerator noteFreq =  ControlMidiToFreq().input(midiNote);
    
    // Here's the actual noise-making object
    Generator tone = SawtoothWave().freq( noteFreq );
    
    // Let's put a filter on the tone
    tone = LPF12().input(tone).Q(2).cutoff((noteFreq * 2) + SineWave().freq(0.2) * 0.5 * noteFreq);
    
    // It's just a steady tone until we modulate the amplitude with an envelope
    ControlGenerator envelopeTrigger = synth.addParameter("trigger");
    Generator toneWithEnvelope = tone * ADSR().attack(0.01).decay(1.5).sustain(0).release(0).trigger(envelopeTrigger).legato(true);
    
    // let's send the tone through some delay
    Generator toneWithDelay = StereoDelay(0.5,0.75).input(toneWithEnvelope).wetLevel(0.1).feedback(0.2);
    
    synth.setOutputGen( toneWithDelay );
    ///arduio crap
    ard.connect("/dev/tty.usbmodem1411", 57600);
    ofAddListener(ard.EInitialized, this, &ofApp::setupArd);
    
    for (int i =0; i < NUM_ARDS; i++) {
        ardVals.push_back(0);
    }
    
}

void ofApp::trigger(){
    // sounds modulation is controlled by scaleDegree here, which is affected by average
    static int twoOctavePentatonicScale[10] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21};
    
    // takes the largest integral value that is produced by the formula using the Arduino value (average) to select a value between the pentatonic scale above, hence clamped of 0-9
    int degreeToTrigger = floor(ofClamp(scaleDegree, 0, 9));
    
    // set a parameter that we created when we defined the synth
    synth.setParameter("midiNumber", 44 + twoOctavePentatonicScale[degreeToTrigger]);
    
    // simply setting the value of a parameter causes that parameter to send a "trigger" message to any
    // using them as triggers
    synth.setParameter("trigger", 1);
    
}

//--------------------------------------------------------------
void ofApp::setScaleDegreeBasedOnMouseX(){
    int newScaleDegree = ofGetMouseX() * NUMBER_OF_KEYS / ofGetWindowWidth();
    if(ofGetMousePressed() && ( newScaleDegree != scaleDegree )){
        scaleDegree = newScaleDegree;
        trigger();
    }else{
        scaleDegree = newScaleDegree;
    }
}


void ofApp::analogPinChanged(const int & pinNum) {
    cout << "data recieved" << endl;
    for (int i=0; i<NUMBER_OF_LIGHT_SENSORS; i++) {
    float value = ard.getAnalog(pinNum);
    float valueMapped = ofMap(value, 0, 420, 0, 1023);
    switch (pinNum) {
        case 0:
            ardVals[0] = valueMapped;
            break;
        case 1:
            ardVals[1] = valueMapped;
            break;
        default:
            break;
    }
    float average =0;
    //average = (ardVals[0] + ardVals[1])/ardVals.size(); - how does .size() work?
    average = (ardVals[0] + ardVals[1])/NUMBER_OF_LIGHT_SENSORS;

    cout << average << endl;
    //cout << "data recieved" << endl;
    trigger();
        
    // HERE IS WHERE WE START TO TAKE THE AVERAGE VALUE AND USE IT TO CHANGE THE SYNTH PARAMETERS
    int newScaleDegree = average * NUMBER_OF_KEYS / ofGetWindowWidth();
    if(ofGetMousePressed() && ( newScaleDegree != scaleDegree )){
        scaleDegree = newScaleDegree;
        trigger();
    }else{
        scaleDegree = newScaleDegree;
    }
    
    // valueTouch1 and valueTouch2 are inputs for the touch sensors. At the moment I have them connected to a sound clip but as a trigger so it would be good to switch to a changing pitch as well.
    // digital out 9 controls the LEDs in the tree tops.
    
    float valueTouch1 = ard.getAnalog(2);
    float valueTouch2 = ard.getAnalog(3);
        
    //ard.sendPwm(7, valueSound);
        
    // valueSound are inputs for the voice sensor
    // digital out 7 controls the LEDs in the cloud.

    if (valueTouch1 < 500) {
        ard.sendDigital(8, ARD_LOW);
        //player.stop();
    }   else if (valueTouch1 > 500) {
        ard.sendDigital(8, ARD_HIGH);
        player.play();
        DELAY(2000);
        //player.stop();
    }
    
    
    float valueSound = ard.getAnalog(4);
        
    if (valueSound < 650) {
        ard.sendDigital(7, ARD_LOW);
        //player.stop();
    }   else if (valueSound > 650) {
        ard.sendDigital(7, ARD_HIGH);
        player.play();
        DELAY(2000);
        //player.stop();
    }
        
    //ard.sendPwm(7, valueSound);
        
   

    
//    int lightVal = 1023 - ardVals[0];
//    cout << lightVal << endl;
    //create light that scales with average val
    //ard.sendPwm(9, (average));

    DELAY(1000);
    
    //create a modulating RGB light based on average val
    
    // RGB is on when in LOW condition
    if (average < 200) {
        ard.sendDigital(9, ARD_LOW);
        ard.sendDigital(10, ARD_HIGH);
        ard.sendDigital(11, ARD_HIGH);
    } else if (average >= 200 && average < 350) {
        ard.sendDigital(9, ARD_LOW);
        ard.sendDigital(10, ARD_LOW);
        ard.sendDigital(11, ARD_HIGH);
    } else if (average >= 350 && average < 500) {
        ard.sendDigital(9, ARD_HIGH);
        ard.sendDigital(10, ARD_LOW);
        ard.sendDigital(11, ARD_HIGH);
    } else if (average >= 500 && average < 650) {
        ard.sendDigital(9, ARD_HIGH);
        ard.sendDigital(10, ARD_LOW);
        ard.sendDigital(11, ARD_LOW);
    } else if(average >= 650) {
        ard.sendDigital(9, ARD_HIGH);
        ard.sendDigital(10, ARD_HIGH);
        ard.sendDigital(11, ARD_LOW);
    }
    }
    
    //float valueTouch = ard.getAnalog(3);
    //float valueTouchMapped = ofMap(valueTouch, 350, 670, 0, 1024);
    
    for (int i=0; i<5; i++) {
        cout << "ard val " << i << "=" << ard.getAnalog(i) << endl;
    }
}

void ofApp::setupArd(const int &version){
    cout << "working!" << endl;
    ofRemoveListener(ard.EInitialized, this, &ofApp::setupArd);
    //this is the output on the arduino for an analog sensor!!! :)
    
    ard.sendAnalogPinReporting(0, ARD_ANALOG);
    ard.sendAnalogPinReporting(1, ARD_ANALOG);
    ard.sendAnalogPinReporting(2, ARD_ANALOG);
    ard.sendAnalogPinReporting(3, ARD_ANALOG);
    ard.sendAnalogPinReporting(4, ARD_ANALOG);
    ofAddListener(ard.EAnalogPinChanged, this, &ofApp::analogPinChanged);
    
    // if want to set pin D11 as PWM (analog output)
    //ard.sendDigitalPinMode(9, ARD_PWM);
    //ard.sendDigitalPinMode(10, ARD_PWM);
    //ard.sendDigitalPinMode(11, ARD_PWM);
    
    // send the project digital output
    ard.sendDigitalPinMode(7, ARD_OUTPUT);
    ard.sendDigitalPinMode(9, ARD_OUTPUT);
    ard.sendDigitalPinMode(10, ARD_OUTPUT);
    ard.sendDigitalPinMode(11, ARD_OUTPUT);
    
    //like say you'd want to send in data instead, do this.
    //ard.sendDigitalPinMode(9, ARD_INPUT);
    cout << "up" <<endl;
}
//--------------------------------------------------------------
void ofApp::update(){
    //update Arduino
    ard.update();

}

//--------------------------------------------------------------
void ofApp::draw(){
    float keyWidth = ofGetWindowWidth()/NUMBER_OF_KEYS;
    for (int i =0; i< NUMBER_OF_KEYS; i++) {
        if((i==scaleDegree) && ofGetMousePressed())
        {
            ofSetColor(255,151,0);
        }else{
            int brightness = 100 + (55*i/NUMBER_OF_KEYS);
            ofSetColor(brightness, brightness, brightness);
        }
        ofRect(keyWidth*i, 0, keyWidth, ofGetWindowHeight());
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
        setScaleDegreeBasedOnMouseX();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    trigger(); 
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
void ofApp::audioRequested (float * output, int bufferSize, int nChannels){
    synth.fillBufferOfFloats(output, bufferSize, nChannels);
    
}