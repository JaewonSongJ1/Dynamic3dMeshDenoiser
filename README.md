# Dynamic 3D Mesh Denoiser

**프로덕션용 4D/동적 메쉬 디노이징 툴킷**

Dynamic 3D Mesh Denoiser는 4D 스캔·모션캡처·시뮬레이션 등에서 생성된 **시간적 메쉬 시퀀스(Alembic .abc)**의 노이즈를 제거하는 고성능 C++/Standalone 도구입니다. **Bilateral(프로덕션 퀄리티)** 및 **Temporal(고속 프리뷰)** 필터를 제공하며, **단독 UI**(Dynamic3DMeshDenoiser.exe)로도 실행할 수 있습니다.

---

## 주요 기능

- **고성능**: C++/OpenMP 병렬화 기반. 대규모 프레임·버텍스 처리에 최적화.
- **2가지 알고리즘**
  - **Bilateral Temporal Filter**: 에지 보존·모션 인지 기반 고품질 디노이징(프로덕션 최종 결과에 적합).
  - **Temporal Smoothing Filter**: 선형/가우시안 가중치 기반의 단순·고속 프리뷰.
- **Standalone 파이프라인**: Alembic(.abc) 직접 입력/출력. 외부 DCC 의존성 최소화.
- **대용량 처리**: 프로덕션 규모 데이터 처리에 대응.
- **UI 제공**: `deploy/Dynamic3DMeshDenoiser.exe` 단일 실행 파일(PyInstaller 빌드).

---

## 폴더 구조(요약)

```
Dynamic3dMeshDenoiser/
├─ build/                     # PyInstaller 등 빌드 중간물
│  └─ ui/...
├─ build_release/             # CMake/VS 솔루션 및 Release 산출물
│  └─ Release/
│     ├─ BilateralMeshDenoiser.exe
│     └─ TemporalMeshDenoiser.exe
├─ deploy/                    # 최종 배포물 (실행 파일 모음)
│  ├─ BilateralMeshDenoiser.exe
│  ├─ TemporalMeshDenoiser.exe
│  ├─ Dynamic3DMeshDenoiser.exe   # ← UI 실행 파일
│  └─ how_to_use.txt
├─ example/
│  └─ noisy_input.abc         # 샘플 입력 (테스트용)
├─ src/
│  ├─ BilateralMeshDenoiser.cpp
│  └─ TemporalMeshDenoiser.cpp
└─ ui/
   └─ dynamic3dmesh_denoiser_ui.py
```

> 루트에는 `build_dev.bat`, `build_release.bat`, `CMakeLists.txt` 등이 존재합니다.

---

## 빌드/설치

### 1) 전제조건
- Windows 10/11 (x64)
- Visual Studio 2022 (C++ Desktop), CMake 3.16+
- (선택) vcpkg 사용 시 `VCPKG_ROOT` 세팅 권장
- Python 3.10+ (UI 빌드용, PyQt5/pyinstaller 필요)

### 2) C++ 엔진 빌드(개발/릴리즈)

- **개발(동적 링크·빠른 빌드)**
  ```bat
  build_dev.bat
  ```
  - 용도: 개발/디버그/알고리즘 테스트

- **릴리즈(배포용)**
  ```bat
  build_release.bat
  ```
  - 산출물: `build_release/Release/BilateralMeshDenoiser.exe`, `build_release/Release/TemporalMeshDenoiser.exe`
  - 일반적으로 위 두 파일을 `deploy/`에 복사하여 배포합니다.

### 3) UI 빌드(Windows, PyInstaller)

UI는 `deploy/`의 엔진 실행 파일을 기준으로 동작합니다. PyInstaller로 **단일 exe**를 배포 폴더에 직접 생성합니다.

```bat
cd ui
pip install pyinstaller PyQt5

pyinstaller ^
  --noconfirm ^
  --onefile ^
  --windowed ^
  --name Dynamic3DMeshDenoiser ^
  --distpath ..\deploy ^
  --workpath ..\\build\\ui ^
  --specpath ..\\build\\ui ^
  --collect-submodules PyQt5 ^
  dynamic3dmesh_denoiser_ui.py
```

- 산출물: `deploy/Dynamic3DMeshDenoiser.exe`
- 실행 파일 탐색 우선순위(내장 로직):  
  `D3MD_BIN` 환경변수 폴더 > (frozen) UI exe가 위치한 폴더(`deploy/`) > (소스 실행 시) `<repo_root>/deploy/`

---

## 실행 방법

