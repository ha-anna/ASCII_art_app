#include "ofApp.h"

//========================================================================
int main( ){

	// 목적: 다중 모니터 전체화면 등 고급 윈도우 설정을 관리하기 위한 OpenGL 설정 객체 선언
	ofGLWindowSettings settings;
	
	// 목적: 애플리케이션 윈도우의 해상도를 클래식 4:3 비율인 960x720으로 고정
	// (720p 웹캠 피드의 세로 길이에 맞추고 가로를 크롭하여 비율을 유지하기 위함)
	settings.setSize(960, 720);
	
	// 목적: 프로그램 실행 모드를 전체화면(OF_FULLSCREEN)이 아닌 일반 창 모드(OF_WINDOW)로 설정
	settings.windowMode = OF_WINDOW;

	// 목적: 위에서 지정한 설정(settings)을 바탕으로 실제 애플리케이션 윈도우 객체 생성
	auto window = ofCreateWindow(settings);

	// 목적: 생성된 윈도우에서 메인 애플리케이션 클래스(ofApp)의 인스턴스를 실행
	// std::make_shared를 사용하여 스마트 포인터로 객체를 할당함으로써, 프로그램 종료 시 안전한 메모리 해제 보장
	ofRunApp(window, std::make_shared<ofApp>());
	
	// 목적: 사용자가 프로그램을 명시적으로 종료할 때까지 프레임 업데이트 및 렌더링 루프를 무한히 유지
	ofRunMainLoop();
}
