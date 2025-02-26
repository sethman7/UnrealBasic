// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"  //언리얼 오브젝트를 Json 오브젝트로 편리하게 변환해주는 헬퍼 라이브러리


void PrintStudentInfo(const UStudent* InStundet, const FString& InTag)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] 이름 %s 순번 %d"), *InTag, *InStundet->GetName(), InStundet->GetOrder());
}

UMyGameInstance::UMyGameInstance()
{

}

void UMyGameInstance::Init()
{
	Super::Init();

//<1> { 언리얼 오브젝트가 아닌 기본 C++ 구조체(struct FStudentData)를 아카이브에 저장하고 불러오도록 하기. }

	FStudentData RawDataSrc(16, TEXT("이득우"));

	//프로젝트 디렉토리 내부에 임의의 폴더 경로를 얻어오기 위해, TEXT("Saved") 매개변수 전달해서 얻어오는 코드
	const FString SaveDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	UE_LOG(LogTemp, Log, TEXT("저장할 파일 폴더 : %s"), *SaveDir);

	const FString RawDataFileName(TEXT("RawData.bin"));
	FString RawDataAbsolutePath = FPaths::Combine(*SaveDir, *RawDataFileName);
	UE_LOG(LogTemp, Log, TEXT("저장할 파일 전체 경로 : %s"), *RawDataAbsolutePath);

	//파일 절대경로를 보기 좋게 만드는 코드 
	FPaths::MakeStandardFilename(RawDataAbsolutePath);
	UE_LOG(LogTemp, Log, TEXT("변경할 파일 전체 경로 : %s"), *RawDataAbsolutePath);


	{
		//저장하기

		//다음 경로에 RawData.bin 파일 생성 , RawFileWriterAr은 RawData폴더를 가리킴.
		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (nullptr != RawFileWriterAr)
		{
			//RawDataSrc구조체의 정보(데이터)를 RawData.bin 파일에 전송(작성)
			*RawFileWriterAr << RawDataSrc;
			// 마무리 작업
			RawFileWriterAr->Close();
			delete RawFileWriterAr;
			RawFileWriterAr = nullptr;
		}

		//불러오기

		FStudentData RawDataDest;
		//RawData폴더를 읽기 전용으로 가져와 RawFileReaderAr가 가리킴. 
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (nullptr != RawFileReaderAr)
		{
			//RawFileReaderAr의 데이터를 RawDataDest에 읽게 한다.
			*RawFileReaderAr << RawDataDest;
			// 마무리 작업
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;

			UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 : %s  순번 : %d"), *RawDataDest.Name, RawDataDest.Order);
		}


	}








//<2> { 언리얼 오브젝트(Student Class)를 아카이브에 저장하고 불러오도록 하기. }
	

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("이득우"));
	StudentSrc->SetOrder(59);
	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin"));
		FString ObjectDataAbsolutePath = FPaths::Combine(*SaveDir, ObjectDataFileName);
		FPaths::MakeStandardFilename(ObjectDataAbsolutePath);


	
		// 언리얼 오브젝트 ---> 바이트 스트림 변환

		TArray<uint8> BufferArray; //바이트 스트림(버퍼) 생성
		FMemoryWriter MemoryWriterAr(BufferArray); //버퍼와 연동되는 메모리 아카이브 생성.
		StudentSrc->Serialize(MemoryWriterAr); //메모리에 쓸 아카이브 지정.
		/*
		위 작업을 통해 메모리의 BufferArray에는 StudentSrc정보들이 채워짐.이를 파일로 다시 저장할 것임.
		앞서 맨 위 코드처럼 파일을 읽거나 저장할 때, 마무리 작업(Delete , null초기화)으로 번거로웠지만
		이것을 한번에 처리하기 위해 스마트 포인터를 사용할 것임.
		*/
		

		if (TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjectDataAbsolutePath)))
		{
			//파일에 쓰기(저장)
			*FileWriterAr << BufferArray;
			FileWriterAr->Close();
		}

		TArray<uint8> BufferArrayFromFile;
		if (TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjectDataAbsolutePath)))
		{
			//파일에 읽기
			*FileReaderAr << BufferArrayFromFile;
			FileReaderAr->Close();
		}

		// 바이트 스트림 ---> 언리얼 오브젝트 변환

		FMemoryReader MemoryReaderAr(BufferArrayFromFile); //버퍼와 연동되는 메모리 아카이브 생성.
		UStudent* StudentDest = NewObject<UStudent>(); 
		StudentDest->Serialize(MemoryReaderAr); //역질렬화 : 단순화시킨 Array<uint8> 타입의 BufferArrayFromFile 바이트스트림을  오브젝트 그래프(StudentDest)로 변환함.
		PrintStudentInfo(StudentDest, TEXT("ObjectData"));
	}


//Json 직렬화
	{
		const FString JsonDataFileName(TEXT("StudentJsonData.txt"));
		FString JsonDataAbsoultePath = FPaths::Combine(*SaveDir, *JsonDataFileName);
		FPaths::MakeStandardFilename(JsonDataAbsoultePath);

		//쉐어드 레퍼런스 : NULL이 아님을 보장해주는 스마트 포인터 
		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		//언리얼 오브젝트를 Json 오브젝트로 변환 
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);
		
		//Json 오브젝트를 바이트 스트림(문자열)로 변환하기.
		FString JsonOutString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString); //Json값들을 String으로 가져오기 위해 아카이브 생성
		if (FJsonSerializer::Serialize(JsonObjectSrc,JsonWriterAr)) //직렬화를 통해 Json오브젝트에 값들을 바이트 스트림(FString)으로 변환.  Json Text String 제작 
		{
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsoultePath); //다음 JsonDataAbsoultePath 파일에다가 Json값을 쓰기.
		}

		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsoultePath);
		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString); 	//불러들인 String 을 사용해서 Reader아카이브 생성.

		//문자열이 잘못들어가 NULL이 들어갈 가능성이 있기에 포인터 사용.
		//역직렬화를 통해 바이트스트림(FString JsonInString) 정보를 다시 오브젝트 그래프(Json 오브젝트)로 변환한다. 
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest))
		{
			//얻어온 Json오브젝트를 Unreal오브젝트로 변환한다. 
			UStudent* JsonStudentDest = NewObject<UStudent>();
			if(FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				PrintStudentInfo(JsonStudentDest, TEXT("JsonData"));
			}
		}
	}

}

