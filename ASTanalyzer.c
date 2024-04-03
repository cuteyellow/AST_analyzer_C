#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

#define MAX_PARAMS 10 // 하나의 함수가 가질 수 있는 최대 매개변수 개수
#define MAX_FUNCTIONS 100 // 추출할 수 있는 최대 함수 개수

typedef struct {
	char type[100]; // 파라미터 타입
	char name[100]; // 파라미터 이름
} ParamInfo;

typedef struct {
	char name[100]; // 함수 이름
	char returnType[100]; // 리턴 타입
	ParamInfo params[MAX_PARAMS]; // 파라미터 정보
	int paramCount; // 파라미터 개수
	int ifCount; // if 조건의 개수
	int bBody; //함수 body부분 구현 여부
} FunctionInfo;

FunctionInfo functions[MAX_FUNCTIONS]; // 함수 정보를 저장할 전역 배열
int iFunctionCount = 0; // 저장된 함수의 개수

int iFunctionNum = 0;
void iterJsonObj(cJSON* json);
void analyzeJsonObj(cJSON* object);
void analyzeJsonObj_Function(cJSON* nodeTypeFuncdef);
void analyzeJsonObj_Function_Param(cJSON* param, int k);
void analyzeJsonObj_Decl(cJSON* nodeTypeDecl);
int countIfNodetypes(cJSON* item);
void writeFunctionsToJson(const char* filename);

//JSON 파싱
cJSON* parseJsonString(const char* jsonString) {
	cJSON* json = cJSON_Parse(jsonString);
	if (json == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		return;
	}
	else {
		return json;
	}
}

void iterJsonObj(cJSON* json) {
	cJSON* functionAndVariables = cJSON_GetObjectItemCaseSensitive(json, "ext");
	cJSON* object = NULL;
	if (cJSON_IsArray(functionAndVariables)) {
		cJSON_ArrayForEach(object, functionAndVariables) {
			analyzeJsonObj(object);
		}//ext 배열의 모든 원소 순회
	}
	else {
		cJSON_Delete(functionAndVariables);
		return;
	}
}


//함수 선언 변수 선언 구별
void analyzeJsonObj(cJSON* object) {
	if (object != NULL) {
		cJSON* _nodetype = cJSON_GetObjectItemCaseSensitive(object, "_nodetype");

		char* nodetype = _nodetype->valuestring;
		if (strcmp(nodetype, "Decl") == 0) {
			analyzeJsonObj_Decl(object);
		}
		else {
			analyzeJsonObj_Function(object);
		}

	}
}

//함수 처리
void analyzeJsonObj_Function(cJSON* nodeTypeFuncdef) {
	functions[iFunctionCount].paramCount = 0;
	int iIfnum = 0;
	cJSON* decl = cJSON_GetObjectItemCaseSensitive(nodeTypeFuncdef, "decl");
	cJSON* body = cJSON_GetObjectItemCaseSensitive(nodeTypeFuncdef, "body");

	cJSON* name = cJSON_GetObjectItemCaseSensitive(decl, "name");
	cJSON* type = cJSON_GetObjectItemCaseSensitive(decl, "type");
	printf("function name: %s\n", name->valuestring);
	strcpy(functions[iFunctionCount].name, name->valuestring);
	functions[iFunctionCount].bBody = 1;

	if (cJSON_IsObject(type)) {
		cJSON* typeOfType = cJSON_GetObjectItemCaseSensitive(type, "type");

		cJSON* typeOfTypeOfType = cJSON_GetObjectItemCaseSensitive(typeOfType, "type");
		cJSON* identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(typeOfTypeOfType, "names"), 0);

		//반환타입
		if (identifier == NULL) {
			printf("return type: void\n");
			strcpy(functions[iFunctionCount].returnType, "void");
		}
		else {
			printf("return type: %s\n", identifier->valuestring);
			strcpy(functions[iFunctionCount].returnType, identifier->valuestring);
		}

		//매개변수
		cJSON* args = cJSON_GetObjectItemCaseSensitive(type, "args");
		cJSON* params = cJSON_GetObjectItemCaseSensitive(args, "params");
		cJSON* param;
		if (params != NULL) {
			int i = cJSON_GetArraySize(params);
			for (int k = 0; k < i; k++) {
				analyzeJsonObj_Function_Param(cJSON_GetArrayItem(params, k),k);
			}
		}
		else {
			printf("param: none\n");
		}
	}
	//if 개수
	if (cJSON_IsObject(body)) {
		iIfnum = countIfNodetypes(body);
		functions[iFunctionCount].ifCount = iIfnum;
	}
	printf("if 개수: %d", iIfnum);
	printf("\n");
	iFunctionCount++;
}

//매개변수 추적
void analyzeJsonObj_Function_Param(cJSON* param,int k) {
	cJSON* type = cJSON_GetObjectItemCaseSensitive(param, "type");

	cJSON* type2 = cJSON_GetObjectItemCaseSensitive(type, "type");
	cJSON* name = cJSON_GetObjectItemCaseSensitive(param, "name");

	cJSON* type3 = cJSON_GetObjectItemCaseSensitive(type2, "type");
	cJSON* identifier;
	if (type3 != NULL) {
		identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type3, "names"), 0);
		name = cJSON_GetObjectItemCaseSensitive(type2, "declname");
		strcpy(functions[iFunctionCount].params[k].type, identifier->valuestring);
		strcpy(functions[iFunctionCount].params[k].name, name->valuestring);
	}
	else {
		identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type2, "names"), 0);
		strcpy(functions[iFunctionCount].params[k].type, identifier->valuestring);
	}
	functions[iFunctionCount].paramCount++;
	printf("param: %s %s\n", identifier->valuestring, name->valuestring);

}

