#pragma once

#include "ofMain.h"
#include <vector>
#include <string>

// --- 새로운 자료구조: 매트릭스 비 파티클 (Particle System) ---
// 목적: 커스텀 구조체(Struct)를 선언하여 위치, 속도, 꼬리 길이 등 물리적 상태를 독립적으로 관리
struct MatrixDrop {
	float x;           // X 좌표
	float y;           // Y 좌표 (실수형으로 미세한 물리 속도 제어)
	float speed;       // 중력에 의해 떨어지는 기본 속도
	int length;        // 비 꼬리의 길이
	char headChar;     // 맨 앞단에 떨어지는 무작위 문자
};

class ofApp : public ofBaseApp {

public:
	// --- 기본 openFrameworks 함수 ---
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);

	// --- 커스텀 프로젝트 함수 ---
	void generateAsciiFrame();                 // 목적: 픽셀을 샘플링하여 ASCII 그리드를 생성하는 핵심 알고리즘
	char brightnessToAscii(int brightness);    // 목적: 특정 밝기 값(정수)을 지정된 ASCII 문자로 매핑
	void saveAsciiToTxt();                     // 목적: 현재 asciiFrame 벡터 데이터를 로컬 .txt 파일로 저장
	void displayInfo();                        // 목적: 화면에 HUD(Heads-Up Display) 및 조작 가이드 메뉴 렌더링
	void matrixRain();     // 목적: 픽셀 밝기 기반의 마찰력이 적용된 매트릭스 파티클 물리 상태 업데이트
	void drawMatrixRain(); // 목적: 업데이트된 물리 좌표를 기반으로 매트릭스 파티클 시스템 렌더링
	void loadImage();      // 목적: 시스템 파일 창을 열어 이미지를 불러오고 캔버스 비율에 맞춰 왜곡 없이 크롭 및 리사이징

	// --- 핵심 미디어 변수 ---
	ofVideoGrabber cam;       // 목적: 하드웨어 웹캠으로부터 실시간 비디오 피드 캡처
	ofImage loadedImage;      // 목적: 사용자의 컴퓨터에서 로드된 정적 이미지 파일 저장
	ofPixels pixels;          // 목적: 웹캠 또는 로드된 이미지의 픽셀 데이터를 공통으로 저장 및 관리하는 통합 객체

	// --- 자료구조 ---
	// 목적: 생성된 텍스트를 행(row) 단위로 저장하는 동적 배열.
	// 매 프레임마다 빠르게 초기화(clear)하고 새로운 문자열 데이터를 동적으로 추가(push_back)하기 위해 std::vector 사용.
	std::vector<std::string> asciiFrame;

	// --- 프로그램 상태 플래그 변수 ---
	bool useWebcam;           // 목적: 실시간 카메라 피드 처리 모드(true)와 정적 이미지 처리 모드(false) 상태 추적
	bool isCalibrating;       // 목적: 'm' 키 입력에 따른 마우스 캘리브레이션 툴의 활성화/비활성화 상태 잠금 제어
	bool isDisplayInfo;       // 목적: HUD 컨트롤 메뉴의 화면 표시 여부 토글 상태 추적

	// --- 캘리브레이션 및 시각적 요소 변수 ---
	float currentGamma;       // 목적: 그림자와 중간 톤의 대비를 조정하기 위한 비선형 감마 보정 곡선 값 저장
	int blackLevel;           // 목적: 빈 공간(공백)으로 매핑될 절대적인 어둠의 하한 임계값 정의
	int whiteLevel;           // 목적: 가장 밀도가 높은 문자('@')로 매핑될 밝기의 상한 임계값 정의
	
	int colorMode;            // 목적: 현재 선택된 레트로 컬러 팔레트의 인덱스(0~4) 저장
	ofColor terminalColor;    // 목적: ASCII 텍스트를 화면에 그릴 때 사용할 현재 RGB 색상 값 저장
	std::string paletteName;  // 목적: HUD에 표시할 현재 활성화된 컬러 팔레트의 이름(문자열) 저장
	
	// --- 파티클 시스템 자료구조 및 플래그 ---
	std::vector<MatrixDrop> rainDrops; // 목적: 다수의 파티클 객체를 동적으로 관리하기 위한 컨테이너
	bool isRainActive;                 // 목적: 스페이스바 입력에 따른 파티클 시스템 활성화/비활성화 상태 추적
};
