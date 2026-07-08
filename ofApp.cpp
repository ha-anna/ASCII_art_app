#include "ofApp.h"
#include <algorithm>
#include <cmath>

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowTitle("ASCII Webcam & Image Processing Tool");
	cam.setup(1280, 720);
	
	// 초기 캘리브레이션 기본값 설정
	currentGamma = 0.55f;
	blackLevel = 40;
	whiteLevel = 220;
	
	// 초기 프로그램 상태 설정
	useWebcam = true;
	isCalibrating = false;
	isDisplayInfo = true;
	colorMode = 0;
	
	// 파티클 시스템 초기화 (화면 전체에 무작위로 150개의 빗방울 생성)
	isRainActive = false;
	for (int i = 0; i < 150; i++) {
		MatrixDrop drop;
		drop.x = ofRandom(0, ofGetWidth());
		drop.y = ofRandom(-ofGetHeight(), 0); // 화면 위쪽 바깥에서 대기
		drop.speed = ofRandom(4.0f, 12.0f);
		drop.length = (int)ofRandom(5, 20);
		drop.headChar = (char)ofRandom(33, 126);
		rainDrops.push_back(drop);
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	// 목적: 현재 활성화된 미디어 모드(웹캠 또는 이미지)에 따라 최신 픽셀 데이터 가져오기
	if (useWebcam) {
		cam.update();
		if (cam.isFrameNew()) {
			pixels = cam.getPixels(); // 하드웨어 카메라에서 픽셀 추출
			generateAsciiFrame();
		}
	} else {
		if (loadedImage.isAllocated()) {
			pixels = loadedImage.getPixels(); // 정적 이미지 파일에서 픽셀 추출
			generateAsciiFrame();
		}
	}
	
	if (isRainActive) {
		matrixRain();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	// 매 프레임마다 배경을 완전한 검은색으로 초기화
	ofBackground(0);
	
	// 목적: 현재 선택된 colorMode 인덱스에 따라 렌더링할 텍스트 색상 결정
	switch (colorMode) {
		case 0: terminalColor.set(57, 255, 20);   paletteName = "Green"; break;
		case 1: terminalColor.set(255, 176, 0);   paletteName = "Amber"; break;
		case 2: terminalColor.set(255, 40, 190);  paletteName = "Pink"; break;
		case 3: terminalColor.set(132, 165, 184); paletteName = "Blue"; break;
		case 4: terminalColor.set(255, 255, 255); paletteName = "White"; break;
	}

	// 목적: 벡터 자료구조를 순회하며 ASCII 그리드를 화면에 렌더링
	// 이 반복문은 CRT 모니터 특유의 글로우(Glow) 효과를 내기 위해 두 번에 걸쳐 텍스트를 그립니다.
	for (int row = 0; row < asciiFrame.size(); row++) {
		int yPos = (row + 1) * 14; // 네이티브 폰트 높이에 맞춰 행(row) 간격을 14픽셀로 설정

		// Pass 1: CRT 글로우 효과 (투명도를 낮추고 1픽셀씩 엇갈리게 렌더링)
		ofSetColor(terminalColor.r, terminalColor.g, terminalColor.b, 60);
		ofDrawBitmapString(asciiFrame[row], 0, yPos + 1);
		ofDrawBitmapString(asciiFrame[row], 1, yPos);

		// Pass 2: 선명한 코어 텍스트 (원래 위치에 불투명도 100%로 렌더링)
		ofSetColor(terminalColor.r, terminalColor.g, terminalColor.b, 255);
		ofDrawBitmapString(asciiFrame[row], 0, yPos);
	}
	
	// 모듈화된 파티클 렌더링 함수 호출
	drawMatrixRain();
		
	// HUD 오버레이 메뉴가 텍스트 위에 표시되도록 마지막에 호출
	displayInfo();
}

//--------------------------------------------------------------
char ofApp::brightnessToAscii(int brightness) {
	// 목적: 시각적 무게감이 가장 가벼운 것(공백)부터 무거운 것('@') 순으로 정렬된 ASCII 팔레트
	string chars = " .:-=+*#%@";

	// 수학적 연산을 위해 밝기 값을 0.0 ~ 1.0 비율로 정규화
	float normalizedB = brightness / 255.0f;
	
	// 어두운 영역의 디테일을 살리기 위해 감마 보정(Gamma Correction) 곡선 적용
	float gammaCorrected = pow(normalizedB, currentGamma);
	int adjustedBrightness = gammaCorrected * 255;

	// 보정된 밝기 값을 문자 배열의 인덱스로 매핑 (blackLevel 및 whiteLevel 임계값 적용)
	int index = ofMap(adjustedBrightness, blackLevel, whiteLevel, 0, chars.size() - 1, true);

	return chars[index];
}

//--------------------------------------------------------------
void ofApp::generateAsciiFrame() {
	asciiFrame.clear(); // 새로운 프레임을 위해 벡터 초기화

	// openFrameworks 기본 비트맵 폰트의 네이티브 해상도 비율
	int sampleW = 8;
	int sampleH = 14;
	
	int startX = 0;
	int endX = pixels.getWidth();
	
	// 목적: 16:9 와이드스크린 웹캠 피드의 좌우를 잘라내어 클래식한 4:3 비율(960x720)로 중앙 크롭 처리
	if (useWebcam) {
		startX = (pixels.getWidth() - 960) / 2;
		endX = startX + 960;
	}

	// 목적: 외부 반복문 - 지정된 폰트 높이만큼 세로로 이동하며 행(row) 단위 생성
	for (int y = 0; y < pixels.getHeight(); y += sampleH) {
		string row;

		// 목적: 내부 반복문 - 지정된 폰트 너비만큼 가로로 이동하며 개별 픽셀 블록 평가
		for (int x = startX; x < endX; x += sampleW) {
			
			// 핵심 시간 복잡도 최적화 (O(1)):
			// 블록 내의 모든 픽셀의 평균을 구하는 이중 반복문(O(N^2))을 사용하는 대신,
			// 블록의 기하학적 중심 좌표를 찾아 단일 픽셀만 샘플링하여 연산 속도를 대폭 향상시킴.
			int centerX = std::min(x + (sampleW / 2), (int)pixels.getWidth() - 1);
			int centerY = std::min(y + (sampleH / 2), (int)pixels.getHeight() - 1);

			int brightness = pixels.getColor(centerX, centerY).getBrightness();
			row += brightnessToAscii(brightness);
		}

		// 완성된 문자열 행을 벡터 자료구조에 추가
		asciiFrame.push_back(row);
	}
}

//--------------------------------------------------------------
void ofApp::saveAsciiToTxt() {
	// 목적: 파일 저장 창에 기본적으로 뜰 추천 파일명 생성 (타임스탬프 포함하여 덮어쓰기 방지)
	string defaultName = "ascii_" + ofGetTimestampString() + ".txt";
	
	// 목적: 네이티브 OS의 '다른 이름으로 저장' 창을 호출하여 사용자가 직접 저장 경로를 선택하도록 유도
	ofFileDialogResult result = ofSystemSaveDialog(defaultName, "Save");
	
	// 목적: 사용자가 '저장' 버튼을 눌렀는지 확인 (취소 버튼을 누르지 않았을 경우만 실행)
	if (result.bSuccess) {
		
		// 사용자가 지정한 전체 경로(getPath)를 사용하여 텍스트 파일 오픈 (쓰기 전용 모드)
		ofFile file(result.getPath(), ofFile::WriteOnly);
		
		// 현재 벡터 배열(asciiFrame)에 저장된 모든 행 데이터를 텍스트 문서에 기록
		for (int i = 0; i < asciiFrame.size(); i++) {
			file << asciiFrame[i] << endl;
		}
		
		// 성공적으로 저장되었음을 콘솔에 출력
		ofLogNotice() << "Successfully saved frame to: " << result.getPath();
	} else {
		// 사용자가 창을 그냥 닫거나 취소한 경우
		ofLogNotice() << "Save process cancelled";
	}
}

//--------------------------------------------------------------
void ofApp::displayInfo() {
	if (isDisplayInfo) {
		ofSetColor(0);
		ofDrawRectangle(10, 10, 350, 210); // 정보 표시를 위한 반투명 배경 상자 렌더링
		
		ofSetColor(255);
		
		// 목적: 화면에 표시될 HUD 텍스트를 구성합니다.
		// 한글 폰트 미설정 시 발생하는 텍스트 깨짐 현상을 방지하기 위해 출력 문자열은 영문으로 유지합니다.
		
		string infoText = "--- CURRENT SETTINGS ---\n"; // 의미: "--- 현재 설정 ---"
		
		// 현재 적용된 마우스 X, Y 좌표 기반의 감마 및 블랙 레벨 수치 연결
		infoText += "Gamma (Mouse X): " + ofToString(currentGamma, 2) + "\n"; // 의미: "감마 (마우스 X): "
		infoText += "Black Level (Mouse Y): " + ofToString(blackLevel) + "\n"; // 의미: "블랙 레벨 (마우스 Y): "
		infoText += "Palette: " + paletteName + "\n\n"; // 현재 적용된 레트로 컬러 팔레트 이름 표시
		
		// 단축키 조작 가이드 섹션
		infoText += "--- CONTROLS ---\n"; // 의미: "--- 조작 방법 ---"
		infoText += "[B] : Hold + Move Mouse to calibrate.\n"; // 의미: "[M] : 길게 눌러 명암 보정"
		infoText += "[R] : Reset Calibration to Default\n"; // 의미: "[R] : 보정 설정 기본값으로 초기화"
		infoText += "[C] : Cycle Color Palette\n"; // 의미: "[C] : 컬러 팔레트 변경"
		infoText += "[L] : Load Static Image (.jpg/.png)\n"; // 의미: "[L] : 이미지 불러오기 (.jpg/.png)"
		infoText += "[W] : Switch to Live Webcam\n"; // 의미: "[W] : 실시간 웹캠으로 전환"
		infoText += "[S] : Save Frame to .txt File\n"; // 의미: "[S] : 현재 프레임을 .txt 파일로 저장"
		infoText += "[I] : Hide this Info Menu\n"; // 의미: "[I] : 정보 메뉴 숨기기"
		infoText += "[M] : Toggle Matrix Rain Particle System\n"; // 의미: "[SPACE] : 매트릭스 비 파티클 시스템 켜기/끄기"
		
		ofDrawBitmapString(infoText, 20, 30);
	} else {
		ofSetColor(0);
		ofDrawRectangle(10, 10, 220, 30); // 최소화된 가이드 상자 렌더링
		
		ofSetColor(255);
		// 메뉴가 숨겨졌을 때 표시되는 최소화 안내 문구
		ofDrawBitmapString("[I] : Show Controls Menu", 20, 30); // 의미: "[I] : 조작 메뉴 보기"
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 'b' || key == 'B') isCalibrating = true;
	
	if (key == 'r' || key == 'R') { // 캘리브레이션 변수 초기화 로직
		currentGamma = 0.55f;
		blackLevel = 40;
		whiteLevel = 220;
	}
	
	if (key == 's' || key == 'S') saveAsciiToTxt();
	
	if (key == 'w' || key == 'W') useWebcam = true;
	
	if (key == 'l' || key == 'L') loadImage();
	
	if (key == 'c' || key == 'C') colorMode = (colorMode + 1) % 5;
	
	if (key == 'i' || key == 'I') isDisplayInfo = !isDisplayInfo;
	
	if (key == 'm' || key == 'M') isRainActive = !isRainActive;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	if (key == 'b' || key == 'B') isCalibrating = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	// 목적: 'm' 키가 눌려있을 때 윈도우 창 내의 마우스 좌표를 감마 및 블랙 레벨 값으로 동적 매핑
	if (isCalibrating) {
		currentGamma = ofMap(x, 0, ofGetWidth(), 0.1f, 2.0f, true);
		blackLevel = ofMap(y, 0, ofGetHeight(), 0, 100, true);
	}
}

//--------------------------------------------------------------
void ofApp::matrixRain(){
	// 목적: [핵심 알고리즘] 파티클 운동학(Kinematics) 및 공간 충돌 감지(Spatial Collision Detection)
	for (auto& drop : rainDrops) {
		// 1. 공간 픽셀 샘플링 (O(1) 시간 복잡도)
		// 파티클의 현재 물리적 좌표를 웹캠 픽셀 좌표로 변환하여 해당 위치의 밝기 값을 참조합니다.
		int bright = 0;
		if (pixels.isAllocated()) {
			// 화면 해상도와 픽셀 해상도 사이의 비율 매핑
			int px = ofMap(drop.x, 0, ofGetWidth(), 0, pixels.getWidth(), true);
			int py = ofMap(drop.y, 0, ofGetHeight(), 0, pixels.getHeight(), true);
			bright = pixels.getColor(px, py).getBrightness();
		}

		// 2. 마찰력 연산 알고리즘
		// 밝은 픽셀(사용자의 얼굴 등)에 닿으면 마찰력이 증가하여 속도가 둔화되는 물리 효과 구현
		float friction = ofMap(bright, 0, 255, 1.0f, 0.1f);
		drop.y += drop.speed * friction;

		// 3. 화면 밖으로 나간 파티클 재활용 (메모리 최적화)
		if (drop.y > ofGetHeight() + (drop.length * 14)) {
			drop.y = ofRandom(-200, -50);
			drop.x = ofRandom(0, ofGetWidth());
			drop.speed = ofRandom(4.0f, 12.0f);
		}
		
		// 4. 맨 앞단의 텍스트를 사이버펑크 스타일로 무작위 변경
		if (ofGetFrameNum() % 4 == 0) {
			drop.headChar = (char)ofRandom(33, 126);
		}
	}
}

//--------------------------------------------------------------
void ofApp::drawMatrixRain() {
	// 목적: 파티클 시스템이 활성화되었을 때, 계산된 물리 좌표에 맞춰 매트릭스 비 효과를 화면에 그립니다.
	if (isRainActive) {
		for (auto& drop : rainDrops) {
			for (int i = 0; i < drop.length; i++) {
				int drawY = drop.y - (i * 14); // 폰트 높이(14px)만큼 위로 꼬리 생성
				
				// 화면 내에 있는 텍스트만 그리기 (렌더링 연산 최적화)
				if (drawY > 0 && drawY < ofGetHeight()) {
					if (i == 0) {
						// 머리 부분: 떨어지는 위치를 강조하기 위한 강렬한 흰색
						ofSetColor(255, 255, 255, 255);
						ofDrawBitmapString(string(1, drop.headChar), drop.x, drawY);
					} else {
						// 꼬리 부분: 위로 갈수록 투명해지는 그라데이션 페이드 아웃 및 글리치 효과
						float alpha = ofMap(i, 1, drop.length, 255, 0);
						ofSetColor(terminalColor.r, terminalColor.g, terminalColor.b, alpha);
						char tailChar = (char)ofRandom(33, 126);
						ofDrawBitmapString(string(1, tailChar), drop.x, drawY);
					}
				}
			}
		}
	}
}

void ofApp::loadImage(){
	ofFileDialogResult result = ofSystemLoadDialog("Select an image to process");
			
	if (result.bSuccess) {
		ofImage tempImg;
		tempImg.load(result.getPath());
		
		// 목표 캔버스 크기
		int targetW = 960;
		int targetH = 720;
		
		float imgW = tempImg.getWidth();
		float imgH = tempImg.getHeight();
		
		// 크롭(Crop) 방지: std::min을 사용하여 이미지가 화면에 온전히 들어가도록 스케일링 (Fit)
		float scaleFactor = std::min(targetW / imgW, targetH / imgH);
		
		int newW = std::round(imgW * scaleFactor);
		int newH = std::round(imgH * scaleFactor);
		
		// 1. 원본 비율을 유지하며 축소/확대
		tempImg.resize(newW, newH);
		
		// 2. 여백(Pillarbox/Letterbox)을 검은색으로 채울 960x720 빈 픽셀 캔버스 생성
		ofPixels paddedPixels;
		// 원본 이미지의 색상 포맷(RGB/RGBA)과 동일하게 메모리 할당
		paddedPixels.allocate(targetW, targetH, tempImg.getImageType());
		paddedPixels.setColor(ofColor::black); // 빈 공간을 완전한 검은색으로 초기화
		
		// 3. 스케일링된 이미지를 검은 캔버스의 기하학적 정중앙에 붙여넣기
		int offsetX = (targetW - newW) / 2;
		int offsetY = (targetH - newH) / 2;
		tempImg.getPixels().pasteInto(paddedPixels, offsetX, offsetY);
		
		// 4. 최종 결과물을 메인 loadedImage 객체에 적용
		loadedImage.setFromPixels(paddedPixels);
		
		useWebcam = false;
	}
}