//재귀적 if 탐색 및 개수 반환
int countIfNodetypes(cJSON* item) {
	int count = 0; // 현재 노드에 대한 카운트

	if (item == NULL) {
		return 0; // NULL 노드일 경우, 카운트하지 않음
	}

	// 현재 노드가 '_nodetype'이 "If"인지 확인
	cJSON* nodeType = cJSON_GetObjectItemCaseSensitive(item, "_nodetype");
	if (nodeType && cJSON_IsString(nodeType) && strcmp(nodeType->valuestring, "If") == 0) {
		count = 1; // 현재 노드가 조건을 만족하면 카운트 증가
	}

	// 현재 노드가 객체이거나 배열인 경우, 그 내부를 재귀적으로 탐색
	cJSON* child = NULL;
	cJSON_ArrayForEach(child, item) {
		count += countIfNodetypes(child); // 자식 노드 탐색 결과를 카운트에 추가
	}

	return count; // 최종 카운트 값 반환
}

//변수 선언 및 함수 선언
void analyzeJsonObj_Decl(cJSON* nodeTypeDecl) {
	functions[iFunctionCount].paramCount = 0;
	cJSON* name = cJSON_GetObjectItemCaseSensitive(nodeTypeDecl, "name");
	
	cJSON* type = cJSON_GetObjectItemCaseSensitive(nodeTypeDecl, "type");
	cJSON* type2 = cJSON_GetObjectItemCaseSensitive(type, "type");
	cJSON* nodetype= cJSON_GetObjectItemCaseSensitive(type, "_nodetype");
	
	//body 없는 함수 선언
	if (strcmp(nodetype->valuestring, "FuncDecl") == 0) {
		printf("(함수 미구현) %s\n", name->valuestring);
		cJSON* type3 = cJSON_GetObjectItemCaseSensitive(type2, "type");

		
		cJSON* identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type3, "names"), 0);

		if (identifier == NULL) {
			printf("return type: void\n");
			strcpy(functions[iFunctionCount].returnType, "void");
		}
		else {
			printf("return type: %s\n", identifier->valuestring);
			strcpy(functions[iFunctionCount].returnType, identifier->valuestring);
		}

		cJSON* args = cJSON_GetObjectItemCaseSensitive(type, "args");
		cJSON* params = cJSON_GetObjectItemCaseSensitive(args, "params");
		cJSON* param;
		if (params != NULL) {
			int i = cJSON_GetArraySize(params);
			for (int k = 0; k < i; k++) {
				analyzeJsonObj_Function_Param(cJSON_GetArrayItem(params, k),k);
			}
		}
		else {
			printf("param: none\n");
		}
		iFunctionCount++;
	}
	
}

//새로운 json 작성 함수
void writeFunctionsToJson(const char* filename) {
	cJSON* root = cJSON_CreateArray();
	for (int i = 0; i < iFunctionCount; i++) {
		cJSON* functionJson = cJSON_CreateObject();
		cJSON_AddStringToObject(functionJson, "name", functions[i].name);
		cJSON_AddStringToObject(functionJson, "returnType", functions[i].returnType);

		cJSON* paramsArray = cJSON_CreateArray();
		for (int j = 0; j < functions[i].paramCount; j++) {
			cJSON* param = cJSON_CreateObject();
			cJSON_AddStringToObject(param, "type", functions[i].params[j].type);
			cJSON_AddStringToObject(param, "name", functions[i].params[j].name);
			cJSON_AddItemToArray(paramsArray, param);
		}
		cJSON_AddItemToObject(functionJson, "params", paramsArray);
		cJSON_AddNumberToObject(functionJson, "ifCount", functions[i].ifCount);

		cJSON_AddItemToArray(root, functionJson);
	}

	char* jsonString = cJSON_Print(root);
	FILE* file = fopen(filename, "w");
	if (file != NULL) {
		fprintf(file, "%s", jsonString);
		fclose(file);
	}
	free(jsonString);
	cJSON_Delete(root);
}



int main() {
	
	const char* filePath = "target.json";
	FILE* file = fopen(filePath, "r");
	if (file == NULL) {
		perror("Unable to open the file.");
		return 1;
	}

	// 파일 크기 구하기
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* sTemp = NULL;
	char line[1024];

	char* jsonString = malloc(length + 1); // +1 null 문자 추가 위해 +1
	if (jsonString == NULL) {
		perror("Memory allocation failed");
		fclose(file);
		return 1;
	}
	jsonString[0] = '\0'; // 널 문자로 초기화



	// 파일 읽기
	while (fgets(line, sizeof(line), file) != NULL) {
		
		strncat(jsonString, line, length - strlen(jsonString));
	}


	// JSON 문자열 파싱
	cJSON* json = parseJsonString(jsonString);
	iterJsonObj(json);

	// 정리
	fclose(file);
	free(jsonString);
	printf("함수 총 %d개\n", iFunctionCount);

	writeFunctionsToJson("functions_info.json");

	return 0;
}

