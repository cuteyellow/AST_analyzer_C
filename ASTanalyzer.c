#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

int iFunctionNum = 0;
void iterJsonObj(cJSON* json);
void analyzeJsonObj(cJSON* object);
void analyzeJsonObj_Function(cJSON* nodeTypeFuncdef);
void analyzeJsonObj_Function_Param(cJSON* param);
void analyzeJsonObj_Decl(cJSON* nodeTypeDecl);
int countIfNodetypes(cJSON* item);

//JSON �Ľ�
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
		}//ext �迭�� ��� ���� ��ȸ
	}
	else {
		cJSON_Delete(functionAndVariables);
		return;
	}
}


//�Լ� ���� ���� ���� ����
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

//�Լ� ó��
void analyzeJsonObj_Function(cJSON* nodeTypeFuncdef) {
	int iIfnum = 0;
	cJSON* decl = cJSON_GetObjectItemCaseSensitive(nodeTypeFuncdef, "decl");
	cJSON* body = cJSON_GetObjectItemCaseSensitive(nodeTypeFuncdef, "body");

	cJSON* name = cJSON_GetObjectItemCaseSensitive(decl, "name");
	cJSON* type = cJSON_GetObjectItemCaseSensitive(decl, "type");
	printf("function name: %s\n", name->valuestring);
	iFunctionNum++;

	if (cJSON_IsObject(type)) {
		cJSON* typeOfType = cJSON_GetObjectItemCaseSensitive(type, "type");

		cJSON* typeOfTypeOfType = cJSON_GetObjectItemCaseSensitive(typeOfType, "type");
		cJSON* identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(typeOfTypeOfType, "names"), 0);

		//��ȯŸ��
		if (identifier == NULL) {
			printf("return type: void\n");
		}
		else {
			printf("return type: %s\n", identifier->valuestring);
		}

		//�Ű�����
		cJSON* args = cJSON_GetObjectItemCaseSensitive(type, "args");
		cJSON* params = cJSON_GetObjectItemCaseSensitive(args, "params");
		cJSON* param;
		if (params != NULL) {
			int i = cJSON_GetArraySize(params);
			for (int k = 0; k < i; k++) {
				analyzeJsonObj_Function_Param(cJSON_GetArrayItem(params, k));
			}
		}
		else {
			printf("param: none\n");
		}
	}
	//if ����
	if (cJSON_IsObject(body)) {
		iIfnum = countIfNodetypes(body);
		
	}
	printf("if ����: %d", iIfnum);
	printf("\n");
}

//�Ű����� ����
void analyzeJsonObj_Function_Param(cJSON* param) {
	cJSON* type = cJSON_GetObjectItemCaseSensitive(param, "type");

	cJSON* type2 = cJSON_GetObjectItemCaseSensitive(type, "type");
	cJSON* name = cJSON_GetObjectItemCaseSensitive(param, "name");

	cJSON* type3 = cJSON_GetObjectItemCaseSensitive(type2, "type");
	cJSON* identifier;
	if (type3 != NULL) {
		identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type3, "names"), 0);
		name = cJSON_GetObjectItemCaseSensitive(type2, "declname");
	}
	else {
		identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type2, "names"), 0);
	}

	printf("param: %s %s\n", identifier->valuestring, name->valuestring);

}

//����� if Ž�� �� ���� ��ȯ
int countIfNodetypes(cJSON* item) {
	int count = 0; // ���� ��忡 ���� ī��Ʈ

	if (item == NULL) {
		return 0; // NULL ����� ���, ī��Ʈ���� ����
	}

	// ���� ��尡 '_nodetype'�� "If"���� Ȯ��
	cJSON* nodeType = cJSON_GetObjectItemCaseSensitive(item, "_nodetype");
	if (nodeType && cJSON_IsString(nodeType) && strcmp(nodeType->valuestring, "If") == 0) {
		count = 1; // ���� ��尡 ������ �����ϸ� ī��Ʈ ����
	}

	// ���� ��尡 ��ü�̰ų� �迭�� ���, �� ���θ� ��������� Ž��
	cJSON* child = NULL;
	cJSON_ArrayForEach(child, item) {
		count += countIfNodetypes(child); // �ڽ� ��� Ž�� ����� ī��Ʈ�� �߰�
	}

	return count; // ���� ī��Ʈ �� ��ȯ
}

//���� ���� �� �Լ� ����
void analyzeJsonObj_Decl(cJSON* nodeTypeDecl) {
	cJSON* name = cJSON_GetObjectItemCaseSensitive(nodeTypeDecl, "name");
	
	cJSON* type = cJSON_GetObjectItemCaseSensitive(nodeTypeDecl, "type");
	cJSON* type2 = cJSON_GetObjectItemCaseSensitive(type, "type");
	cJSON* nodetype= cJSON_GetObjectItemCaseSensitive(type, "_nodetype");
	
	//body ���� �Լ� ����
	if (strcmp(nodetype->valuestring, "FuncDecl") == 0) {
		printf("(�Լ� �̱���) %s\n", name->valuestring);
		cJSON* type3 = cJSON_GetObjectItemCaseSensitive(type2, "type");

		
		cJSON* identifier = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(type3, "names"), 0);

		if (identifier == NULL) {
			printf("return type: void\n");
		}
		else {
			printf("return type: %s\n", identifier->valuestring);
		}

		cJSON* args = cJSON_GetObjectItemCaseSensitive(type, "args");
		cJSON* params = cJSON_GetObjectItemCaseSensitive(args, "params");
		cJSON* param;
		if (params != NULL) {
			int i = cJSON_GetArraySize(params);
			for (int k = 0; k < i; k++) {
				analyzeJsonObj_Function_Param(cJSON_GetArrayItem(params, k));
			}
		}
		else {
			printf("param: none\n");
		}

	}
	
}



int main() {
	
	const char* filePath = "target.json";
	FILE* file = fopen(filePath, "r");
	if (file == NULL) {
		perror("Unable to open the file.");
		return 1;
	}

	// ���� ũ�� ���ϱ�
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* sTemp = NULL;
	char line[1024];

	char* jsonString = malloc(length + 1); // +1 null ���� �߰� ���� +1
	if (jsonString == NULL) {
		perror("Memory allocation failed");
		fclose(file);
		return 1;
	}
	jsonString[0] = '\0'; // �� ���ڷ� �ʱ�ȭ



	// ���� �б�
	while (fgets(line, sizeof(line), file) != NULL) {
		
		strncat(jsonString, line, length - strlen(jsonString));
	}


	// JSON ���ڿ� �Ľ�
	cJSON* json = parseJsonString(jsonString);
	iterJsonObj(json);

	// ����
	fclose(file);
	free(jsonString);
	printf("������ �Լ� �� %d��\n", iFunctionNum);


	return 0;
}
