# 실시간 ASCII 변환 및 매트릭스 파티클 시스템

## 프로젝트 소개

이 프로그램은 C++이랑 openFrameworks로 만들었습니다. 웹캠 화면을 가져와서 실시간으로 ASCII 아트로 바꿔줍니다. 그리고 카메라 밝기에 반응하는 '매트릭스 비' 효과도 있습니다.

## 필요한 환경

- openFrameworks v0.10.4
- 웹캠 (노트북 기본 카메라도 가능)
- IDE: Mac은 Xcode, Windows는 Visual Studio

## 제출 파일

프로젝트 전체 폴더가 아니라 코드 파일 딱 3개만 제출합니다.

- main.cpp: 창 크기랑 프로그램 시작 세팅을 합니다.
- ofApp.h: 프로젝트에 필요한 변수와 클래스, 그리고 파티클에 쓸 struct MatrixDrop이 있습니다.
- ofApp.cpp: 밝기 계산이나 화면을 그리는 메인 코드가 전부 들어있습니다.

## 실행하는 방법 (중요)

코드 파일만 제출했기 때문에, 테스트하려면 컴퓨터에서 새 프로젝트를 만들어야 합니다.

- openFrameworks 폴더에서 Project Generator를 실행합니다.
- 프로젝트 이름을 적고 새로운 프로젝트를 하나 만듭니다 (예: Generate 버튼 클릭).
- 새로 만들어진 프로젝트 폴더 안에서 src 폴더로 들어갑니다. (경로: apps/myApps/새프로젝트/src)
- 거기에 원래 있던 기본 파일들을 다 지웁니다. 그리고 제가 제출한 3개 파일(main.cpp, ofApp.cpp, ofApp.h)을 넣습니다.
- Mac이면 project.pbxproj를 Xcode로 열고, Windows면 .sln을 Visual Studio로 엽니다.
- IDE에서 실행(Run) 버튼을 누릅니다.

(참고 사항)
프로그램을 켰는데 화면이 까맣게 나오면 에러가 아닙니다. Mac이나 Windows 설정 메뉴에서 카메라 접근 권한을 허용해야 화면이 보입니다.
