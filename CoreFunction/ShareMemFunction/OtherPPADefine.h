#pragma once

#define		VTM_SHARE_MEM_STRING_PPA	"VTM_PPA_Shared_Memory_KMAC_by_KJH"

#define		IDLE						0
#define		STARTMEAS					1
#define		ENDMEAS						2
#define		FAILMEAS					3
#define		ALIVEMODE					4
#define		ALIVEMODEOK					5
#define		SHOWHIDEMODE				6


typedef struct PPA_Info_Shared
{
	char LayerName[100]; // Layer Name
	CRect LayerROI; // ROI 좌표
	
}PPA_Info_Shared;

typedef struct INPUT_DLL_DATA_Shared // DLL Input데이터
{
	char INPUT_Image_File_Path_White[500]; // White광원 원본 이미지 파일 경로(ex : D:\A2QS2S44RJGA8_A2QS2S44RJGAA010_0_088_20140506234644.jpg)
	char INPUT_Image_File_Path_UV[500]; // UV광원 원본 이미지 파일 경로(ex : D:\A2QS2S44RJGA8_A2QS2S44RJGAA010_0_088_20140506234644.jpg)
	char OUTPUT_Image_Folder_Path[500]; // 결과 폴더 상위 경로(ex : "D:\원장이름\" 에 저장하고 싶으시면 "D:\"만 넣어주세요)
	char Glass_ID[50]; // Glass ID
	char Cell_ID[50]; // Cell ID
	int Cell_Current_No; // 현재 검사 Point -> 원장 내 전체 Point 개수 중 몇번째 인지.(첫번째 시작은 1번-> 2번 -> 3번 이런식으로)
	int Light_Type; // 0 -> White 광원 1 -> UV 광원 2 -> 동시 조사
	float Camera_Angle; // 카메라 틀어짐 각도
	float Pixel_Size_X; // Pixel X 사이즈
	float Pixel_Size_Y; // pixel Y 사이즈
	char Input_Char[50]; // "SAMSUNG"이라고 넣어주세요
	
	int Addition_Data[300]; // 향후 추가적인 데이터를 받기 위해 쓰일 변수입니다. 
	
	//////////////////////////////////////////////////////////////////////////
	///////////////////////// 레시피 변수 ///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	// char Recipe_Folder_Path[500]; // 레시피 파일 경로였지만 레시피 변수를 받는걸로 바꿈
	int Model_Type; // A / B / C(신규) // A : 0 / B : 1 /가로  C : 2(신규) /오른쪽, 왼쪽
	//A3 TOP

	// 1~4 -> TEG ROI
	// 5~9 -> Deposition ROI (리스광 이미지에서 상단에 위치한 영역)
	// 10 -> Wide TEG ROI
	PPA_Info_Shared Recipe_Data[11];
	
	int TEG_Width[6]; // 이미지상 TEG 가로축 길이(um단위)
	int TEG_Height[6]; // 이미지상 TEG 세로축 길이(um단위)
	float m_fXPitch[5]; // TEG간 가로축 거리 -- 0으로 넣으셔도 무방
	float m_fYPitch[5]; // TEG간 세로축 거리 -- 0으로 넣으셔도 무방

	int Direcrion;	//Top : 0 / Lift : 1 / Right : 2 180도 : 3
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
}INPUT_DLL_DATA_Shared;

typedef struct OUTPUT_DLL_DATA_Shared // DLL Output DATA
{
	char Dll_Version[50];
	float DEPO_X[5]; // Deposition X크기, 1~5(왼쪽에서 오른쪽 순서)
	float DEPO_Y[5]; // Deposition Y크기, 1~5(왼쪽에서 오른쪽 순서)
	float Glass_X[6]; // TEG X크기, 1~5(왼쪽에서 오른쪽 순서) ,6(Wide TEG)
	float Glass_Y[6]; // TEG Y크기, 1~5(왼쪽에서 오른쪽 순서) ,6(Wide TEG)
	float PPA_X[5]; // PPA X, 1~5(왼쪽에서 오른쪽 순서)
	float PPA_Y[5]; // PPA Y, 1~5(왼쪽에서 오른쪽 순서)
	
	CRect TEG_Result[6]; // TEG 검출결과 좌표
	CRect DEPO_ROI[5]; // Deposition ROI 좌표
	CRect DEPO_Result[5]; // Deposition 검출결과 좌표
	
	char OUTPUT_Image_File_Path[500]; // 검사 결과 파일이름
}OUTPUT_DLL_DATA_Shared;


typedef struct 
{
	int						nCommand[2];
	INPUT_DLL_DATA_Shared	inputPPA[2];
	OUTPUT_DLL_DATA_Shared	outputPPA[2];
}VTM_PPA, *PVTM_PPA;