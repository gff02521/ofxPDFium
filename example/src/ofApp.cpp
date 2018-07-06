#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofxPDFium::PdfLibInit();

	pdf = new ofxPDFium("data/test.pdf");
	pdf->loadPage(0);
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofxPDFium::PdfLibCleanup();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255);
	pdf->drawToScale(10, 40, .75);

	ofSetColor(0);
	ofDrawBitmapString("Left click for next page/Right click for previous page", 10, 15);
	ofDrawBitmapString("Current Page = " + ofToString(pdf->getCurrentPage()), 10, 30);
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

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (button == 0)
		pdf->loadPage(pdf->getCurrentPage() + 1);
	if (button == 2)
		pdf->loadPage(pdf->getCurrentPage() - 1);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
