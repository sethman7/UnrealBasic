// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "StudentManager.h" //일반 C++ 

void CheckUObjectIsValid(const UObject* InObject, const FString& InTag)
{
	if (InObject->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 유효한 언리얼 오브젝트"), *InTag);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 유효하지 않은 언리얼 오브젝트"), *InTag);
	}
}

void CheckUObjectIsNull(const UObject* InObject, const FString& InTag)
{
	if (nullptr == InObject)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 널 포인터 언리얼 오브젝트"), *InTag);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 널 포인터가 아닌 언리얼 오브젝트"), *InTag);
	}
}

void UMyGameInstance::Init() //어플리케이션 초기화 때 호출 
{
	Super::Init();

	NonPropStudent = NewObject<UStudent>(); //프로퍼티 관리를 받지않는 객체 생성
	PropStudent = NewObject<UStudent>(); //프로퍼티 관리를 받는 객체 생성

	NonPropStudents.Add(NewObject<UStudent>());
	PropStudents.Add(NewObject<UStudent>());

	StudentManager = new FStudentManager(NewObject<UStudent>()); //일반 C++ 객체 생성
}

void UMyGameInstance::Shutdown() //어플리케이션 종료될 때 호출 
{
	Super::Shutdown();

/* ------실습 1------ */

	CheckUObjectIsNull(NonPropStudent, TEXT("NonPropStudent"));  // 널 포인터가 아닌 언리얼 오브젝트
	CheckUObjectIsValid(NonPropStudent, TEXT("NonPropStudent")); // 유효하지 않은 언리얼 오브젝트   <-- GUObjectArray에 등록이 되어있지 않기 때문, 댕글링 포인터 문제 발생

	CheckUObjectIsNull(PropStudent, TEXT("PropStudent")); // 널 포인터가 아닌 언리얼 오브젝트
	CheckUObjectIsValid(PropStudent, TEXT("PropStudent")); // 유효한 언리얼 오브젝트

	//TArray도 위와 같이 동일
	CheckUObjectIsNull(NonPropStudents[0], TEXT("NonPropStudents"));
	CheckUObjectIsValid(NonPropStudents[0], TEXT("NonPropStudents"));

	CheckUObjectIsNull(PropStudents[0], TEXT("PropStudents"));
	CheckUObjectIsValid(PropStudents[0], TEXT("PropStudents"));



/* ------실습 2------ */

	const UStudent* StudentInManager = StudentManager->GetStudent();

	delete StudentManager;
	StudentManager = nullptr;


	// 메모리 관리 기능을 받기 위해 FGCObject란 특수한 클래스를 상속받음.
	CheckUObjectIsNull(StudentInManager, TEXT("StudentInManager"));  // 널 포인터가 아닌 언리얼 오브젝트
	CheckUObjectIsValid(StudentInManager, TEXT("StudentInManager")); // 유효한 언리얼 오브젝트

}
