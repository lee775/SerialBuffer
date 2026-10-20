# SerialBuffer

가변 길이 패킷을 다루기 위한 직렬화 버퍼(`Packet` 클래스)다. `<<` / `>>` 연산자로 기본 자료형을 바이트 버퍼에 차례로 쓰고 같은 순서로 읽는다.

## 소개

- 길이가 일정하지 않은 데이터를 하나의 버퍼에 순서대로 쌓고(직렬화), 쌓은 순서대로 꺼내는(역직렬화) 버퍼 클래스다.
- 정수형을 비트 시프트로 1바이트씩 기록하는 방식과 `memcpy`로 통째로 복사하는 방식을 모두 구현하고, 컴파일 타임 매크로로 골라 성능을 비교할 수 있게 했다.

## 주요 기능

- 자료형별 연산자 오버로딩
  - 쓰기 `<<`: `char`, `unsigned char`, `short`, `unsigned short`, `int`, `DWORD`, `long`, `float`, `__int64`, `double`
  - 읽기 `>>`: `char`, `BYTE`, `short`, `WORD`, `int`, `DWORD`, `float`, `__int64`, `double`
- 연쇄 호출: `p << a << c << z;` / `p >> b >> d >> x;`
- 임의 크기 데이터 입출력: `PutData()` / `GetData()` (구조체 등)
- 남은 데이터 복사: `GetBuffer()`는 지정한 크기까지 남은 데이터를 한 번에 복사한다.
- 외부에서 직접 쓴 데이터 반영: `GetBufferPtr()`로 버퍼 주소를 받아 직접 쓴 뒤 `MoveWritePos()` / `MoveReadPos()`로 위치만 옮긴다.
- 깊은 복사 대입 연산자 `operator=`
- 상태 조회와 초기화: `GetBufferSize()`, `GetDataSize()`, `GetFreeSize()`, `Clear()`

## 핵심 구현

구현은 [`SerialBuffer.h`](SerialBufferTest/SerialBuffer.h), [`SerialBuffer.cpp`](SerialBufferTest/SerialBuffer.cpp)에 있다.

### 읽기 / 쓰기 위치 관리

- 쓰기 위치와 읽기 위치를 인덱스(`_writeIdx`, `_readIdx`)와 포인터(`_writePtr`, `_readPtr`) 두 형태로 함께 관리한다.
- 읽기 위치가 쓰기 위치를 따라잡으면(`_readIdx == _writeIdx`) 자동으로 `Clear()`를 호출해 다음 데이터를 버퍼 맨 앞부터 다시 쓴다.
- 남은 데이터보다 많이 읽으려 하면 `"Data not available."` 예외를 던진다. 호출하는 쪽에서 데이터 부족을 예외로 처리할 수 있다.

### 버퍼 확장: `Resize()`

- 기본 크기 `BUFFER_DEFAULT`(1000바이트)로 시작한다.
- 쓰기 전에 `GetFreeSize()`로 남은 공간을 확인하고, 부족하면 1000바이트 더 큰 버퍼를 새로 할당해 기존 내용을 복사한다. 읽기 / 쓰기 포인터는 새 버퍼 기준으로 다시 계산한다.
- 확장할 때 `LogPrint()`가 버퍼 전체를 16바이트 단위 16진수 덤프로 `Log/log.txt`에 남긴다. 실행 경로에 `Log` 폴더가 있어야 한다.

### 비트 시프트 방식과 `memcpy` 방식: `USE_MEMCPY`

- `USE_MEMCPY`를 정의하지 않은 기본 상태에서는 정수형을 `value >> 8`, `value >> 16` … 순서로 하위 바이트부터 1바이트씩 기록하고, 읽을 때 `<<`로 다시 조립한다.
- `USE_MEMCPY`를 정의하면 정수형도 `memcpy_s`로 통째로 복사한다.
- `float`, `double`은 비트 시프트를 쓸 수 없으므로 설정과 관계없이 항상 `memcpy_s`로 처리한다.

### 테스트와 측정

- [`SerialBufferTest.cpp`](SerialBufferTest/SerialBufferTest.cpp)의 `main()`은 `int`, `double`, `char` 연쇄 입출력과 구조체 `PutData()` / `GetData()` 왕복 결과를 출력한다.
- 같은 파일의 `Test1()`(`float`, 즉 `memcpy` 경로)과 `Test2()`(`int`, 즉 시프트 경로)는 각각 `<<` / `>>`를 1,000번 반복하는 구간을 프로파일러 매크로 `PRO_BEGIN` / `PRO_END`로 감싸 측정한다. 현재 `main()`에서는 호출 부분이 주석 처리되어 있다.
- [`profile.txt`](SerialBufferTest/profile.txt)에는 이 측정의 기록이 남아 있다. 함수를 각각 1,000번 호출했을 때 1회(`<<` / `>>` 1,000쌍) 평균이 `Shift` 28.26㎲, `Memcpy` 40.45㎲다. 두 항목은 측정한 자료형이 다르고(`int` / `float`), 파일의 나머지 행은 이 비교와 관계없는 기록이다.

## 기술 스택

- 언어: C++
- 플랫폼: Windows 콘솔 (`Windows.h`의 `BYTE`, `WORD`, `DWORD` 사용)
- 개발 환경: Visual Studio 2022 (플랫폼 도구 집합 v143)

## 프로젝트 구조

```
SerialBufferTest.sln
SerialBufferTest/
├── SerialBuffer.h        # Packet 클래스 선언
├── SerialBuffer.cpp      # 연산자 오버로딩, 버퍼 확장, 로그 덤프
├── SerialBufferTest.cpp  # 사용 예제와 성능 측정 코드
└── profile.txt           # 측정 결과 기록
```

## 빌드 및 실행

> 프로젝트 파일과 `SerialBufferTest.cpp`가 참조하는 `Profiling.h` / `Profiling.cpp`(프로파일러 소스)는 이 저장소에 들어 있지 않아, 그대로 빌드하면 파일을 찾지 못해 실패한다(C1083).

1. 프로파일러 파일을 직접 추가하거나, `SerialBufferTest` 폴더에 아래처럼 빈 매크로만 둔 파일을 만든다.

   ```cpp
   // Profiling.h
   #pragma once
   #define PRO_BEGIN(tag)
   #define PRO_END(tag)
   ```

   ```cpp
   // Profiling.cpp
   #include "Profiling.h"
   ```

2. Visual Studio 2022에서 `SerialBufferTest.sln`을 열고 `x64` 구성으로 빌드한 뒤 실행한다.
   - 명령줄 빌드: `msbuild SerialBufferTest.sln /p:Configuration=Debug /p:Platform=x64`
3. 위 빈 매크로 파일로 빌드했을 때의 출력은 다음과 같다.

   ```
   5, 4.140000, y
   5, 4.140000, y
   5 , 4, 3.140000, A
   ```

## 개발 기간

- 2025-01-02 (저장소 등록, 단일 커밋)
- 1인 개인 프로젝트