### 1) UI(권장)
1. `deploy/Dynamic3DMeshDenoiser.exe` 실행
2. **Input**: 예) `example/noisy_input.abc`
3. **Output**: 예) `example/noisy_output.abc`
4. 알고리즘 선택
   - **Bilateral Mesh Denoiser**(프로덕션)
   - **Temporal Mesh Denoiser**(고속 프리뷰)
5. 필요 시 **Advanced Options** 활성화 후 파라미터 조정
6. **Process Denoising** 실행

> 프레임 범위는 Maya 기준(1-based) 또는 내부 옵션으로 지정할 수 있습니다.

### 2) CLI(엔진 단독)

- **기본 사용**
  ```bat
  BilateralMeshDenoiser.exe input.abc output.abc
  TemporalMeshDenoiser.exe input.abc output.abc
  ```

- **프레임 범위 (Maya 1-based)**
  ```bat
  BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100
  TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100
  ```

- **프레임 범위 (Alembic 0-based)**
  ```bat
  BilateralMeshDenoiser.exe input.abc output.abc --sf 0 --ef 99
  TemporalMeshDenoiser.exe input.abc output.abc --start-frame 0 --end-frame 99
  ```

- **Temporal 전용 옵션**
  ```bat
  TemporalMeshDenoiser.exe input.abc output.abc --window 7
  TemporalMeshDenoiser.exe input.abc output.abc --weight gaussian --sigma 1.5
  ```

- **Bilateral 고급 예시**
  ```bat
  BilateralMeshDenoiser.exe input.abc output.abc ^
    --maya-range 1 100 --window 15 --sigma-temporal 5.0 --sigma-spatial 0.35
  ```

---

## 파라미터 가이드(요지)

- **Temporal**
  - `--window`: 시간 윈도우 크기(자동/수동)
  - `--weight`: `linear` | `gaussian`
  - `--sigma`: 가우시안 표준편차(가중치용)

- **Bilateral**
  - `--window`: 시간 윈도우 크기(권장: 9~15)
  - `--sigma-temporal`: 시간적 민감도
  - `--sigma-spatial`: 공간적(에지 보존) 민감도

세부 권장값과 프리셋(예: subtle/medium/strong)은 UI 프리셋 또는 위의 CLI 예시를 참고하세요.

---

## 예제

- 입력: `example/noisy_input.abc`
- 출력: `example/noisy_output.abc` (사용자 지정)
- UI 또는 CLI로 동일하게 처리 가능. (예제 .abc는 용량·형식에 따라 처리 시간이 달라질 수 있음)

---

## 트러블슈팅

- **UI가 “아무 말 없이 종료”되는 것처럼 보임**
  - PyInstaller `--windowed` 빌드에서는 미처리 예외가 콘솔에 보이지 않을 수 있습니다.
  - 해결:
    - 소스 실행(`python ui\\dynamic3dmesh_denoiser_ui.py`)로 콘솔 로그 확인
    - 또는 `--windowed` 대신 `--console`로 빌드하여 예외 출력 확인
    - UI 코드에 전역 `sys.excepthook` 설치하여 메시지 박스로 예외 표시(권장)
  - 탭 위젯에서 메인윈도우 메서드 호출 시에는 `self.window().execute_command(...)` 형태로 접근하십시오.

- **엔진 실행 파일 탐색 실패**
  - `deploy/`에 `BilateralMeshDenoiser.exe`, `TemporalMeshDenoiser.exe`, `Dynamic3DMeshDenoiser.exe` 존재 확인
  - 외부 경로 사용 시 `D3MD_BIN` 환경변수로 폴더 지정 가능

- **Alembic 버전/포맷 이슈**
  - 0-based/1-based 프레임 처리 옵션(`--sf/--ef`, `--maya-range`)을 적절히 사용
  - 데이터 스펙(프레임 수, fps, topology consistency)에 따라 파라미터 튜닝 필요

---

## 개발자

- 소속: Dexter Studios(https://dexterstudios.com)
- 개발자 : Jaewon Song(Head of R&D, jaewon.song@dexterstudios.com)


---

## 라이선스/크레딧

- 라이선스: MIT (루트 `LICENSE` 파일 추가를 권장합니다)
- 기여: 이슈/PR 환영. (Dexter Studios R&D)

---

## 변경 사항 요약(이번 업데이트)

- **UI 분리/정리**: “Alembic Denoise” 단일 창 UI → `deploy/Dynamic3DMeshDenoiser.exe`
- **경로 규칙 통일**: 실행 파일 탐색을 `deploy/` 기준으로 일원화 (`D3MD_BIN` > deploy > PATH)
- **예제 추가**: `example/noisy_input.abc` 테스트 경로 명시
- **배포 빌드 가이드**: PyInstaller 옵션을 `deploy/` 산출로 정리

_문서 버전: 2025-09-04 (KST)_
